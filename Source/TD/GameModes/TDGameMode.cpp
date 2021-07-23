// Copyright 2021, James S. Wang, All rights reserved.

#include "TDGameMode.h"

#include "EngineUtils.h"
#include "GameConfiguration.h"
#include "GameFramework/GameSession.h"
#include "GameState/TDGameState.h"
#include "GameState/Components/OrbState.h"
#include "GameState/Components/ScoreState.h"
#include "GameState/Components/TeamState.h"
#include "Player/TDCharacter.h"
#include "Player/TDController.h"
#include "Player/TDPlayerState.h"
#include "RoundStateMachine.h"
#include "GameRules/GameRules.h"
#include "Orb/Orb.h"

DEFINE_LOG_CATEGORY(LogTDGM);

namespace RoundState
{
    const FName WaitingPreRound = FName(TEXT("WaitingPreRound"));
    const FName RoundInProgress = FName(TEXT("RoundInProgress"));
    const FName RoundInOvertime = FName(TEXT("RoundInOvertime"));
    const FName WaitingPostRound = FName(TEXT("WaitingPostRound"));
}

#pragma region Match Initialization

ATDGameMode::ATDGameMode()
{
    GameStateClass = ATDGameState::StaticClass();

    bStartPlayersAsSpectators = true;

    RSM = CreateDefaultSubobject<URoundStateMachine>(TEXT("RSM"));
    check(RSM != nullptr);
}

void ATDGameMode::PreInitializeComponents()
{
    RSM->TDGameMode = this;
    Super::PreInitializeComponents();
    InitGameRules();
}

void ATDGameMode::InitGameRules()
{
    UWorld* const World = GetWorld();

    FActorSpawnParameters SpawnInfo;
    SpawnInfo.Instigator = GetInstigator();
    SpawnInfo.ObjectFlags |= RF_Transient;
    if (GameRulesClass == nullptr)
    {
        LogInvalidPointer("ATDGameMode", "InitGameRules", "GameRulesClass",
            "No GameRulesClass was specified, using default.");
        GameRulesClass = AGameRules::StaticClass();
    }

    AGameRules* const GameRules = World->SpawnActor<AGameRules>(
        GameRulesClass, SpawnInfo);
    if (GameRules != nullptr)
    {
        GameRules->SetGameState(TDGameState);
    }
}

void ATDGameMode::InitGameState()
{
    Super::InitGameState();
    TDGameState = Cast<ATDGameState>(GameState);
    checkf(TDGameState != nullptr, TEXT("ATDGameMode requires ATDGameState."));

    RSM->InitState(TDGameState);
}

void ATDGameMode::BeginPlay()
{
    Super::BeginPlay();
    RSM->RoundIsWaitingToStartEvent.AddUObject(this,
        &ATDGameMode::HandleRoundIsWaitingToStart);
    RSM->RoundHasStartedEvent.AddUObject(this,
        &ATDGameMode::HandleRoundHasStarted);
    RSM->RoundHasEndedEvent.AddUObject(this, &ATDGameMode::HandleRoundHasEnded);
    RSM->OvertimeHasStartedEvent.AddUObject(this,
        &ATDGameMode::HandleOvertimeHasStarted);
}

#pragma endregion

#pragma region Player Connection

/**
 * @brief Switches the player to the spectator team before spawning them.
 * This is so that they are given the correct spawning location.
 */
FString ATDGameMode::InitNewPlayer(APlayerController* NewPlayerController,
    const FUniqueNetIdRepl& UniqueId, const FString& Options,
    const FString& Portal)
{
    SwitchPlayerToTeam(Cast<ATDController>(NewPlayerController),
        ETeamIndex::Spectator);
    return Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);
}

void ATDGameMode::RestartPlayerAtPlayerStart(AController* NewPlayer,
    AActor* StartSpot)
{
    Super::RestartPlayerAtPlayerStart(NewPlayer, StartSpot);
    if (NewPlayer->GetPawn() != nullptr)
    {
        NewPlayer->GetPawn()->SetActorLocation(StartSpot->GetActorLocation());
        NewPlayer->GetPawn()->SetActorRotation(StartSpot->GetActorRotation());
    }
}

AActor* ATDGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
    UTeamState* TeamState = TDGameState->GetTeamStateComponent();
    return TeamState->GetNextPlayerSpawn(Player);
}

/**
 * @brief Always false. This is because players spawn in as spectators,
 * which have different spawn points than active players. When the game
 * starts, they shouldn't spawn where they started (the spectator spawn);
 * they should spawn at the team's corresponding spawn spot.
 */
