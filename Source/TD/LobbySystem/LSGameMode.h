// Copyright 2021, James S. Wang, All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "HostLobbySettings.h"

#include "LSGameMode.generated.h"

/**
 * @brief Handles players logging in and various game states. Exists only on the
 * server.
 */
UCLASS(minimalapi)
class ALSGameMode : public AGameMode
{
    GENERATED_BODY()

public:
    /**
     * @brief Sets the initial game settings from the options string.
     * @param MapName Name of the map, unused.
     * @param Options Options String, used to initialize @link Settings.
     * @param ErrorMessage Set to throw an error with a message.
     */
    virtual void InitGame(const FString& MapName, const FString& Options,
        FString& ErrorMessage) override;

    /**
     * @brief Determines whether the match should start; if there are at
     * least MinPlayers in the match.
     * @return Whether the match should start.
     */
    virtual bool ReadyToStartMatch_Implementation() override;

    /**
     * @brief Starts the match; overriden to reset @link StartMatchDelay helper
     * variables.
     */
    virtual void StartMatch() override;

protected:
    /**
     * @brief Various settings for this match, set by the player when they host
     * a lobby.
     */
    FHostLobbySettings Settings;

    /**
     * @brief A delay before starting if @link bDelayedStart is false. This is
     * so that you can start the game automatically when enough players have
     * joined, but you still want to delay the match from starting for a
     * specific amount of time.
     */
    UPROPERTY(EditAnywhere, Category = "Base Game Mode")
    float StartMatchDelay = 3.0f;

private:
    /**
     * @brief Whether the match delay has passed if using @link StartMatchDelay.
     */
    bool HasStartMatchDelayPassed = false;

    /**
     * @brief The handle to the timer used for the @link StartMatchDelay.
     */
    FTimerHandle StartMatchDelayTimerHandle;
};
