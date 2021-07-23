// Copyright 2021, James S. Wang, All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MainMenuInterface.h"

#include "LSGameInstance.generated.h"

struct FLobbyData;
class UMultiplayerMenu;
/**
 * @brief The base game instance that handles player interaction. Handles
 * loading maps and constructing UI widgets for the player to see. Also
 * is responsible for hosting and joining lobbies in multiplayer mode.
 */
UCLASS(Abstract)
class TD_API ULSGameInstance
    : public UGameInstance,
      public IMainMenuInterface
{
    GENERATED_BODY()

#pragma region Initialization

public:
    /**
     * @brief Finds the Online Subsystem and sets the SessionInterface. Then
     * binds this class to the delegates related to lobby hosting, searching,
     * and joining.
     */
    virtual void Init() override;

    /**
     * UCLASS's can't have pure virtual methods, so we override these and
     * call unimplemented(). Subclasses must implement these methods.
     */
    virtual void LoadPracticeMap() override;

    virtual void LoadMultiplayerMap() override;

protected:
    /**
     * @brief Binds methods of this class to delegates of the SessionInterface
     * such as OnCreateSessionCompleteDelegates. Override this if you want to
     * customize handling of the interface delegates.
     */
    virtual void BindToSessionInterfaceDelegates();

#pragma endregion

#pragma region MainMenu

public:
    /** Overridden method from @link IMainMenuInterface */
    virtual void LoadMainMenuMap() override;

    /** @see IMainMenuInterface */
    virtual void DestroySession() override;

protected:
    /**
     * @brief Callback for GEngine NetworkFailureEvent; loads player back to
     * main menu.
     */
    virtual void OnNetworkFailed(UWorld* World, UNetDriver* NetDriver,
        ENetworkFailure::Type FailureType, const FString& ErrorString);

private:
    /**
     * @brief Path to the MainMenu level.
     */
    const FString MainMenuMapDirectory =
        FString("/Game/LobbySystem/Maps/MainMenu");

#pragma endregion

#pragma region HostingLobbies

public:
    /** Overridden method from @link IMainMenuInterface */
    virtual void HostLobby(const FHostLobbySettings& LobbySettings) override;

protected:
    /**
     * @brief Pointer to the OnlineSessionInterface, which is used for all
     * things dealing with lobbies (game sessions).
     */
    IOnlineSessionPtr SessionInterface = nullptr;

    /**
     * @brief Various settings for the lobby (name, password, max players, etc.)
     */
    FHostLobbySettings HostLobbySettings;

    /**
     * @brief Create settings for a session based on the lobby settings.
     * @param LobbySettings Various info about the lobby's settings.
     * @return The FOnlineSessionSettings created from the lobby settings.
     */
    virtual FOnlineSessionSettings CreateSessionSettings(
        const FHostLobbySettings& LobbySettings);

    /**
     * @brief Sets lobby settings that are specific to the current subsystem
     * (NULL, Steam, etc.).
     * @param SessionSettings The session settings to directly modify.
     */
    virtual void SetSubsystemSpecificSessionSettings(
        const FHostLobbySettings& LobbySettings,
        FOnlineSessionSettings& SessionSettings);

private:
    /**
     * @brief Creates a lobby according to the stored @link HostLobbySettings.
     * Must be called after @link HostLobby is called at least once.
     */
    void CreateLobby();

    /**
     * @brief Callback for OnCreateSessionComplete; transfers the player to the
     * game level if successful.
     * @param SessionName is the session name constant, unused.
     * @param WasSuccessful if the lobby was successfully created.
     */
    void OnLobbyCreated(FName SessionName, bool WasSuccessful);

    /**
     * @brief Callback for OnDestroySessionComplete; re-attempts to host the
     * lobby if the previous one was successfully destroyed.
     */
    void OnLobbyDestroyed(FName SessionName, bool WasSuccessful);

#pragma endregion

#pragma region JoiningLobbies
public:
    /** Overridden method from @link IMainMenuInterface */
    virtual void SearchLobbies() override;

protected:
    /**
     * @brief The pointer to the Multiplayer Menu WBP. We need it to set the
     * lobby search results since we perform the lobby search through C++.
     * This is currently set through Blueprint on construction of the main menu.
     */
    UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    UMultiplayerMenu* MultiplayerMenu = nullptr;

    /**
     * @brief Shared pointer to the lobby search; this is what is used to get
     * all of the lobbies (search results).
     */
    TSharedPtr<FOnlineSessionSearch> LobbySearch = nullptr;

    /**
     * @brief Sets search query settings that are specific to the current
     * subsystem (NULL, Steam, etc.). Uses the LobbySearch shared pointer member
     * variable instead of passing in an FOnlineSessionSearch.
     */
    virtual void SetSubsystemSpecificSearchSettings();

    /**
     * @brief Creates FLobbyData struct based on the provided search result.
     * @param SearchResult The lobby search result to base this data off of.
     * @return FLobbyData struct containing info about the lobby.
     */
    virtual FLobbyData CreateLobbyData(
        const FOnlineSessionSearchResult& SearchResult);

    /**
     * @brief Sets lobby data that is specific to the current subsystem (NULL,
     * Steam, etc.).
     * @param LobbyData The data to modify according to the subsystem.
     */
    virtual void SetSubsystemSpecificLobbyData(FLobbyData& LobbyData);

    /** Overridden method from @link IMainMenuInterface */
    virtual void JoinLobby(int32 LobbyIndex) override;

private:
    /**
     * @brief Callback to OnFindSessionsComplete; populates the LobbyList of the
     * MultiplayerMenu with the results if successful.
     * @param WasSuccessful If the search completed successfully.
     */
    void OnLobbiesSearched(bool WasSuccessful);

    /**
     * @brief Adds an item to the array of lobby data.
     * @param LobbyDatas The TArray of FLobbyData to add to.
     * @param SearchResult The lobby search result to base the data off of.
     */
    void AddLobbyItem(TArray<FLobbyData>& LobbyDatas,
        const FOnlineSessionSearchResult& SearchResult);

    /**
     * @brief Callback to OnJoinSessionComplete; client travels to the server.
     * @param SessionName The session name constant.
     * @param JoinResult An enum describing whether it succeeded or why not.
     */
    void OnLobbyJoined(
        FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult);

    /**
     * @brief Logs depending on how the join session attempt failed.
     * @param JoinResult A non-successful result.
     */
    void LogUnsuccessfulJoinResult(
        EOnJoinSessionCompleteResult::Type JoinResult);

#pragma endregion
};
