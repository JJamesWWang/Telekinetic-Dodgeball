// Copyright 2021, James S. Wang, All rights reserved.

#include "RoundStateMachine.h"

#include "GameState/RoundState.h"
#include "GameState/TDGameState.h"
#include "GameState/Components/TeamState.h"
#include "GameFramework/GameMode.h"
#include "TDGameMode.h"

DEFINE_LOG_CATEGORY(LogTDRSM);

#pragma region Round State

void URoundStateMachine::InitState(ATDGameState* GameState)
{
    check(GameState != nullptr);
    TDGameState = GameState;
    State = GameState->GetRoundStateComponent();
    State->StateMachine = this;
}

FName URoundStateMachine::GetRoundState() const
{
    return RoundState;
}

void URoundStateMachine::SetRoundState(const FName& NewState)
{
    if (RoundState == NewState)
    {
        return;
    }

    RoundState = NewState;
    State->SetRoundState(NewState);
    OnRoundStateSet(NewState);
}

void URoundStateMachine::OnRoundStateSet(const FName& NewState)
{
    UE_LOG(LogTDGM, Log, TEXT("RoundState set to %s"), *NewState.ToString());
    if (RoundState == RoundState::WaitingPreRound)
    {
        HandleRoundIsWaitingToStart();
    }
    else if (RoundState == RoundState::RoundInProgress)
    {
        HandleRoundHasStarted();
    }
    else if (RoundState == RoundState::WaitingPostRound)
    {
        HandleRoundHasEnded();
    }
    else if (RoundState == RoundState::RoundInOvertime)
    {
        HandleOvertimeHasStarted();
    }
    else
    {
        UE_LOG(LogTDGM, Warning,
            TEXT(
                "URoundStateMachine::OnRoundStateSet received unknown state: %s"
            ),
            *NewState.ToString())
    }
}

#pragma region Starting Round

uint8 URoundStateMachine::GetRoundStartDelayInSeconds() const
{
    return RoundStartDelayInSeconds;
}

void URoundStateMachine::HandleRoundIsWaitingToStart()
{
    RoundIsWaitingToStartEvent.Broadcast();
}

bool URoundStateMachine::ReadyToStartRound() const
{
    return RoundState == RoundState::WaitingPreRound &&
           !State->IsThereRoundStartDelayLeft();
}

void URoundStateMachine::StartRound()
{
    if (RoundState == RoundState::RoundInProgress)
    {
        return;
    }
    SetRoundState(RoundState::RoundInProgress);
}

#pragma endregion Starting Round

#pragma region Round In Progress

void URoundStateMachine::HandleRoundHasStarted()
{
    RoundHasStartedEvent.Broadcast();
}

bool URoundStateMachine::ReadyToEndRound() const
{
    UTeamState* TeamState = TDGameState->GetTeamStateComponent();
    return TDGameMode->GetMatchState() == MatchState::InProgress &&
           RoundState == RoundState::RoundInProgress &&
           TeamState->HasAPlayerBeenEliminated();
}

#pragma endregion Round In Progress

#pragma region Ending Round

void URoundStateMachine::EndRound()
{
    if (RoundState == RoundState::WaitingPostRound)
    {
        return;
    }
    SetRoundState(RoundState::WaitingPostRound);
}

bool URoundStateMachine::ReadyToStartNextRound() const
{
    return TDGameMode->GetMatchState() == MatchState::InProgress &&
           RoundState == RoundState::WaitingPostRound &&
           !State->IsThereEndOfRoundDelayLeft();
}

void URoundStateMachine::StartNextRound()
{
    if (RoundState == RoundState::WaitingPreRound)
    {
        return;
    }

    SetRoundState(RoundState::WaitingPreRound);
}

#pragma endregion Ending Round

#pragma region Post Round

uint8 URoundStateMachine::GetEndOfRoundDelayInSeconds() const
{
    return EndOfRoundDelayInSeconds;
}

void URoundStateMachine::HandleRoundHasEnded()
{
    RoundHasEndedEvent.Broadcast();
}

#pragma endregion

#pragma region Overtime

/**
 * @brief Since the check for if the match is ready to end comes before the
 * check for if overtime should begin, we don't have to check if the match
 * should end here.
 */
bool URoundStateMachine::ReadyToStartOvertime() const
{
    if (TDGameMode->GetMatchState() != MatchState::InProgress)
    {
        return false;
    }

    if (RoundState == RoundState::WaitingPreRound)
    {
        return !TDGameState->IsThereMatchTimeLeft() &&
               !State->IsThereRoundStartDelayLeft();
    }

    if (RoundState == RoundState::RoundInProgress)
    {
        return !TDGameState->IsThereMatchTimeLeft();
    }

    return false;
}

void URoundStateMachine::InterruptRoundWithOvertime()
{
    if (RoundState == RoundState::WaitingPreRound)
    {
        return;
    }

    SetRoundState(RoundState::WaitingPreRound);
}

void URoundStateMachine::StartOvertime()
{
    if (RoundState == RoundState::RoundInOvertime)
    {
        return;
    }

    SetRoundState(RoundState::RoundInOvertime);
}

void URoundStateMachine::HandleOvertimeHasStarted()
{
    OvertimeHasStartedEvent.Broadcast();
}

void URoundStateMachine::HandleMatchHasEnded()
{
}

#pragma endregion

#pragma endregion
