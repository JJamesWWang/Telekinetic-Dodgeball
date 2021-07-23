// Copyright 2021, James S. Wang, All rights reserved.

#include "UIGameState.h"
#include "GameFramework/GameMode.h"
#include "GameModes/RoundStateMachine.h"

/**
 * Setter with delegate template.
 * Replace VariableType, VariableName, and DelegateName.

void UUIGameState::SetVariableName(VariableType NewVariableName)
{
    VariableType OldVariableName = VariableName;
    VariableName = NewVariableName;
    DelegateName.Broadcast(OldVariableName, NewVariableName);
}
 */

void UUIGameState::SetMatchState(const FName& NewMatchState)
{
    const FName& OldMatchState = MatchState;
    MatchState = NewMatchState;
    OnMatchStateUpdated.Broadcast(OldMatchState, NewMatchState);
    BroadcastNewMatchState();
}

void UUIGameState::BroadcastNewMatchState() const
{
    if (MatchState == MatchState::InProgress)
    {
        OnMatchStarted.Broadcast();
    }
    else if (MatchState == MatchState::WaitingPostMatch)
    {
        OnMatchEnded.Broadcast();
    }
}

void UUIGameState::SetRoundState(const FName& NewRoundState)
{
    const FName& OldRoundState = RoundState;
    RoundState = NewRoundState;
    OnRoundStateUpdated.Broadcast(OldRoundState, NewRoundState);
    BroadcastNewRoundState();
}

void UUIGameState::BroadcastNewRoundState() const
{
    if (RoundState == RoundState::WaitingPreRound)
    {
        OnRoundIsWaitingToStart.Broadcast();
    }
    else if (RoundState == RoundState::RoundInProgress)
    {
        OnRoundStarted.Broadcast();
    }
    else if (RoundState == RoundState::WaitingPostRound)
    {
        OnRoundEnded.Broadcast();
    }
    else if (RoundState == RoundState::RoundInOvertime)
    {
        OnOvertimeStarted.Broadcast();
    }
}

void UUIGameState::SetTeams(const TArray<FTeam>& NewTeams)
{
    const TArray<FTeam>& OldTeams = Teams;
    Teams = NewTeams;
    OnTeamsChanged.Broadcast(OldTeams, NewTeams);
}

void UUIGameState::SetPlayerRequest(const FPlayerRequest& NewPlayerRequest)
{
    const FPlayerRequest& OldPlayerRequest = PlayerRequest;
    PlayerRequest = NewPlayerRequest;
    OnPlayerRequestCompleted.Broadcast(OldPlayerRequest, NewPlayerRequest);
}

void UUIGameState::SetMatchTimeInSeconds(
    const float NewSecondsUntilMatchEnds)
{
    float OldSecondsUntilMatchEnds = MatchTimeInSeconds;
    MatchTimeInSeconds = NewSecondsUntilMatchEnds;
    OnMatchTimeInSecondsUpdated.Broadcast(OldSecondsUntilMatchEnds,
        NewSecondsUntilMatchEnds);
}

void UUIGameState::SetSecondsUntilRoundStarts(
    const float NewSecondsUntilRoundStarts)
{
    const float OldSecondsUntilRoundStarts = SecondsUntilRoundStarts;
    SecondsUntilRoundStarts = NewSecondsUntilRoundStarts;
    OnSecondsUntilRoundStartsUpdated.Broadcast(OldSecondsUntilRoundStarts,
        NewSecondsUntilRoundStarts);
}

void UUIGameState::SetTeamScores(const TArray<uint8>& NewTeamScores)
{
    const TArray<uint8>& OldTeamScores = TeamScores;
    TeamScores = NewTeamScores;
    OnTeamScoresUpdated.Broadcast(OldTeamScores, NewTeamScores);
}

void UUIGameState::SetSecondsUntilMatchRestarts(
    const float NewSecondsUntilMatchRestarts)
{
    const float OldSecondsUntilMatchRestarts = SecondsUntilMatchRestarts;
    SecondsUntilMatchRestarts = NewSecondsUntilMatchRestarts;
    OnSecondsUntilMatchRestartsUpdated.Broadcast(OldSecondsUntilMatchRestarts,
        NewSecondsUntilMatchRestarts);
}

void UUIGameState::SetRoundWinningTeams(
    const TArray<ETeamIndex>& NewRoundWinningTeams)
{
    const TArray<ETeamIndex>& OldRoundWinningTeams = RoundWinningTeams;
    RoundWinningTeams = NewRoundWinningTeams;
    OnRoundWinningTeamsUpdated.Broadcast(OldRoundWinningTeams,
        NewRoundWinningTeams);
}

void UUIGameState::SetMatchWinningTeams(
    const TArray<ETeamIndex>& NewMatchWinningTeams)
{
    const TArray<ETeamIndex>& OldMatchWinningTeams = MatchWinningTeams;
    MatchWinningTeams = NewMatchWinningTeams;
    OnMatchWinningTeamsUpdated.Broadcast(OldMatchWinningTeams,
        NewMatchWinningTeams);
}

ETeamIndex UUIGameState::GetWinnerTeam() const
{
    uint8 Index = 0;
    uint8 MaxScore = 0;
    uint8 WinnerTeamIndex = 0;
    for (uint8 Score : TeamScores)
    {
        if (Score > MaxScore)
        {
            MaxScore = Score;
            WinnerTeamIndex = Index;
        }
        Index += 1;
    }
    return static_cast<ETeamIndex>(WinnerTeamIndex);
}

FText UUIGameState::TeamsToString(const TArray<ETeamIndex>& TeamIndices)
{
    FString String;
    if (TeamIndices.Num() == 1)
    {
        String = TeamIndexToString(TeamIndices[0]);
    }
    else if (TeamIndices.Num() == 2)
    {
        String = TeamIndexToString(TeamIndices[0]) + " and " +
                 TeamIndexToString(TeamIndices[1]);
    }
    else if (TeamIndices.Num() >= 3)
    {
        for (int i = 0; i < TeamIndices.Num() - 1; ++i)
        {
            String += TeamIndexToString(TeamIndices[0]) + ", ";
        }
        String += "and " +
            TeamIndexToString(TeamIndices[TeamIndices.Num() - 1]);
    }
    return FText::FromString(String);
}

FString UUIGameState::TeamIndexToString(const ETeamIndex TeamIndex) const
{
    UEnum* TeamIndexEnum = StaticEnum<ETeamIndex>();
    return TeamIndexEnum->GetNameStringByValue(Int(TeamIndex));
}
