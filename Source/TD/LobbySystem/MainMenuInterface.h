// Copyright 2021, James S. Wang, All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "HostLobbySettings.h"
#include "UObject/Interface.h"

#include "MainMenuInterface.generated.h"

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UMainMenuInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 * @brief Interface for interacting with the LobbySystem.
 * It contains methods for loading maps that are specific to each game so that
 * the LobbySystem can easily load these without knowing them beforehand.
 * It also provides methods for hosting and joining lobbies that are general to
 * all multiplayer games.
 */
class TD_API IMainMenuInterface
{
    GENERATED_BODY()

public:
    /**
     * @brief Loads the main menu screen, presumably from any game map.
     */
    UFUNCTION(BlueprintCallable, Category = "Main Menu Interface")
    virtual void LoadMainMenuMap() = 0;

    /**
     * @brief Loads the single player practice map, presumably from the main
     * menu.
     */
    UFUNCTION(BlueprintCallable, Category = "Main Menu Interface")
    virtual void LoadPracticeMap() = 0;

    /**
     * @brief Loads the multiplayer map as a server so that others can join,
     * presumably from the main menu.
     */
    virtual void LoadMultiplayerMap() = 0;

    /**
     * @brief Create a new lobby with the player's specified settings. If
     * successful, should transfer the player to the game map.
     * @param LobbySettings Custom settings struct for the lobby.
     */
    UFUNCTION(BlueprintCallable, Category = "Main Menu Interface")
    virtual void HostLobby(const FHostLobbySettings& LobbySettings) = 0;

    /**
     * @brief Call a search for currently active lobbies.
     */
    UFUNCTION(BlueprintCallable, Category = "Main Menu Interface")
    virtual void SearchLobbies() = 0;

    /**
     * @brief Joins the lobby at the index specified and transfers the player.
     * @param LobbyIndex The index of the lobby in the search results.
     */
    UFUNCTION(BlueprintCallable, Category = "Main Menu Interface")
    virtual void JoinLobby(int32 LobbyIndex) = 0;

    /**
     * @brief Destroys the current session if it exists.
     */
    UFUNCTION(BlueprintCallable, Category = "Main Menu Interface")
    virtual void DestroySession() = 0;
};
