// Copyright 2021, James S. Wang, All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "TDTypes.h"
#include "Components/ActorComponent.h"
#include "UIGameState.generated.h"

/**
 * @brief Enum representing the result a player's request (usually an RPC).
 */
UENUM()
enum class EPlayerRequestResult : uint8
{
    Success,
    Error,
};

/**
 * @brief What the server sends down to the client, detailing the results of
 * their request.
 */
USTRUCT(BlueprintType)
struct TD_API FPlayerRequest
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    EPlayerRequestResult Result;

    UPROPERTY(BlueprintReadOnly)
    FString Message;
};

/**
 * @brief Subset of the Game State plus whatever other information that the UI
 * needs to know about.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TD_API UUIGameState : public UActorComponent
{
    GENERATED_BODY()

    /**
     * Declare variable and corresponding delegate template.
     * Replace VariableType, VariableName, and OnRoundStateUpdated.
    
public:
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDelegateName, VariableType, OldVariableName, VariableType, NewVariableName);
    
    UPROPERTY(BlueprintAssignable, Category = "UI Data Events")
    FDelegateName DelegateName;

    void SetVariableName(VariableType NewVariableName);

protected:
    UPROPERTY(BlueprintReadOnly)
    VariableType VariableName;

    */

#pragma region Delegate Declarations

public:
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMatchStateUpdated,
        const FName&, OldMatchState, const FName&, NewMatchState);

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMatchStarted);

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMatchEnded);

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRoundStateUpdated,
        const FName&, OldRoundState, const FName&, NewRoundState);

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRoundIsWaitingToStart);

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRoundStarted);

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRoundEnded);

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOvertimeStarted);

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTeamsChanged,
        const TArray<FTeam>&, OldTeams, const TArray<FTeam>&,
        NewTeams);

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
        FOnMatchTimeInSecondsUpdated, float, OldMatchTimeInSeconds, float,
        NewMatchTimeInSeconds);

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
        FOnSecondsUntilRoundStartsUpdated, float, OldSecondsUntilRoundStarts,
        float, NewSecondsUntilRoundStarts);

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTeamScoresUpdated,
        const TArray<uint8>&, OldTeamScores, const TArray<uint8>&,
        NewTeamScores);

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerRequestCompleted,
        const FPlayerRequest&, OldPlayerRequest, const FPlayerRequest&,
        NewPlayerRequest);

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
        FOnSecondsUntilMatchRestartsUpdated, const float,
        OldSecondsUntilMatchRestarts, const float,
        NewSecondsUntilMatchRestarts);

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRoundWinningTeamsUpdated,
        const TArray<ETeamIndex>&, OldRoundWinningTeams,
        const TArray<ETeamIndex>&, NewRoundWinningTeams);

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMatchWinningTeamsUpdated,
        const TArray<ETeamIndex>&, OldMatchWinningTeams,
        const TArray<ETeamIndex>&, NewMatchWinningTeams);

private:
    /**
     * @brief Broadcasts match state changes.
     */
    void BroadcastNewMatchState() const;

    /**
     * @brief Broadcasts round state changes.
     */
    void BroadcastNewRoundState() const;

#pragma endregion

#pragma region Delegate Members

