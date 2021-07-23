// Copyright 2021, James S. Wang, All rights reserved.

#include "RoundState.h"

#include "GameModes/RoundStateMachine.h"
#include "UIGameState.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY(LogTDRS);

#pragma region Round State

URoundState::URoundState()
{
    SetIsReplicatedByDefault(true);
}

void URoundState::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(URoundState, RoundState);
    DOREPLIFETIME(URoundState, SecondsUntilRoundStarts);
    DOREPLIFETIME(URoundState, SecondsUntilNextRoundStarts);
    DOREPLIFETIME(URoundState, OvertimeSecondsPassed);
}

#pragma region Round State

void URoundState::MatchTick(float DeltaSeconds)
{
    if (ShouldStartRoundTimerTick)
    {
        SecondsUntilRoundStarts -= DeltaSeconds;
        UIGameState->SetSecondsUntilRoundStarts(SecondsUntilRoundStarts);
    }
    SecondsUntilNextRoundStarts -= ShouldEndOfRoundTimerTick
                                       ? DeltaSeconds
                                       : 0;
    if (ShouldOvertimeTimerTick)
    {
        OvertimeSecondsPassed += DeltaSeconds;
        UIGameState->SetMatchTimeInSeconds(OvertimeSecondsPassed);
    }
}

void URoundState::SetRoundState(FName NewState)
{
    if (GetOwnerRole() == ROLE_Authority)
    {
        UE_LOG(LogTDRS, Log, TEXT("Round State Changed from %s to %s"),
            *RoundState.ToString(), *NewState.ToString())
        RoundState = NewState;

        // So callbacks happen on the server too.
        OnRep_RoundState();
    }
}

void URoundState::OnRep_RoundState()
{
    if (RoundState == RoundState::WaitingPreRound)
    {
        HandleRoundIsWaitingToStart();
    }
    else if (RoundState == RoundState::RoundInProgress)
    {
        HandleRoundHasStarted();
    }
    else if (RoundState == RoundState::RoundInOvertime)
    {
        HandleOvertimeHasStarted();
    }
    else if (RoundState == RoundState::WaitingPostRound)
    {
        HandleRoundHasEnded();
    }
    UIGameState->SetRoundState(RoundState);
}

#pragma region Pre Round

void URoundState::HandleRoundIsWaitingToStart()
{
    if (GetOwnerRole() == ROLE_Authority)
    {
        SecondsUntilRoundStarts = StateMachine->GetRoundStartDelayInSeconds();
    }
    ShouldStartRoundTimerTick = true;
    ShouldEndOfRoundTimerTick = false;
    ShouldMatchTimerTick = false;
    RoundIsWaitingToStartEvent.Broadcast();
}

bool URoundState::IsThereRoundStartDelayLeft() const
{
    return SecondsUntilRoundStarts > 0.0f;
}

#pragma endregion

#pragma region Round In Progress

void URoundState::HandleRoundHasStarted()
{
    ShouldMatchTimerTick = true;
    ShouldStartRoundTimerTick = false;
    RoundHasStartedEvent.Broadcast();
}

#pragma endregion

#pragma region Post Round

void URoundState::HandleRoundHasEnded()
{
    if (GetOwnerRole() == ROLE_Authority)
    {
        SecondsUntilNextRoundStarts = StateMachine->
            GetEndOfRoundDelayInSeconds();
    }
    ShouldMatchTimerTick = false;
    ShouldEndOfRoundTimerTick = true;
    RoundHasEndedEvent.Broadcast();
}

bool URoundState::IsThereEndOfRoundDelayLeft() const
{
    return SecondsUntilNextRoundStarts > 0.0f;
}

#pragma endregion

#pragma region Overtime

void URoundState::HandleOvertimeHasStarted()
{
    ShouldOvertimeTimerTick = true;
    ShouldStartRoundTimerTick = false;
    OvertimeHasStartedEvent.Broadcast();
}

#pragma endregion

#pragma region Post Match

void URoundState::HandleMatchHasEnded()
{
    ShouldMatchTimerTick = false;
    ShouldOvertimeTimerTick = false;
}

#pragma endregion

#pragma endregion
