// Copyright 2021, James S. Wang, All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RoundStateMachine.generated.h"

class UTeamState;
class URoundState;
class ATDGameState;
class ATDGameMode;

DECLARE_LOG_CATEGORY_EXTERN(LogTDRSM, Log, All);

/**
 * @brief Possible states for a round as FNames
 */
namespace RoundState
{
    /**
     * @brief The round is waiting (for the match or itself) to start.
     */
    extern TD_API const FName WaitingPreRound;

    /**
     * @brief The round started and is now in progress.
     */
    extern TD_API const FName RoundInProgress;

    /**
     * @brief We could not determine a winner when the timer reached 0 and we
     * are now in extended play time.
     */
    extern TD_API const FName RoundInOvertime;

    /**
     * @brief The round ended and is waiting for the next one to start.
     */
    extern TD_API const FName WaitingPostRound;
}

/**
 * @brief Handles transitions between round states.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TD_API URoundStateMachine : public UActorComponent
{
    GENERATED_BODY()

#pragma region Initialization

public:
    /**
     * @brief Initializes references to the GameState.
     */
    void InitState(ATDGameState* GameState);

private:
    UPROPERTY()
    ATDGameMode* TDGameMode = nullptr;

    UPROPERTY()
    ATDGameState* TDGameState = nullptr;

    UPROPERTY()
    URoundState* State = nullptr;

#pragma endregion

#pragma region Round State

public:
    /**
     * @brief Gets the current round state.
     */
    FName GetRoundState() const;

    /**
     * @brief Sets the round state and calls appropriate callbacks.
     * @param NewState The new round state to switch to.
     */
    UFUNCTION(Exec)
    void SetRoundState(const FName& NewState);

private:
    /**
     * @brief The current round state. Usually one of WaitingPreRound,
     * RoundInProgress, Overtime, or WaitingPostRound, but is None until the
     * match starts since it's Transient.
     */
    UPROPERTY(Transient, VisibleInstanceOnly)
    FName RoundState;

    /**
     * @brief Calls the appropriate callbacks for the new round state.
     */
    void OnRoundStateSet(const FName& NewState);

#pragma region State Change Events

    DECLARE_EVENT(URoundStateMachine, FRoundIsWaitingToStartEvent);

    DECLARE_EVENT(URoundStateMachine, FRoundHasStartedEvent);

    DECLARE_EVENT(URoundStateMachine, FRoundHasEndedEvent);

    DECLARE_EVENT(URoundStateMachine, FOvertimeHasStartedEvent);

public:
    /**
     * @brief Event for when round state is set to WaitingPreRound.
     */
    FRoundIsWaitingToStartEvent RoundIsWaitingToStartEvent;

    /**
     * @brief Event for when round state is set to RoundInProgress.
     */
    FRoundHasStartedEvent RoundHasStartedEvent;

    /**
     * @brief Event for when round state is set to WaitingPostRound.
     */
    FRoundHasEndedEvent RoundHasEndedEvent;

    /**
     * @brief Event for when round state is set to RoundInOvertime.
     */
    FOvertimeHasStartedEvent OvertimeHasStartedEvent;

#pragma endregion

#pragma region Starting Round

public:
    /**
     * @brief Gets how long to wait before starting a round.
     */
    uint8 GetRoundStartDelayInSeconds() const;

protected:
    /**
     * @brief How long to wait before starting a round.
     */
    UPROPERTY(EditAnywhere)
    uint8 RoundStartDelayInSeconds = 5;

private:
    /**
     * @brief Callback for when round state has been set to WaitingPreRound.
     */
    void HandleRoundIsWaitingToStart();

    /**
     * @brief Check for whether the round should start.
     */
    bool ReadyToStartRound() const;

    /**
     * @brief Transitions round state from None to WaitingPreRound.
     */
    void StartRound();

#pragma endregion

#pragma region Round In Progress

private:
    /**
     * @brief Callback for when round state has been set to RoundInProgress.
     */
    void HandleRoundHasStarted();

    /**
     * @brief Check for whether the round should end.
     */
    bool ReadyToEndRound() const;

#pragma endregion

#pragma region Ending Round

private:
    /**
     * @brief Transitions round state from RoundInProgress to WaitingPostRound.
     */
    void EndRound();

    /**
     * @brief Check for whether the next round should start.
     */
    bool ReadyToStartNextRound() const;

    /**
     * @brief Transitions round state from WaitingPostRound to WaitingPreRound.
     */
    void StartNextRound();

#pragma endregion

#pragma region Post Round

public:
    /**
     * @brief Gets how long to wait before starting the next round.
     */
    uint8 GetEndOfRoundDelayInSeconds() const;

protected:
    /**
     * @brief How long to wait before starting the next round.
     */
    UPROPERTY(EditAnywhere)
    uint8 EndOfRoundDelayInSeconds = 5;

private:
    /**
     * @brief Callback for when round state has been set to WaitingPostRound.
     */
    void HandleRoundHasEnded();

#pragma endregion

#pragma region Overtime

private:
    /**
     * @brief Check for whether to start overtime.
     */
    bool ReadyToStartOvertime() const;

    /**
     * @brief Transitions round state from RoundInProgress to WaitingPreRound.
     */
    void InterruptRoundWithOvertime();

    /**
     * @brief Transitions round state from WaitingPreRound to RoundInOvertime.
     */
    void StartOvertime();

    /**
     * @brief Callback for when round state has been set to RoundInOvertime.
     */
    void HandleOvertimeHasStarted();

    /**
     * There is no ReadyToEndOvertime() method because Overtime is the final
     * state of the Round State Machine. There are no transitions outside of
     * this state, internally. Instead, the GameMode should define its own
     * ReadyToEndOvertime() and EndOvertime() and then call this class's
     * HandleMatchHasEnded() to clean up the round state machine.
     */

    /**
     * @brief Cleans up the state machine assuming the match is over.
     */
    void HandleMatchHasEnded();

#pragma endregion

#pragma endregion
    /**
     * @brief The Round State Machine is an extension of ATDGameMode for the
     * sake of modularity.
     */
    friend class ATDGameMode;
};