public:
    UPROPERTY(BlueprintAssignable, Category = "UI Data Events")
    FOnMatchStateUpdated OnMatchStateUpdated;

    UPROPERTY(BlueprintAssignable, Category = "UI Data Events")
    FOnMatchStarted OnMatchStarted;

    UPROPERTY(BlueprintAssignable, Category = "UI Data Events")
    FOnMatchEnded OnMatchEnded;

    UPROPERTY(BlueprintAssignable, Category = "UI Data Events")
    FOnRoundStateUpdated OnRoundStateUpdated;

    UPROPERTY(BlueprintAssignable, Category = "UI Data Events")
    FOnRoundIsWaitingToStart OnRoundIsWaitingToStart;

    UPROPERTY(BlueprintAssignable, Category = "UI Data Events")
    FOnRoundStarted OnRoundStarted;

    UPROPERTY(BlueprintAssignable, Category = "UI Data Events")
    FOnRoundEnded OnRoundEnded;

    UPROPERTY(BlueprintAssignable, Category = "UI Data Events")
    FOnOvertimeStarted OnOvertimeStarted;

    UPROPERTY(BlueprintAssignable, Category = "UI Data Events")
    FOnTeamsChanged OnTeamsChanged;

    UPROPERTY(BlueprintAssignable, Category = "UI Data Events")
    FOnMatchTimeInSecondsUpdated OnMatchTimeInSecondsUpdated;

    UPROPERTY(BlueprintAssignable, Category = "UI Data Events")
    FOnSecondsUntilRoundStartsUpdated OnSecondsUntilRoundStartsUpdated;

    UPROPERTY(BlueprintAssignable, Category = "UI Data Events")
    FOnTeamScoresUpdated OnTeamScoresUpdated;

    UPROPERTY(BlueprintAssignable, Category = "UI Data Events")
    FOnSecondsUntilMatchRestartsUpdated OnSecondsUntilMatchRestartsUpdated;

    UPROPERTY(BlueprintAssignable, Category = "UI Data Events")
    FOnRoundWinningTeamsUpdated OnRoundWinningTeamsUpdated;

    UPROPERTY(BlueprintAssignable, Category = "UI Data Events")
    FOnMatchWinningTeamsUpdated OnMatchWinningTeamsUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Player Requests")
    FOnPlayerRequestCompleted OnPlayerRequestCompleted;

#pragma endregion

#pragma region Setters

public:
    void SetMatchState(const FName& NewMatchState);
    void SetRoundState(const FName& NewRoundState);
    void SetTeams(const TArray<FTeam>& NewTeams);
    void SetPlayerRequest(const FPlayerRequest& NewPlayerRequest);
    void SetMatchTimeInSeconds(const float NewSecondsUntilMatchEnds);
    void SetSecondsUntilRoundStarts(const float NewSecondsUntilRoundStarts);
    void SetTeamScores(const TArray<uint8>& NewTeamScores);
    void SetSecondsUntilMatchRestarts(const float NewSecondsUntilMatchRestarts);
    void SetRoundWinningTeams(const TArray<ETeamIndex>& NewRoundWinningTeams);
    void SetMatchWinningTeams(const TArray<ETeamIndex>& NewMatchWinningTeams);

#pragma endregion

#pragma region Variables

public:
    /**
     * @brief Gets the team that won the round.
     */
    UFUNCTION(BlueprintCallable)
    ETeamIndex GetWinnerTeam() const;

    /**
     * @brief Converts an ETeamIndex to an FString.
     */
    UFUNCTION(BlueprintCallable)
    FString TeamIndexToString(const ETeamIndex TeamIndex) const;

    UFUNCTION(BlueprintCallable)
    FText TeamsToString(const TArray<ETeamIndex>& TeamIndices);

protected:
    UPROPERTY(BlueprintReadOnly)
    FName MatchState;

    UPROPERTY(BlueprintReadOnly)
    FName RoundState;

    UPROPERTY(BlueprintReadOnly, Category = "UI Data")
    TArray<FTeam> Teams;

    UPROPERTY(BlueprintReadOnly, Category = "Game HUD")
    float MatchTimeInSeconds = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Game HUD")
    float SecondsUntilRoundStarts = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Game HUD")
    TArray<uint8> TeamScores;

    UPROPERTY(BlueprintReadOnly, Category = "Game HUD")
    float SecondsUntilMatchRestarts = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Game HUD")
    TArray<ETeamIndex> RoundWinningTeams;

    UPROPERTY(BlueprintReadOnly, Category = "Game HUD")
    TArray<ETeamIndex> MatchWinningTeams;

    /**
     * @brief Information about the player's most recent request.
     */
    UPROPERTY(BlueprintReadOnly, Category = "Player Requests")
    FPlayerRequest PlayerRequest;

#pragma endregion
};
