// Copyright 2021, James S. Wang, All rights reserved.

#include "TeamState.h"

#include "EngineUtils.h"
#include "GameConfiguration.h"
#include "TDTypes.h"
#include "Arena/TeamPlayerStart.h"
#include "GameState/UIGameState.h"
#include "Net/UnrealNetwork.h"
#include "Player/TDPlayerState.h"

#pragma region Initialization

UTeamState::UTeamState()
{
    SetIsReplicatedByDefault(true);
    FillTeams();
    FillNextTeamPlayerStartIndices();
}

void UTeamState::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UTeamState, Teams);
}

void UTeamState::FillTeams()
{
    for (ETeamIndex Index : TEnumRange<ETeamIndex>())
    {
        FTeam Team(Index);
        Teams.Add(Team);
    }
}

void UTeamState::FindAllTeamPlayerStarts()
{
    // Fill arrays because the iterator isn't guaranteed to be sorted by team.
    for (const ETeamIndex Team : TEnumRange<ETeamIndex>())
    {
        AllTeamPlayerStarts.Emplace(TArray<ATeamPlayerStart*>());
    }

    UWorld* const World = GetWorld();
    if (World == nullptr)
    {
        LogInvalidPointer("UTeamState", "FindAllTeamPlayerStarts", "World");
        return;
    }

    for (TActorIterator<ATeamPlayerStart> It(World); It; ++It)
    {
        ATeamPlayerStart* PlayerStart = *It;
        const uint8 TeamIndex = PlayerStart->GetTeamIndex();
        AllTeamPlayerStarts[TeamIndex].Emplace(PlayerStart);
    }
}

void UTeamState::FillNextTeamPlayerStartIndices()
{
    for (const ETeamIndex Team : TEnumRange<ETeamIndex>())
    {
        NextTeamPlayerStartIndices.Emplace(0);
    }
}

#pragma endregion

#pragma region Team State

const TArray<FTeam>& UTeamState::GetTeams() const
{
    return Teams;
}

void UTeamState::OnRep_Teams()
{
    UIGameState->SetTeams(Teams);
}

TArray<FTeam> UTeamState::GetActiveTeams() const
{
    return Teams.FilterByPredicate([&](const FTeam& Team)
    {
        return Team.IsActive;
    });
}

TArray<FTeam> UTeamState::GetActiveTeamsWithPlayersLeft() const
{
    return GetActiveTeams().FilterByPredicate([&](const FTeam& Team)
    {
        return DoesTeamHavePlayersLeft(Team);
    });
}

TArray<FTeam> UTeamState::GetActiveTeamsWithNoEliminatedPlayers() const
{
    return GetActiveTeams().FilterByPredicate([&](const FTeam& Team)
    {
        return DoesTeamHaveNoEliminatedPlayers(Team);
    });
}

void UTeamState::SwitchPlayerToTeam(ATDPlayerState* Player,
    const ETeamIndex TeamIndex)
{
    if (Player == nullptr)
    {
        LogInvalidPointer("UTeamState", "SwitchPlayerToTeam", "Player");
        return;
    }

    for (FTeam& Team : Teams)
    {
        if (Team.Index == TeamIndex && !Team.Players.Contains(Player))
        {
            Team.Players.Emplace(Player);
        }
        else
        {
            Team.Players.Remove(Player);
        }
    }
    Player->SetTeam(TeamIndex);
    UIGameState->SetTeams(Teams);
}

#pragma endregion

#pragma region Spawning Players

AActor* UTeamState::GetNextPlayerSpawn(AController* Player)
{
    if (AllTeamPlayerStarts.Num() == 0)
    {
        FindAllTeamPlayerStarts();
    }

    const ATDPlayerState* const PlayerState = Player->GetPlayerState<
        ATDPlayerState>();
    if (PlayerState == nullptr)
    {
        LogInvalidPointer("UTeamState", "ChoosePlayerStart_Implementation",
            "PlayerState");
        return nullptr;
    }

    const uint8 TeamIndex = PlayerState->GetTeamIndex();
    const TArray<ATeamPlayerStart*>& TeamPlayerStarts = AllTeamPlayerStarts[
        TeamIndex];
    if (TeamPlayerStarts.Num() == 0)
    {
        static const UEnum* Enum = StaticEnum<ETeamIndex>();
        LogInvalidPointer("UTeamState", "ChoosePlayerStart_Implementation",
            "TeamPlayerStarts",
            FString::Printf(
                TEXT("ID %s has no TeamPlayerStarts to choose from."),
                *Enum->GetNameStringByValue(TeamIndex)));
        return nullptr;
    }

    const uint8 PlayerStartIndex = NextTeamPlayerStartIndices[TeamIndex];
    ATeamPlayerStart* TeamPlayerStart = TeamPlayerStarts[PlayerStartIndex];
    NextTeamPlayerStartIndices[TeamIndex] =
        (PlayerStartIndex + 1) % TeamPlayerStarts.Num();
    return TeamPlayerStart;
}

#pragma endregion

#pragma region Team Queries

bool UTeamState::DoesEachActiveTeamHaveAtLeastOnePlayer() const
{
    for (const FTeam& Team : GetActiveTeams())
    {
        if (Team.Players.Num() < 1)
        {
            return false;
        }
    }
    return true;
}

bool UTeamState::HasAPlayerBeenEliminated() const
{
    for (const FTeam& Team : GetActiveTeams())
    {
        if (!DoesTeamHaveNoEliminatedPlayers(Team))
        {
            return true;
        }
    }
    return false;
}

bool UTeamState::DoesTeamHavePlayersLeft(const FTeam& Team) const
{
    for (ATDPlayerState* const Player : Team.Players)
    {
        if (!Player->GetIsEliminated())
        {
            return true;
        }
    }
    return false;
}

bool UTeamState::DoesTeamHaveNoEliminatedPlayers(const FTeam& Team) const
{
    uint8 TotalPlayers = Team.Players.Num();
    uint8 PlayersNotEliminated = 0;
    for (ATDPlayerState* const Player : Team.Players)
    {
        PlayersNotEliminated += !Player->GetIsEliminated() ? 1 : 0;
    }
    return TotalPlayers == PlayersNotEliminated;
}

bool UTeamState::AreThereFewerThanTwoTeamsWithPlayersLeft() const
{
    uint8 NumTeamsWithPlayersLeft = 0;
    for (const FTeam& Team : GetActiveTeams())
    {
        NumTeamsWithPlayersLeft += DoesTeamHavePlayersLeft(Team) ? 1 : 0;
        if (NumTeamsWithPlayersLeft > 1)
        {
            return false;
        }
    }
    return true;
}

#pragma endregion
