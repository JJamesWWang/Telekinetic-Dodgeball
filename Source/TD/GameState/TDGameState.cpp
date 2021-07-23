// Copyright 2021, James S. Wang, All rights reserved.

#include "TDGameState.h"

#include "GameConfiguration.h"
#include "UIGameState.h"
#include "RoundState.h"
#include "TDGameInstance.h"
#include "Components/OrbState.h"
#include "Components/TeamState.h"
#include "Components/ScoreState.h"
#include "GameModes/TDGameMode.h"
#include "Orb/Orb.h"

#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY(LogTDGS);

#pragma region Initialization

ATDGameState::ATDGameState()
{
    PrimaryActorTick.bCanEverTick = true;

    UIGameState = CreateDefaultSubobject<UUIGameState>(TEXT("UI Game State"));
    RoundStateComponent = CreateDefaultSubobject<URoundState>(
        TEXT("Round State"));
    RoundStateComponent->UIGameState = UIGameState;

    CreateStateComponents();
    AddStateComponents();
    InitStateComponents();
}

void ATDGameState::CreateStateComponents()
{
    TeamStateComponent = CreateDefaultSubobject<UTeamState>(TEXT("Team State"));
    ScoreStateComponent = CreateDefaultSubobject<UScoreState>(
        TEXT("Score State"));
    OrbStateComponent = CreateDefaultSubobject<UOrbState>(TEXT("Orb State"));
}

void ATDGameState::AddStateComponents()
{
    StateComponents.Emplace(TeamStateComponent);
    StateComponents.Emplace(ScoreStateComponent);
    StateComponents.Emplace(OrbStateComponent);
}

void ATDGameState::InitStateComponents()
{
    for (UTDGameStateComponent* Component : StateComponents)
    {
        Component->UIGameState = UIGameState;
        Component->TDGameState = this;
    }
}

void ATDGameState::BeginPlay()
{
    Super::BeginPlay();
    RoundStateComponent->RoundIsWaitingToStartEvent.AddUObject(this,
        &ATDGameState::HandleRoundIsWaitingToStart);
    RoundStateComponent->RoundHasStartedEvent.AddUObject(this,
        &ATDGameState::HandleRoundHasStarted);
    RoundStateComponent->RoundHasEndedEvent.AddUObject(this,
        &ATDGameState::HandleRoundHasEnded);
    RoundStateComponent->OvertimeHasStartedEvent.AddUObject(this,
        &ATDGameState::HandleOvertimeHasStarted);
}

void ATDGameState::ReceivedGameModeClass()
{
    Super::ReceivedGameModeClass();
    TDGameMode = Cast<ATDGameMode>(AuthorityGameMode);
    if (HasAuthority())
    {
        checkf(TDGameMode != nullptr,
            TEXT("ATDGameState requires ATDGameMode."))
    }
}

void ATDGameState::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ATDGameState, SecondsUntilMatchEnds);
    DOREPLIFETIME(ATDGameState, SecondsUntilMatchRestarts);
}

URoundState* ATDGameState::GetRoundStateComponent() const
{
    return RoundStateComponent;
}

UTeamState* ATDGameState::GetTeamStateComponent() const
{
    return TeamStateComponent;
}

UScoreState* ATDGameState::GetScoreStateComponent() const
{
    return ScoreStateComponent;
}

UOrbState* ATDGameState::GetOrbStateComponent() const
{
    return OrbStateComponent;
}

UUIGameState* ATDGameState::GetUIGameState() const
{
    return UIGameState;
}

#pragma endregion

#pragma region Gameplay Requests

void ATDGameState::HandleOrbImpact(AOrb* InstigatorOrb,
    const FHitResult& Hit) const
{
    OrbStateComponent->HandleOrbImpact(InstigatorOrb, Hit);
}

void ATDGameState::PlayLocalCue(USoundBase* Sound, FVector* Location) const
{
    const FPlayerAudioSettings& AudioSettings = GetAudioSettings();
    if (Location != nullptr)
    {
        UGameplayStatics::PlaySoundAtLocation(this, Sound, *Location,
            AudioSettings.GameSFXVolume);
    }
    else
    {
        UGameplayStatics::PlaySound2D(this, Sound, AudioSettings.GameSFXVolume);
    }
}

void ATDGameState::PlayReplicatedCueAtLocation_Implementation(USoundBase* Sound,
    const FVector& Location)
{
    const FPlayerAudioSettings& AudioSettings = GetAudioSettings();
    UGameplayStatics::PlaySoundAtLocation(this, Sound, Location,
        AudioSettings.GameSFXVolume);
}

