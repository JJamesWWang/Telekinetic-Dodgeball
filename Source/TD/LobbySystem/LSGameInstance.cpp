// Copyright 2021, James S. Wang, All rights reserved.

#include "LSGameInstance.h"

#include "GameConfiguration.h"
#include "Blueprint/UserWidget.h"
#include "MultiplayerMenu.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

#pragma region Initialization

void ULSGameInstance::Init()
{
    IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
    if (OSS == nullptr)
    {
        LogInvalidPointer("ULSGameInstance", "Init", "OSS");
        return;
    }

    UE_LOG(LobbySystem, Log, TEXT("Loaded OSS: %s"),
        *OSS->GetSubsystemName().ToString());
    SessionInterface = OSS->GetSessionInterface();
    if (SessionInterface.IsValid())
    {
        BindToSessionInterfaceDelegates();
    }

    if (GEngine == nullptr)
    {
        LogInvalidPointer("ULSGameInstance", "Init", "Engine");
        return;
    }
    GEngine->NetworkFailureEvent.AddUObject(
        this, &ULSGameInstance::OnNetworkFailed);
}

void ULSGameInstance::BindToSessionInterfaceDelegates()
{
    SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(
        this, &ULSGameInstance::OnLobbyCreated);
    SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(
        this, &ULSGameInstance::OnLobbiesSearched);
    SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(
        this, &ULSGameInstance::OnLobbyJoined);
    SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(
        this, &ULSGameInstance::OnLobbyDestroyed);
}

void ULSGameInstance::LoadPracticeMap()
{
    unimplemented();
}

void ULSGameInstance::LoadMultiplayerMap()
{
    unimplemented();
}

#pragma endregion

#pragma region MainMenu

void ULSGameInstance::LoadMainMenuMap()
{
    APlayerController* PlayerController = GetFirstLocalPlayerController();
    if (PlayerController == nullptr)
    {
        LogInvalidPointer(
            "ULSGameInstance", "LoadMainMenuMap", "PlayerController");
        return;
    }

    PlayerController->ClientTravel(MainMenuMapDirectory, TRAVEL_Absolute);
}

void ULSGameInstance::DestroySession()
{
    if (SessionInterface->GetNamedSession(NAME_GameSession) != nullptr)
    {
        SessionInterface->DestroySession(NAME_GameSession);
    }
}

void ULSGameInstance::OnNetworkFailed(UWorld* World, UNetDriver* NetDriver,
    ENetworkFailure::Type FailureType, const FString& ErrorString)
{
    UE_LOG(LobbySystem, Warning, TEXT("%s"), *ErrorString);
    LoadMainMenuMap();
}

#pragma endregion

#pragma region HostingLobbies

void ULSGameInstance::HostLobby(const FHostLobbySettings& LobbySettings)
{
    if (!SessionInterface.IsValid())
    {
        LogInvalidPointer("ULSGameInstance", "HostLobby", "SessionInterface");
        return;
    }

    HostLobbySettings = LobbySettings;
    CreateLobby();
}

void ULSGameInstance::CreateLobby()
{
    SessionInterface->CreateSession(
        0, NAME_GameSession, CreateSessionSettings(HostLobbySettings));
    UE_LOG(LobbySystem, Log,
        TEXT(
            "Attempting to create lobby with name %s, friendsonly %s, minplayers "
            "%d, maxplayers %d."),
        *HostLobbySettings.LobbyName,
        HostLobbySettings.IsFriendsOnly ? TEXT("yes") : TEXT("no"),
        HostLobbySettings.MinPlayers, HostLobbySettings.MaxPlayers);
}

FOnlineSessionSettings ULSGameInstance::CreateSessionSettings(
    const FHostLobbySettings& LobbySettings)
{
    FOnlineSessionSettings SessionSettings;
    SessionSettings.NumPublicConnections = LobbySettings.MaxPlayers;
    SessionSettings.Set("LobbyName", LobbySettings.LobbyName,
        EOnlineDataAdvertisementType::ViaOnlineService);
    SessionSettings.Set("MinPlayers", LobbySettings.MinPlayers,
        EOnlineDataAdvertisementType::ViaOnlineService);

    SessionSettings.bShouldAdvertise = true;
    SessionSettings.bAllowJoinInProgress = true;
    SetSubsystemSpecificSessionSettings(LobbySettings, SessionSettings);
    return SessionSettings;
}

