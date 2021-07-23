// Copyright 2021, James S. Wang, All rights reserved.

#include "ScoreState.h"

#include "TDTypes.h"
#include "TeamState.h"
#include "GameState/TDGameState.h"
#include "GameState/UIGameState.h"
#include "Net/UnrealNetwork.h"

#pragma region Initialization

UScoreState::UScoreState()
{
    SetIsReplicatedByDefault(true);
    FillTeamScores();
}

void UScoreState::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UScoreState, TeamScores);
    DOREPLIFETIME(UScoreState, RoundWinningTeams);
    DOREPLIFETIME(UScoreState, MatchWinningTeams);
}

void UScoreState::FillTeamScores()
{
    for (const ETeamIndex Index : TEnumRange<ETeamIndex>())
    {
        TeamScores.Emplace(0);
    }
}

void UScoreState::HandleRoundIsWaitingToStart()
{
    TArray<ETeamIndex> ResetRoundWinningTeams;
    ResetRoundWinningTeams.Emplace(ETeamIndex::None);
    RoundWinningTeams = ResetRoundWinningTeams;
    UIGameState->SetRoundWinningTeams(RoundWinningTeams);
}

#pragma endregion

#pragma region Game State Events

void UScoreState::HandleRoundHasEnded()
{
    if (GetOwnerRole() != ROLE_Authority)
    {
        return;
    }
    UTeamState* TeamState = TDGameState->GetTeamStateComponent();
    TArray<FTeam> WinningTeams = TeamState->
        GetActiveTeamsWithNoEliminatedPlayers();
    AddScoreToTeams(WinningTeams);
    RoundWinningTeams = ConvertTeamsToIndices(WinningTeams);
    UIGameState->SetRoundWinningTeams(RoundWinningTeams);
}

void UScoreState::HandleOvertimeHasEnded()
{
    if (GetOwnerRole() != ROLE_Authority)
    {
        return;
    }
    UTeamState* TeamState = TDGameState->GetTeamStateComponent();
    TArray<FTeam> WinningTeams = TeamState->
        GetActiveTeamsWithNoEliminatedPlayers();
    AddScoreToTeams(WinningTeams);
}

void UScoreState::HandleMatchHasEnded()
{
    MatchWinningTeams = GetTeamIndicesWithScore(GetTeamMaxScore());
    UIGameState->SetMatchWinningTeams(MatchWinningTeams);
}

uint8 UScoreState::GetTeamMaxScore() const
{
    uint8 MaxScore = 0;
    for (uint8 Score : TeamScores)
    {
        MaxScore = FMath::Max<uint8>(MaxScore, Score);
    }
    return MaxScore;
}

TArray<ETeamIndex> UScoreState::GetTeamIndicesWithScore(uint8 Score) const
{
    TArray<ETeamIndex> TeamIndices;
    for (ETeamIndex Index : TEnumRange<ETeamIndex>())
    {
        if (TeamScores[Int(Index)] == Score)
        {
            TeamIndices.Emplace(Index);
        }
    }
    return TeamIndices;
}

#pragma endregion

#pragma region Score State

void UScoreState::OnRep_TeamScores()
{
    UIGameState->SetTeamScores(TeamScores);
}

void UScoreState::OnRep_RoundWinningTeams()
{
    UIGameState->SetRoundWinningTeams(RoundWinningTeams);
}

void UScoreState::OnRep_MatchWinningTeams()
{
    UIGameState->SetMatchWinningTeams(MatchWinningTeams);
}

void UScoreState::AddScoreToTeams(const TArray<FTeam>& Teams)
{
    for (const FTeam& Team : Teams)
    {
        AddScoreToTeam(Team);
    }
}

void UScoreState::AddScoreToTeam(const FTeam& Team)
{
    TeamScores[Int(Team.Index)]++;
    UIGameState->SetTeamScores(TeamScores);
}

#pragma endregion

#pragma region Score Queries

/**
 * @brief A team is strictly winning if it has a higher score than every other
 * team.
 */
bool UScoreState::IsATeamStrictlyWinning() const
{
    uint8 MaxScore = 0;
    uint8 MaxScoreFrequency = 0;
    for (ETeamIndex Index : TEnumRange<ETeamIndex>())
    {
        const uint8 Score = TeamScores[Int(Index)];
        if (Score > MaxScore)
        {
            MaxScore = Score;
            MaxScoreFrequency = 1;
        }
        else if (Score == MaxScore)
        {
            MaxScoreFrequency += 1;
        }
    }
    return MaxScoreFrequency < 2;
}

#pragma endregion