FPlayerAudioSettings ATDGameState::GetAudioSettings() const
{
    UTDGameInstance* GameInstance = GetGameInstance<UTDGameInstance>();
    if (GameInstance == nullptr)
    {
        LogInvalidPointer("ATDGameState", "GetAudioSettings", "GameInstance");
        return FPlayerAudioSettings();
    }
    return GameInstance->LoadPlayerAudioSettings();
}

#pragma endregion

#pragma region Player Requests

void ATDGameState::SetPlayerRequestResult(const EPlayerRequestResult Result,
    const FString& Message) const
{
    // Temporarily log the error message until it's displayed in the UI.
    if (Result == EPlayerRequestResult::Error)
    {
        UE_LOG(LogTDGS, Warning, TEXT("Request failed: %s"), *Message);
    }
    UIGameState->SetPlayerRequest(FPlayerRequest{Result, Message});
}
#pragma endregion

#pragma region Match State

void ATDGameState::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (RoundStateComponent->ShouldMatchTimerTick)
    {
        SecondsUntilMatchEnds -= DeltaSeconds;
        UIGameState->SetMatchTimeInSeconds(SecondsUntilMatchEnds);
    }

    RoundStateComponent->MatchTick(DeltaSeconds);

    if (ShouldRestartMatchTimerTick)
    {
        SecondsUntilMatchRestarts -= DeltaSeconds;
        UIGameState->SetSecondsUntilMatchRestarts(SecondsUntilMatchRestarts);
    }
}

void ATDGameState::OnRep_MatchState()
{
    Super::OnRep_MatchState();
    UIGameState->SetMatchState(MatchState);
}

#pragma region Pre Match

void ATDGameState::HandleMatchIsWaitingToStart()
{
    Super::HandleMatchIsWaitingToStart();
    if (HasAuthority())
    {
        SecondsUntilMatchEnds = TDGameMode->GetMatchLengthInSeconds();
    }

    for (UTDGameStateComponent* Component : StateComponents)
    {
        Component->HandleMatchIsWaitingToStart();
    }
}

#pragma endregion

#pragma region Match In Progress

void ATDGameState::HandleMatchHasStarted()
{
    Super::HandleMatchHasStarted();
    UIGameState->SetMatchTimeInSeconds(SecondsUntilMatchEnds);

    for (UTDGameStateComponent* Component : StateComponents)
    {
        Component->HandleMatchHasStarted();
    }
}

bool ATDGameState::IsThereMatchTimeLeft() const
{
    return SecondsUntilMatchEnds > 0.0f;
}

#pragma region Round State

FName ATDGameState::GetRoundState() const
{
    return RoundStateComponent->RoundState;
}

#pragma region Pre Round

void ATDGameState::HandleRoundIsWaitingToStart()
{
    for (UTDGameStateComponent* Component : StateComponents)
    {
        Component->HandleRoundIsWaitingToStart();
    }
}

#pragma endregion

#pragma region Round In Progress

void ATDGameState::HandleRoundHasStarted()
{
    for (UTDGameStateComponent* Component : StateComponents)
    {
        Component->HandleRoundHasStarted();
    }
}

#pragma endregion

#pragma region Post Round

void ATDGameState::HandleRoundHasEnded()
{
    for (UTDGameStateComponent* Component : StateComponents)
    {
        Component->HandleRoundHasEnded();
    }
}

#pragma endregion

#pragma region Overtime

void ATDGameState::HandleOvertimeHasStarted()
{
    for (UTDGameStateComponent* Component : StateComponents)
    {
        Component->HandleOvertimeHasStarted();
    }
}

#pragma endregion

#pragma endregion

#pragma endregion

#pragma region Post Match

bool ATDGameState::IsThereMatchRestartDelayLeft() const
{
    return SecondsUntilMatchRestarts > 0.0f;
}

void ATDGameState::NotifyOvertimeHasEnded()
{
    HandleOvertimeHasEnded();
}

void ATDGameState::HandleOvertimeHasEnded()
{
    for (UTDGameStateComponent* Component : StateComponents)
    {
        Component->HandleOvertimeHasEnded();
    }
}

void ATDGameState::HandleMatchHasEnded()
{
    Super::HandleMatchHasEnded();
    RoundStateComponent->HandleMatchHasEnded();
    if (HasAuthority())
    {
        SecondsUntilMatchRestarts = TDGameMode->GetMatchRestartDelayInSeconds();
    }
    ShouldRestartMatchTimerTick = true;

    for (UTDGameStateComponent* Component : StateComponents)
    {
        Component->HandleMatchHasEnded();
    }
}

#pragma endregion

#pragma endregion
