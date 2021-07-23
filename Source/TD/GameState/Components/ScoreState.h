// Copyright 2021, James S. Wang, All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "TDGameStateComponent.h"
#include "ScoreState.generated.h"

class UUIGameState;
enum class ETeamIndex : uint8;
struct FTeam;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TD_API UScoreState : public UTDGameStateComponent
{
    GENERATED_BODY()

#pragma region Initialization

public:
    UScoreState();
    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    /**
     * @brief Fills the Teams array with all of the available teams.
     */
    void FillTeamScores();

#pragma endregion

#pragma region Game State Events
    /** @see UTDGameStateComponent */

protected:
    virtual void HandleRoundIsWaitingToStart() override;
    virtual void HandleRoundHasEnded() override;
    virtual void HandleOvertimeHasEnded() override;
    virtual void HandleMatchHasEnded() override;

private:
    uint8 GetTeamMaxScore() const;
    TArray<ETeamIndex> GetTeamIndicesWithScore(uint8 Score) const;

#pragma endregion

#pragma region Score State

private:
    /**
     * @brief Array for each team's scores, indexed by ETeamIndex.
     */
    UPROPERTY(ReplicatedUsing=OnRep_TeamScores)
    TArray<uint8> TeamScores;

    UPROPERTY(ReplicatedUsing=OnRep_RoundWinningTeams)
    TArray<ETeamIndex> RoundWinningTeams;

    UPROPERTY(ReplicatedUsing=OnRep_MatchWinningTeams)
    TArray<ETeamIndex> MatchWinningTeams;

    /**
     * @brief Sets the UIGameState's team scores.
     */
    UFUNCTION()
    void OnRep_TeamScores();

    /**
     * @brief Sets the UIGameState's round winning teams.
     */
    UFUNCTION()
    void OnRep_RoundWinningTeams();

    /**
     * @brief Sets the UIGameState's match winning teams.
     */
    UFUNCTION()
    void OnRep_MatchWinningTeams();

    /**
     * @brief Adds a point to all passed in teams.
     * @param Teams The teams to add a point to.
     */
    void AddScoreToTeams(const TArray<FTeam>& Teams);

    /**
     * @brief Adds a point to a single team.
     * @param Team The team to add a point to.
     */
    void AddScoreToTeam(const FTeam& Team);

#pragma endregion

#pragma region Score Queries

public:
    /**
     * @brief Check for whether a team is strictly winning (no ties for winner).
     */
    bool IsATeamStrictlyWinning() const;

#pragma endregion
    friend class ATDGameState;
    friend class ADebugGameStateHUD;
};