bool ATDGameMode::ShouldSpawnAtStartSpot(AController* Player)
{
    return false;
}

#pragma endregion

#pragma region Match State

void ATDGameMode::Tick(float DeltaSeconds)
{
    // Go to Actor tick, skip GameMode tick
    Super::Super::Tick(DeltaSeconds);

    if (GetMatchState() == MatchState::WaitingToStart)
    {
        if (ReadyToStartMatch())
        {
            StartMatch();
        }
    }

    if (GetMatchState() == MatchState::InProgress)
    {
        if (RSM->GetRoundState() == RoundState::WaitingPreRound)
        {
            if (RSM->ReadyToStartOvertime())
            {
                RSM->StartOvertime();
            }
            else if (RSM->ReadyToStartRound())
            {
                RSM->StartRound();
            }
        }

        if (RSM->GetRoundState() == RoundState::RoundInProgress)
        {
            if (RSM->ReadyToEndRound())
            {
                RSM->EndRound();
            }
            else if (ReadyToEndMatch())
            {
                EndMatch();
            }
            else if (RSM->ReadyToStartOvertime())
            {
                RSM->InterruptRoundWithOvertime();
            }
        }

        if (RSM->GetRoundState() == RoundState::WaitingPostRound)
        {
            if (RSM->ReadyToStartNextRound())
            {
                RSM->StartNextRound();
            }
        }

        if (RSM->GetRoundState() == RoundState::RoundInOvertime)
        {
            if (ReadyToEndOvertime())
            {
                EndOvertime();
            }
        }
    }

    if (GetMatchState() == MatchState::WaitingPostMatch)
    {
        if (ReadyToRestartMatch())
        {
            RestartGame();
        }
    }
}

#pragma region Starting Match

bool ATDGameMode::ReadyToStartMatch_Implementation()
{
    UTeamState* TeamState = TDGameState->GetTeamStateComponent();
    return ShouldStartImmediately ||
           TeamState->DoesEachActiveTeamHaveAtLeastOnePlayer() &&
           Super::ReadyToStartMatch_Implementation();
}

#pragma endregion

#pragma region Match In Progress

uint16 ATDGameMode::GetMatchLengthInSeconds() const
{
    return MatchLengthInSeconds;
}

/**
 * Removes unused features, but most importantly, players are no longer spawned
 * as soon as the match starts. This should be deferred to WaitingPreRound.
 */
void ATDGameMode::HandleMatchHasStarted()
{
    GameSession->HandleMatchHasStarted();
    GEngine->BlockTillLevelStreamingCompleted(GetWorld());
    GetWorldSettings()->NotifyBeginPlay();
    GetWorldSettings()->NotifyMatchStarted();
    RSM->SetRoundState(RoundState::WaitingPreRound);
}

#pragma region Round State

void ATDGameMode::HandleRoundIsWaitingToStart()
{
    SpawnActivePlayers();
    DisablePlayerMovement();
}

void ATDGameMode::SpawnActivePlayers()
{
    for (FConstPlayerControllerIterator Iterator = GetWorld()->
             GetPlayerControllerIterator(); Iterator; ++Iterator)
    {
        ATDController* TDController = Cast<ATDController>(Iterator->Get());
        if (TDController == nullptr)
        {
            continue;
        }
        ATDPlayerState* TDPlayerState = TDController->GetPlayerState<
            ATDPlayerState>();
        if (TDPlayerState == nullptr)
        {
            continue;
        }

        if (IsActiveTeam(TDPlayerState->GetTeam()) &&
            PlayerCanRestart(TDController))
        {
            RestartPlayer(TDController);
            TDPlayerState->ResetRoundState();
        }
    }
}

void ATDGameMode::DisablePlayerMovement() const
{
    for (TActorIterator<ATDCharacter> It(GetWorld()); It; ++It)
    {
        ATDCharacter* Player = *It;
        Player->SetIsMovementEnabled(false);
    }
}

void ATDGameMode::HandleRoundHasStarted()
{
    HandlePlayHasStarted();
}

void ATDGameMode::EnablePlayerMovement() const
{
    for (TActorIterator<ATDCharacter> It(GetWorld()); It; ++It)
    {
        ATDCharacter* Player = *It;
        Player->SetIsMovementEnabled(true);
    }
}

void ATDGameMode::HandleRoundHasEnded()
{
}

void ATDGameMode::HandleOvertimeHasStarted()
{
    HandlePlayHasStarted();
}