void ULSGameInstance::SetSubsystemSpecificSessionSettings(
    const FHostLobbySettings& LobbySettings,
    FOnlineSessionSettings& SessionSettings)
{
    IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
    if (OSS == nullptr)
    {
        LogInvalidPointer(
            "ULSGameInstance", "SetSubsystemSpecificSessionSettings", "OSS");
        return;
    }

    const FName SubsystemName = OSS->GetSubsystemName();
    if (SubsystemName == "NULL")
    {
        SessionSettings.bIsLANMatch = true;
    }
    else if (SubsystemName == "Steam")
    {
        SessionSettings.bUsesPresence = true;
        SessionSettings.bAllowJoinViaPresence = true;
        SessionSettings.bAllowJoinViaPresenceFriendsOnly =
            LobbySettings.IsFriendsOnly;
    }
}

void ULSGameInstance::OnLobbyCreated(FName SessionName, bool WasSuccessful)
{
    if (WasSuccessful)
    {
        UE_LOG(LobbySystem, Log, TEXT("Lobby created."));
        LoadMultiplayerMap();
    }
    else
    {
        UE_LOG(LobbySystem, Warning, TEXT("Failed to create Lobby."));
    }
}

void ULSGameInstance::OnLobbyDestroyed(FName SessionName, bool WasSuccessful)
{
    if (!SessionInterface.IsValid())
    {
        LogInvalidPointer(
            "ULSGameInstance", "DestroyLobby", "SessionInterface");
        return;
    }

    if (WasSuccessful)
    {
        UE_LOG(LobbySystem, Log, TEXT("Lobby destroyed, rehosting."));
    }
    else
    {
        UE_LOG(LobbySystem, Log, TEXT("Failed to destroy lobby."));
    }
}

#pragma endregion

#pragma region JoiningLobbies

void ULSGameInstance::SearchLobbies()
{
    LobbySearch = MakeShareable(new FOnlineSessionSearch());
    if (!LobbySearch.IsValid() || !SessionInterface.IsValid())
    {
        LogInvalidPointer("ULSGameInstance", "SearchLobbies",
            "LobbySearch or SessionInterface");
        return;
    }

    UE_LOG(LobbySystem, Log, TEXT("Searching for lobbies..."));
    LobbySearch->QuerySettings.Set(
        SEARCH_NONEMPTY_SERVERS_ONLY, true, EOnlineComparisonOp::Equals);
    LobbySearch->MaxSearchResults = 20;
    SetSubsystemSpecificSearchSettings();
    SessionInterface->FindSessions(0, LobbySearch.ToSharedRef());
}

