// Copyright 2021, James S. Wang, All rights reserved.

/**
 * This file is for commonly referenced types.
 */

#pragma once

#include "CoreMinimal.h"

#include "TDTypes.generated.h"

class ATDPlayerState;

/**
 * @brief The enum of all gameplay abilities that can be activated through
 * input.
 */
UENUM()
enum class ETDAbilityInputID : uint8
{
    None,
    Confirm,
    Cancel,
    Cast,
    Push,
    Pull,
};

/**
 * @brief The enum of all teams that players and other actors can be on.
 */
UENUM(BlueprintType)
enum class ETeamIndex : uint8
{
    None,
    Blue,
    Orange,
    Spectator,
};

/**
 * @brief Returns whether the team is an active team, which means that players
 * on this team are actively playing TD (or will be).
 * @param Team The team to check.
 * @return Whether the team is active.
 */
inline bool IsActiveTeam(const ETeamIndex Team)
{
    return Team > ETeamIndex::None && Team < ETeamIndex::Spectator;
}

ENUM_RANGE_BY_FIRST_AND_LAST(ETeamIndex, ETeamIndex::None,
    ETeamIndex::Spectator);

/**
 * @brief Converts an ETeamIndex to an uint8.
 */
inline uint8 Int(const ETeamIndex Team)
{
    return static_cast<uint8>(Team);
}

/**
 * @brief Struct containing all information about a team.
 */
USTRUCT(BlueprintType)
struct TD_API FTeam
{
    GENERATED_BODY()

    FTeam()
    {
    }

    FTeam(const ETeamIndex Idx)
    {
        Index = Idx;
        IsActive = IsActiveTeam(Idx);
    }

    /**
     * @brief The index of this team in the teams array.
     */
    UPROPERTY(BlueprintReadOnly)
    ETeamIndex Index = ETeamIndex::None;

    /**
     * @brief An array of all of the players on this team.
     */
    UPROPERTY(BlueprintReadOnly)
    TArray<ATDPlayerState*> Players;

    /**
     * @brief Whether the team participates in the match.
     */
    UPROPERTY(BlueprintReadOnly)
    bool IsActive = false;
};

/**
 * @brief Converts an array of teams to an array of indices.
 * @param Teams An array of FTeams
 * @return FTeams mapped to ETeamIndex
 */
inline TArray<ETeamIndex> ConvertTeamsToIndices(const TArray<FTeam>& Teams)
{
    TArray<ETeamIndex> TeamIndices;
    for (const FTeam& Team : Teams)
    {
        TeamIndices.Emplace(Team.Index);
    }
    return TeamIndices;
}

/**
 * @brief Contains all of the settings regarding to gameplay that the player can
 * set.
 */
USTRUCT(BlueprintType)
struct TD_API FPlayerGameplaySettings
{
    GENERATED_BODY()

    /**
     * @brief Gets the name of the slot to save these settings save to.
     * @return The slot name.
     */
    static FString GetSlotName()
    {
        return "PLAYER_GAMEPLAY_SETTINGS";
    }

    /**
	 * @brief The player set mouse sensitivity, used when looking around.
	 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
    float Sensitivity = 1.0f;
};

USTRUCT(BlueprintType)
struct TD_API FPlayerAudioSettings
{
    GENERATED_BODY()

    static FString GetSlotName()
    {
        return "PLAYER_AUDIO_SETTINGS";
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Audio)
    float GameSFXVolume = 0.5f;
};
