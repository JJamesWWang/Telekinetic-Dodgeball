// Copyright 2021, James S. Wang, All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TDGameStateComponent.generated.h"

class UUIGameState;
class ATDGameState;

/**
 * @brief Base class for a component attached to TDGameState that is notified of
 * various match and round state changes. Also holds references to the
 * TDGameState and the UIGameState for convenience.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TD_API UTDGameStateComponent : public UActorComponent
{
    GENERATED_BODY()

protected:
    /**
     * @brief The game state super-component.
     */
    UPROPERTY()
    ATDGameState* TDGameState = nullptr;

    /**
     * @brief The game state that only the UI uses.
     */
    UPROPERTY()
    UUIGameState* UIGameState = nullptr;

    /**
     * @brief Callback for when match state has been set to WaitingToStart.
     */
    virtual void HandleMatchIsWaitingToStart();

    /**
     * @brief Callback for when match state has been set to InProgress.
     */
    virtual void HandleMatchHasStarted();

    /**
     * @brief Callback for when round state has been set to WaitingPreRound.
     */
    virtual void HandleRoundIsWaitingToStart();

    /**
     * @brief Callback for when round state has been set to RoundInProgress.
     */
    virtual void HandleRoundHasStarted();

    /**
     * @brief Callback for when round state has been set to WaitingPostRound.
     */
    virtual void HandleRoundHasEnded();

    /**
     * @brief Callback for when round state has been set to RoundInOvertime.
     */
    virtual void HandleOvertimeHasStarted();

    /**
     * @brief Callback for after overtime has ended, but before the match has
     * ended.
     */
    virtual void HandleOvertimeHasEnded();

    /**
     * @brief Callback for when match state has been set to WaitingPostMatch.
     */
    virtual void HandleMatchHasEnded();

    friend class ATDGameState;
};