void ATDGameMode::HandlePlayHasStarted()
{
    EnablePlayerMovement();
}

#pragma endregion

#pragma endregion

#pragma region Ending Match

bool ATDGameMode::ReadyToEndMatch_Implementation()
{
    UScoreState* ScoreState = TDGameState->GetScoreStateComponent();
    return MatchState == MatchState::InProgress &&
           !TDGameState->IsThereMatchTimeLeft() &&
           ScoreState->IsATeamStrictlyWinning();
}

bool ATDGameMode::ReadyToEndOvertime() const
{
    UTeamState* TeamState = TDGameState->GetTeamStateComponent();
    return MatchState == MatchState::InProgress &&
           RSM->RoundState == RoundState::RoundInOvertime &&
           TeamState->AreThereFewerThanTwoTeamsWithPlayersLeft();
}

void ATDGameMode::EndOvertime()
{
    if (MatchState != MatchState::InProgress ||
        RSM->RoundState != RoundState::RoundInOvertime)
    {
        return;
    }
    HandleOvertimeHasEnded();
    EndMatch();
}

void ATDGameMode::HandleOvertimeHasEnded()
{
    TDGameState->NotifyOvertimeHasEnded();
}

#pragma endregion

#pragma region Post Match

uint8 ATDGameMode::GetMatchRestartDelayInSeconds() const
{
    return MatchRestartDelayInSeconds;
}

void ATDGameMode::HandleMatchHasEnded()
{
    Super::HandleMatchHasEnded();
    DisablePlayerMovement();
    RSM->HandleMatchHasEnded();
}

void ATDGameMode::HandleLeavingMap()
{
    Super::HandleLeavingMap();
}

bool ATDGameMode::ReadyToRestartMatch() const
{
    return MatchState == MatchState::WaitingPostMatch &&
           !TDGameState->IsThereMatchRestartDelayLeft();
}

#pragma endregion

#pragma region Match Failure

void ATDGameMode::AbortMatch()
{
    Super::AbortMatch();
}

void ATDGameMode::HandleMatchAborted()
{
    Super::HandleMatchAborted();
}

void ATDGameMode::HandleDisconnect(UWorld* InWorld, UNetDriver* NetDriver)
{
    Super::HandleDisconnect(InWorld, NetDriver);
}

void ATDGameMode::HandleOrbImpact(AOrb* InstigatorOrb, const FHitResult& Hit)
{
    ATDCharacter* HitPlayer = Cast<ATDCharacter>(Hit.GetActor());
    if (HitPlayer != nullptr &&
        HitPlayer->GetTeam() != InstigatorOrb->GetTeam())
    {
        EliminatePlayer(HitPlayer);
        InstigatorOrb->Destroy();
    }
    TDGameState->HandleOrbImpact(InstigatorOrb, Hit);
}

void ATDGameMode::EliminatePlayer(ATDCharacter* Player)
{
    if (Player == nullptr)
    {
        LogInvalidPointer("ATDGameMode", "EliminatePlayer", "Player");
        return;
    }

    ATDController* TDController = Player->GetController<ATDController>();
    if (TDController == nullptr)
    {
        LogInvalidPointer("ATDGameMode", "EliminatePlayer", "TDController");
        return;
    }

    Player->Eliminate();
    TDController->Client_NotifyEliminated();
}

#pragma endregion

#pragma endregion

#pragma region Player Requests

void ATDGameMode::SwitchPlayerToTeam(ATDController* Player,
    const ETeamIndex Team) const
{
    if (Player == nullptr || Player->GetPlayerState<ATDPlayerState>() ==
        nullptr)
    {
        LogInvalidPointer("ATDGameMode", "SwitchPlayerToTeam",
            "GameState or Player or PlayerState");
        return;
    }

    if (IsMatchInProgress())
    {
        TDGameState->SetPlayerRequestResult(EPlayerRequestResult::Error,
            "Player tried to switch team, but match is in progress");
        return;
    }

    UTeamState* TeamState = TDGameState->GetTeamStateComponent();
    TeamState->SwitchPlayerToTeam(Player->GetPlayerState<ATDPlayerState>(),
        Team);
    TDGameState->SetPlayerRequestResult(EPlayerRequestResult::Success);

    static const UEnum* Enum = StaticEnum<ETeamIndex>();
    UE_LOG(LogTDGM, Log, TEXT("Switching player %s to team %s"),
        *Player->GetPlayerState<ATDPlayerState>()->GetPlayerName(),
        *Enum->GetNameStringByValue(Int(Team)));
}

#pragma endregion