void ULSGameInstance::SetSubsystemSpecificSearchSettings()
{
    IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
    if (OSS == nullptr)
    {
        LogInvalidPointer(
            "ULSGameInstance", "SetSubsystemSpecificSearchSettings", "OSS");
        return;
    }

    FName SubsystemName = OSS->GetSubsystemName();
    if (SubsystemName == "NULL")
    {
        LobbySearch->bIsLanQuery = true;
    }
    else if (SubsystemName == "Steam")
    {
        LobbySearch->QuerySettings.Set(
            SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
    }
}

void ULSGameInstance::OnLobbiesSearched(bool WasSuccessful)
{
    if (!WasSuccessful || !SessionInterface.IsValid() ||
        MultiplayerMenu == nullptr)
    {
        if (WasSuccessful)
        {
            LogInvalidPointer("ULSGameInstance", "OnLobbiesSearched",
                "SessionInterface or MultiplayerMenu");
        }
        return;
    }

    UE_LOG(LobbySystem, Log, TEXT("Found lobbies!"));
    TArray<FLobbyData> LobbyDatas;
    for (const FOnlineSessionSearchResult& SearchResult :
         LobbySearch->SearchResults)
    {
        if (!SearchResult.IsValid())
        {
            continue;
        }

        AddLobbyItem(LobbyDatas, SearchResult);
    }
    MultiplayerMenu->SetLobbyList(LobbyDatas);
}

void ULSGameInstance::AddLobbyItem(TArray<FLobbyData>& LobbyDatas,
    const FOnlineSessionSearchResult& SearchResult)
{
    FLobbyData LobbyData = CreateLobbyData(SearchResult);
    SetSubsystemSpecificLobbyData(LobbyData);
    LobbyDatas.Emplace(LobbyData);
    UE_LOG(LobbySystem, Log, TEXT("Found lobby with name %s, players: %d/%d"),
        *LobbyData.LobbyName, LobbyData.CurrentPlayers, LobbyData.MaxPlayers);
}

FLobbyData ULSGameInstance::CreateLobbyData(
    const FOnlineSessionSearchResult& SearchResult)
{
    FLobbyData LobbyData;
    FString LobbyName;
    LobbyData.LobbyName =
        SearchResult.Session.SessionSettings.Get("LobbyName", LobbyName)
            ? LobbyName
            : SearchResult.Session.GetSessionIdStr();
    LobbyData.MaxPlayers =
        SearchResult.Session.SessionSettings.NumPublicConnections;
    LobbyData.CurrentPlayers =
        LobbyData.MaxPlayers - SearchResult.Session.NumOpenPublicConnections;
    return LobbyData;
}

void ULSGameInstance::SetSubsystemSpecificLobbyData(FLobbyData& LobbyData)
{
}

void ULSGameInstance::JoinLobby(int32 LobbyIndex)
{
    if (!SessionInterface.IsValid() || !LobbySearch.IsValid())
    {
        LogInvalidPointer("ULSGameInstance", "JoinLobby",
            "SessionInterface or LobbySearch");
        return;
    }

    if (!LobbySearch->SearchResults.IsValidIndex(LobbyIndex))
    {
        UE_LOG(LobbySystem, Warning,
            TEXT("Invalid LobbyIndex selected for search results: %d"),
            LobbyIndex);
        return;
    }

    SessionInterface->JoinSession(
        0, NAME_GameSession, LobbySearch->SearchResults[LobbyIndex]);
    UE_LOG(
        LobbySystem, Log, TEXT("Joining session at index %d..."), LobbyIndex);
}

void ULSGameInstance::OnLobbyJoined(
    FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult)
{
    APlayerController* PlayerController = GetFirstLocalPlayerController();
    if (!SessionInterface.IsValid() || PlayerController == nullptr)
    {
        LogInvalidPointer("ULSGameInstance", "OnLobbyJoined",
            "SessionInterface or PlayerController");
    }

    if (JoinResult != EOnJoinSessionCompleteResult::Success)
    {
        LogUnsuccessfulJoinResult(JoinResult);
    }

    FString ConnectURL;
    bool WasSuccessful =
        SessionInterface->GetResolvedConnectString(SessionName, ConnectURL);
    if (WasSuccessful)
    {
        UE_LOG(LobbySystem, Log, TEXT("Successfully joined lobby."));
        PlayerController->ClientTravel(ConnectURL, TRAVEL_Absolute);
    }
    else
    {
        UE_LOG(LobbySystem, Log, TEXT("Failed to join lobby."));
    }
}

void ULSGameInstance::LogUnsuccessfulJoinResult(
    EOnJoinSessionCompleteResult::Type JoinResult)
{
    switch (JoinResult)
    {
        case EOnJoinSessionCompleteResult::AlreadyInSession:
            UE_LOG(LobbySystem, Warning, TEXT("Player is already in session."));
            break;
        case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
            UE_LOG(LobbySystem, Warning,
                TEXT("Could not retrieve server address of session."));
            break;
        case EOnJoinSessionCompleteResult::SessionDoesNotExist:
            UE_LOG(LobbySystem, Warning, TEXT("Session no longer exists."));
            break;
        case EOnJoinSessionCompleteResult::SessionIsFull:
            UE_LOG(LobbySystem, Warning, TEXT("Session is full."));
            break;
        case EOnJoinSessionCompleteResult::UnknownError:
            UE_LOG(LobbySystem, Warning,
                TEXT("Unknown error encountered trying to join session."));
            break;
    }
}

#pragma endregion
