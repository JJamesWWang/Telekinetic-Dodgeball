// Copyright 2021, James S. Wang, All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "Components/ActorComponent.h"
#include "RoundState.generated.h"

class UUIGameState;
class URoundStateMachine;
DECLARE_LOG_CATEGORY_EXTERN(LogTDRS, Log, All);

class ATDGameMode;

/**
 * @brief State relating to transitioning between rounds.
 */
UCLASS()
class TD_API URoundState : public UActorComponent
{
    GENERATED_BODY()

#pragma region Initialization

public:
    URoundState();
    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    /**
     * @brief RoundState to RoundStateMachine is like GameState to GameMode.
     */
    UPROPERTY()
    URoundStateMachine* StateMachine = nullptr;

private:
    UPROPERTY()
    UUIGameState* UIGameState = nullptr;

#pragma endregion

#pragma region Round State

public:
    /**
     * @brief Tick is called by GameState for consistent ticks.
     */
    void MatchTick(float DeltaSeconds);

    /**
     * @brief Called by the game mode to set the round state. Calls appropriate
     * callbacks.
     * @param NewState The new round state to switch to.
     */
    void SetRoundState(FName NewState);

private:
    /**
     * @brief The current round state. Usually one of WaitingPreRound,
     * RoundInProgress, or WaitingPostRound, but is None until the match starts.
     */
    UPROPERTY(Transient, VisibleInstanceOnly, ReplicatedUsing=OnRep_RoundState)
    FName RoundState;

    /**
     * @brief Calls the appropriate callbacks for the new round state.
     */
    UFUNCTION()
    void OnRep_RoundState();

#pragma region Pre Round

    DECLARE_EVENT(URoundStateMachine, FRoundIsWaitingToStartEvent);

public:
    /**
     * @brief Event for when round state has been set to WaitingPreRound.
     */
    FRoundIsWaitingToStartEvent RoundIsWaitingToStartEvent;

    /**
     * @brief Check for whether the round start should still delay.
     */
    bool IsThereRoundStartDelayLeft() const;

protected:
    /**
     * @brief Callback for when round state has been set to WaitingPreRound.
     */
    void HandleRoundIsWaitingToStart();

private:
    /**
     * @brief How long to wait until the round starts.
     */
    UPROPERTY(VisibleInstanceOnly, Replicated)
    float SecondsUntilRoundStarts = 0.0f;

    /**
     * @brief Whether the round start timer should tick.
     */
    bool ShouldStartRoundTimerTick = false;

#pragma endregion

#pragma region Round In Progress

    DECLARE_EVENT(URoundStateMachine, FRoundHasStartedEvent);

public:
    /**
     * @brief Event for when round state has been set to RoundInProgress.
     */
    FRoundHasStartedEvent RoundHasStartedEvent;

private:
    /**
     * @brief Whether the match timer should tick.
     */
    bool ShouldMatchTimerTick = false;

    /**
     * @brief Callback for when round state has been set to RoundInProgress.
     */
    void HandleRoundHasStarted();

#pragma endregion

#pragma region Post Round

    DECLARE_EVENT(URoundStateMachine, FRoundHasEndedEvent);

public:
    /**
     * @brief Event for when round state has been set to WaitingPostRound.
     */
    FRoundHasEndedEvent RoundHasEndedEvent;

    /**
     * @brief Check for whether the round end should still delay.
     */
    bool IsThereEndOfRoundDelayLeft() const;

protected:
    /**
     * @brief Callback for when round state has been set to WaitingPostRound.
     */
    void HandleRoundHasEnded();

private:
    /**
     * @brief How long to wait until advancing to the next (pre) round.
     */
    UPROPERTY(VisibleInstanceOnly, Replicated)
    float SecondsUntilNextRoundStarts = 0.0f;

    /**
     * @brief Whether the end of round timer should tick.
     */
    bool ShouldEndOfRoundTimerTick = false;

#pragma endregion

#pragma region Overtime

    DECLARE_EVENT(URoundStateMachine, FOvertimeHasStartedEvent);

public:
    /**
     * @brief Set when round state has been set to RoundInOvertime.
     */
    FOvertimeHasStartedEvent OvertimeHasStartedEvent;

private:
    /**
     * @brief How long overtime has lasted.
     */
    UPROPERTY(VisibleInstanceOnly, Replicated)
    float OvertimeSecondsPassed = 0;

    /**
     * @brief Whether the overtime timer should tick.
     */
    bool ShouldOvertimeTimerTick = false;

    /**
     * @brief Callback for when round state has been set to RoundInOvertime.
     */
    void HandleOvertimeHasStarted();

    /**
     * @brief Callback for when the match, and thus overtime, has ended.
     */
    void HandleMatchHasEnded();

#pragma endregion

#pragma endregion

    /**
     * @brief The round state is an extension of the game state.
     */
    friend class ATDGameState;
    friend class ADebugGameStateHUD;
};
