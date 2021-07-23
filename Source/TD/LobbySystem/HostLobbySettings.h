// Copyright 2021, James S. Wang, All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/GameplayStatics.h"

#include "HostLobbySettings.generated.h"

/**
 * @brief Stores all player defined settings for creating lobbies.
 */
USTRUCT(BlueprintType)
struct TD_API FHostLobbySettings
{
    GENERATED_BODY()

    /**
     * @brief The name of the lobby, used for SessionName.
     */
    UPROPERTY(BlueprintReadWrite, Category = "Lobby Params")
    FString LobbyName = "Dedicated Server";

    /**
     * @brief Whether the lobby should only be open to (Steam) friends.
     */
    UPROPERTY(BlueprintReadWrite, Category = "Lobby Params")
    bool IsFriendsOnly = false;

    /**
     * @brief The minimum number of players that have to join the lobby before
     * the game can start.
     */
    UPROPERTY(BlueprintReadWrite, Category = "Lobby Params")
    int32 MinPlayers = 2;

    /**
     * @brief The maximum number of players that can join this lobby.
     */
    UPROPERTY(BlueprintReadWrite, Category = "Lobby Params")
    int32 MaxPlayers = 4;

    /**
     * @brief This struct is only meant to be initialized once by the host, so
     * keep track of this and only initialize if it hasn't been yet.
     */
    bool IsInitialized = false;

    /**
     * @brief Converts the values of this struct to arguments to be passed into
     * the Options String (?VariableName=VariableValue).
     * @return Settings in the format of the Options String.
     */
    FString ToOptionsString() const
    {
        return FString::Printf(
            TEXT("?LobbyName=%s?MinPlayers=%d?MaxPlayers=%d"),
            *LobbyName, MinPlayers, MaxPlayers);
    }

    /**
     * @brief Parses an Options string and retrieves the settings.
     * @param Options The Options String.
     */
    void ParseOptions(const FString& Options)
    {
        LobbyName = UGameplayStatics::ParseOption(Options, "Dedicated Server");
        MinPlayers = UGameplayStatics::GetIntOption(Options, "MinPlayers", 2);
        MaxPlayers = UGameplayStatics::GetIntOption(Options, "MaxPlayers", 4);
        IsInitialized = true;
    }
};
