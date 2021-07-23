// Copyright 2021, James S. Wang, All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "TDGameStateComponent.h"
#include "TDTypes.h"
#include "TeamState.generated.h"

class ATeamPlayerStart;
class UUIGameState;
enum class ETeamIndex : uint8;
class ATDPlayerState;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TD_API UTeamState : public UTDGameStateComponent
{
    GENERATED_BODY()

#pragma region Initialization

public:
    UTeamState();
    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    /**
     * @brief Fills the teams array for each team in ETeamIndex.
     */
    void FillTeams();

    /**
     * @brief Sets all players to spawn at the 0th team player start by default.
     * @see NextTeamPlayerStartIndices
     */
    void FillNextTeamPlayerStartIndices();

    /**
     * @brief Finds and stores all team player starts for player spawning.
     * @see AllTeamPlayerStarts
     */
    void FindAllTeamPlayerStarts();

#pragma endregion

#pragma region Team State

public:
    /**
     * @brief Gets the array of teams.
     */
    const TArray<FTeam>& GetTeams() const;

    /**
     * @brief Gets the array of active teams.
     */
    TArray<FTeam> GetActiveTeams() const;

    TArray<FTeam> GetActiveTeamsWithPlayersLeft() const;

    TArray<FTeam> GetActiveTeamsWithNoEliminatedPlayers() const;

    /**
     * @brief Moves the player from their previous team to the specified team.
     * @param Player The PlayerState for the player that wants to switch teams.
     * @param TeamIndex The team enum to switch to.
     */
    void SwitchPlayerToTeam(ATDPlayerState* Player, const ETeamIndex TeamIndex);

private:
    /**
     * @brief Array of all FTeams.
     */
    UPROPERTY(ReplicatedUsing=OnRep_Teams, VisibleInstanceOnly)
    TArray<FTeam> Teams;

    /**
     * @brief Sets the UIGameState's teams.
     */
    UFUNCTION()
    void OnRep_Teams();

#pragma endregion

#pragma region Spawning Players

public:
    AActor* GetNextPlayerSpawn(AController* Player);

private:
    /**
     * @brief Each team has an array of team player starts; this is the array
     * of each team's player starts. Each team should have at least one team
     * player start, otherwise an error will occur if a player on that team
     * needs to spawn.
     */
    TArray<TArray<ATeamPlayerStart*>> AllTeamPlayerStarts;

    /**
     * @brief Each team has an array of team player starts; this is the array
     * of indices corresponding to those arrays that indicate which player
     * spawn should be chosen next to spawn the player. It is expected that
     * 0 <= NextTeamPlayerStartIndices[TeamIndex] < AllTeamPlayerStarts[
     *                                                  TeamIndex].Num()
     */
    TArray<uint8> NextTeamPlayerStartIndices;

#pragma endregion

#pragma region Team Queries

public:
    /**
     * @brief Check for whether each playing team has at least one player.
     */
    bool DoesEachActiveTeamHaveAtLeastOnePlayer() const;

    /**
     * @brief Check for whether a player on any team has been eliminated.
     */
    bool HasAPlayerBeenEliminated() const;

    /**
     * @brief Check for whether the provided team has players left.
     */
    bool DoesTeamHavePlayersLeft(const FTeam& Team) const;

    /**
     * @brief Check for whether the provided team has no eliminated players.
     */
    bool DoesTeamHaveNoEliminatedPlayers(const FTeam& Team) const;

    /**
     * @brief Check for whether there's one or no teams with players left.
     */
    bool AreThereFewerThanTwoTeamsWithPlayersLeft() const;

#pragma endregion

    friend class ATDGameState;
    friend class ADebugGameStateHUD;
};
