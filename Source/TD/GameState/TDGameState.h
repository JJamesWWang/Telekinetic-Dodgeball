// Copyright 2021, James S. Wang, All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "Components/TDGameStateComponent.h"
#include "UIGameState.h"
#include "GameFramework/GameState.h"

#include "TDGameState.generated.h"

class AOrb;
class UOrbState;
class UTeamState;
class UScoreState;
class URoundState;
class ATDGameMode;
struct FTeam;

DECLARE_LOG_CATEGORY_EXTERN(LogTDGS, Log, All);

/**
 * @brief Contains all of the game's state, most of which is replicated.
 */
UCLASS()
class TD_API ATDGameState : public AGameState
{
    GENERATED_BODY()

#pragma region Initialization

public:
    /**
     * @brief Creates the UIGameState and fills the teams.
     */
    ATDGameState();

    /**
     * @brief Creates all of the UTDGameStateComponents.
     */
    void CreateStateComponents();

    /**
     * @brief Adds the created UTDGameStateComponents to StateComponents.
     */
    void AddStateComponents();

    /**
     * @brief Sets the TDGameState and UIGameState for all StateComponents.
     */
    void InitStateComponents();

    /**
     * @brief Binds to round state events.
     */
    virtual void BeginPlay() override;

    /**
     * @brief Initializes the game mode.
     */
    virtual void ReceivedGameModeClass() override;

    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    /**
     * @brief Gets the component relating to round state. You probably won't
     * need to use this since it's for the RoundStateMachine.
     */
    URoundState* GetRoundStateComponent() const;

    /**
     * @brief Gets the component relating to player teams.
     */
    UTeamState* GetTeamStateComponent() const;

    /**
     * @brief Gets the component relating to scores.
     */
    UScoreState* GetScoreStateComponent() const;

    /**
     * @brief Gets the component relating to orbs.
     */
    UOrbState* GetOrbStateComponent() const;

private:
    /**
     * @brief The game mode has match configuration information.
     */
    UPROPERTY()
    ATDGameMode* TDGameMode = nullptr;

    /**
     * @brief Component relating to round state.
     */
    UPROPERTY(VisibleAnywhere)
    URoundState* RoundStateComponent = nullptr;

    /**
     * @brief Component relating to player teams.
     */
    UPROPERTY(VisibleAnywhere)
    UTeamState* TeamStateComponent = nullptr;

    /**
     * @brief Component relating to scores.
     */
    UPROPERTY(VisibleAnywhere)
    UScoreState* ScoreStateComponent = nullptr;

    /**
     * @brief Component relating to orbs.
     */
    UPROPERTY(VisibleAnywhere)
    UOrbState* OrbStateComponent = nullptr;

    /**
     * @brief Array of all UTDGameStateComponents that are listening for
     * various match and round state events.
     */
    UPROPERTY()
    TArray<UTDGameStateComponent*> StateComponents;

#pragma endregion

#pragma region Match State

protected:
    /**
     * @brief Ticks various timers.
     */
    virtual void Tick(float DeltaSeconds) override;
    virtual void OnRep_MatchState() override;

#pragma region Pre Match

protected:
    /**
     * @brief Callback for when match state has been set to WaitingToStart.
     */
    virtual void HandleMatchIsWaitingToStart() override;

#pragma endregion

#pragma region Match In Progress

public:
    /**
     * @brief Check for whether there's match time left.
     */
    bool IsThereMatchTimeLeft() const;

protected:
    /**
     * @brief Callback for when match state has been set to InProgress.
     */
    virtual void HandleMatchHasStarted() override;

private:
    /**
     * @brief How many seconds until the match should end.
     */
    UPROPERTY(VisibleInstanceOnly, Replicated)
    float SecondsUntilMatchEnds = 0.0f;

#pragma region Round State

public:
    /**
     * @brief Gets the current round state.
     */
    FName GetRoundState() const;

#pragma region Pre Round

private:
    /**
     * @brief Callback for when round state has been set to WaitingPreRound.
     */
    UFUNCTION()
    void HandleRoundIsWaitingToStart();

#pragma endregion

#pragma region Round In Progress

private:
    /**
     * @brief Callback for when round state has been set to RoundInProgress.
     */
    UFUNCTION()
    void HandleRoundHasStarted();

#pragma endregion

#pragma region Post Round

private:
    /**
     * @brief Callback for when round state has been set to WaitingPostRound.
     */
    UFUNCTION()
    void HandleRoundHasEnded();

#pragma endregion

#pragma region Overtime

private:
    /**
     * @brief Callback for when round state has been set to RoundInOvertime.
     */
    UFUNCTION()
    void HandleOvertimeHasStarted();

#pragma endregion

#pragma endregion

#pragma endregion

#pragma region Post Match

public:
    /**
     * @brief Check for whether the match should still delay restarting.
     */
    bool IsThereMatchRestartDelayLeft() const;

    /**
     * @brief Called by the TDGameMode to tell this class to call
     * HandleOvertimeHasEnded().
     */
    void NotifyOvertimeHasEnded();

protected:
    /**
     * @brief Callback for when overtime has ended, but before the match is
     * over.
     */
    virtual void HandleOvertimeHasEnded();

    /**
     * @brief Callback for when match state has been set to WaitingPostMatch.
     */
    virtual void HandleMatchHasEnded() override;

private:
    /**
     * @brief Whether the timer for restarting the match should tick.
     */
    bool ShouldRestartMatchTimerTick = false;

    /**
     * @brief How much time to wait until restarting the match.
     */
    UPROPERTY(VisibleInstanceOnly, Replicated)
    float SecondsUntilMatchRestarts = 0.0f;

#pragma endregion

#pragma endregion

#pragma region Gameplay Requests

public:
    /**
     * @brief Called when an Orb impacts something else.
     */
    void HandleOrbImpact(AOrb* InstigatorOrb, const FHitResult& Hit) const;

    void PlayLocalCue(USoundBase* Sound, FVector* Location = nullptr) const;
    UFUNCTION(NetMulticast, Unreliable)
    void PlayReplicatedCueAtLocation(USoundBase* Sound,
        const FVector& Location);
    void PlayReplicatedCueAtLocation_Implementation(USoundBase* Sound,
        const FVector& Location);

private:
    FPlayerAudioSettings GetAudioSettings() const;

#pragma endregion

#pragma region UI Game State

public:
    /**
     * @brief Gets the game state that the UI only needs to know about.
     */
    UUIGameState* GetUIGameState() const;

    /**
     * @brief Sets the result of a player's request, which will notify the UI.
     * @param Result The result of the player's request.
     * @param Message An optional feedback message to send with the result.
     */
    void SetPlayerRequestResult(const EPlayerRequestResult Result,
        const FString& Message = "") const;

protected:
    /**
     * @brief The information that the UI needs is separated into its own state
     * to make sure it is exposed only to the information that it needs.
     * @see UUIGameState
     */
    UPROPERTY(VisibleAnywhere, Category = "UI Game State")
    UUIGameState* UIGameState = nullptr;

#pragma endregion

    friend class ADebugGameStateHUD;
};
