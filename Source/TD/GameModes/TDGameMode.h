// Copyright 2021, James S. Wang, All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "LobbySystem/LSGameMode.h"
#include "TDGameMode.generated.h"

struct FTeam;
class URoundStateMachine;
class ATDGameState;
class ATDPlayerState;
class ATeamPlayerStart;
enum class ETeamIndex : uint8;
class ATDController;
class ATDCharacter;
class AOrb;
class AGameRules;

DECLARE_LOG_CATEGORY_EXTERN(LogTDGM, Log, All);

/**
 * @brief Handles all gameplay rules and exists only on the server.
 *
 * Here's a rough outline for how TD plays: The standard game mode is a 3v3
 * elimination game. Players may each spawn a single orb, but they can re-cast
 * it, causing the old orb to despawn. Players may then push or pull these orbs
 * to try to make them collide into opponent players to eliminate them. When a
 * team eliminates all opposing players, they score a point. When a team scores
 * a point, play is stopped and all player positions and orbs are reset. The
 * team with the most points after three minutes wins.
 */
UCLASS()
class TD_API ATDGameMode : public ALSGameMode
{
    GENERATED_BODY()

#pragma region Match Initialization

public:
    /**
     * @brief Sets various classes and makes sure players start as spectators.
     */
    ATDGameMode();

    virtual void PreInitializeComponents() override;

    /**
     * @brief Initializes the TDGameState and stores it as a pointer.
     */
    virtual void InitGameState() override;

protected:
    /**
     * @brief The game rules class to spawn into the world.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, NoClear, Category = "Game Rules")
    TSubclassOf<AGameRules> GameRulesClass = nullptr;

    /**
     * @brief Binds to round state events.
     */
    virtual void BeginPlay() override;

private:
    /**
     * @brief The game's state.
     */
    UPROPERTY()
    ATDGameState* TDGameState = nullptr;

    /**
     * @brief The sub-component of the match state, round state.
     */
    UPROPERTY(VisibleAnywhere)
    URoundStateMachine* RSM = nullptr;

    /**
     * @brief Spawns the Game Rules actor and initializes its game state.
     */
    void InitGameRules();

#pragma endregion

#pragma region Player Connection

public:
    /**
     * @brief Initializes the player before spawning them in.
     */
    virtual FString InitNewPlayer(APlayerController* NewPlayerController,
        const FUniqueNetIdRepl& UniqueId, const FString& Options,
        const FString& Portal) override;

    /**
     * @brief Spawns in a pawn for the NewPlayer at the StartSpot.
     */
    virtual void RestartPlayerAtPlayerStart(AController* NewPlayer,
        AActor* StartSpot) override;

    /**
     * @brief Used by the superclass to find a place to spawn the player.
     */
    virtual AActor*
    ChoosePlayerStart_Implementation(AController* Player) override;

    /**
     * @brief Used by the superclass to determine whether the player should
     * spawn where the initially spawned.
     */
    virtual bool ShouldSpawnAtStartSpot(AController* Player) override;

#pragma endregion

#pragma region Match State

public:
    /**
     * @brief Checks if the match and round states should change states.
     */
    virtual void Tick(float DeltaSeconds) override;

#pragma region Starting Match

protected:
    /**
     * @brief Check for whether the match should start.
     */
    virtual bool ReadyToStartMatch_Implementation() override;

    UPROPERTY(EditAnywhere, Category = "Starting Match")
    bool ShouldStartImmediately = false;

#pragma endregion

#pragma region Match In Progress

public:
    /**
     * @brief Gets how long a match will last without interruptions or pauses.
     */
    uint16 GetMatchLengthInSeconds() const;

protected:
    /**
     * @brief How long a match will last without interruptions or pauses.
     */
    UPROPERTY(EditAnywhere)
    uint16 MatchLengthInSeconds = 180;

    /**
     * @brief Callback for when match state has been set to InProgress.
     */
    virtual void HandleMatchHasStarted() override;

#pragma region Round State

private:
    /**
     * @brief Callback for when round state has been set to WaitingPreRound.
     */
    UFUNCTION()
    void HandleRoundIsWaitingToStart();

    /**
     * @brief Spawns all players that are on an active team.
     */
    void SpawnActivePlayers();

    /**
     * @brief Stops players from being able to move.
     */
    void DisablePlayerMovement() const;

    /**
     * @brief Callback for when round state has been set to RoundInProgress.
     */
    UFUNCTION()
    void HandleRoundHasStarted();

    /**
     * @brief Enables players to start moving.
     */
    void EnablePlayerMovement() const;

    /**
     * @brief Callback for when round state has been set to WaitingPostRound.
     */
    UFUNCTION()
    void HandleRoundHasEnded();

    /**
     * @brief Callback for when round state has been set to RoundInOvertime.
     */
    UFUNCTION()
    void HandleOvertimeHasStarted();

    /**
     * @brief Called by both HandleRoundHasStarted and HandleOvertimeHasStarted.
     */
    void HandlePlayHasStarted();

#pragma endregion

#pragma region Ending Match

protected:
    /**
     * @brief Check for whether the match should end.
     */
    virtual bool ReadyToEndMatch_Implementation() override;

private:
    /**
     * @brief Check for whether overtime, and thus the match, should end.
     */
    bool ReadyToEndOvertime() const;

    /**
     * @brief Ends overtime, and then ends the match.
     */
    void EndOvertime();

    /**
     * @brief Callback for when overtime has ended, but before the match has
     * ended.
     */
    void HandleOvertimeHasEnded();

#pragma endregion

#pragma endregion

#pragma region Post Match

public:
    /**
     * @brief Returns how long to delay restarting the match once it's over.
     */
    uint8 GetMatchRestartDelayInSeconds() const;

protected:
    /**
     * @brief How long to wait before restarting the match.
     */
    UPROPERTY(EditAnywhere)
    uint8 MatchRestartDelayInSeconds = 5;

    /**
     * @brief Callback for when match state has been set to WaitingPostMatch.
     */
    virtual void HandleMatchHasEnded() override;

    virtual void HandleLeavingMap() override;

private:
    /**
     * @brief Check for whether the match should restart.
     */
    bool ReadyToRestartMatch() const;

#pragma endregion

#pragma region Match Failure

public:
    virtual void AbortMatch() override;

protected:
    virtual void HandleMatchAborted() override;
    virtual void
    HandleDisconnect(UWorld* InWorld, UNetDriver* NetDriver) override;

#pragma endregion

#pragma endregion

#pragma endregion

#pragma region Gameplay Requests

public:
    /**
     * @brief Called by Orb when it impacts something else.
     */
    void HandleOrbImpact(AOrb* InstigatorOrb, const FHitResult& Hit);

private:
    /**
     * @brief Eliminates a player by destroying their character.
     * @param Player The player to be eliminated.
     */
    void EliminatePlayer(ATDCharacter* Player);

#pragma endregion

#pragma region Player Requests

public:
    /**
     * Make sure to call @see SetPlayerRequestResult() upon completion.
     */

    /**
     * @brief Switches a player from their previous team to the specified one.
     * @param Player The PlayerController that is requesting to switch teams.
     * @param Team The team enum to switch to.
     */
    void SwitchPlayerToTeam(ATDController* Player,
        const ETeamIndex Team) const;

#pragma endregion
};
