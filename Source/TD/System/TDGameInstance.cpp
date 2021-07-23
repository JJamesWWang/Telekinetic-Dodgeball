// Copyright 2021, James S. Wang, All rights reserved.

#include "TDGameInstance.h"
#include "GameConfiguration.h"
#include "GameState/TDGameState.h"
#include "GameState/UIGameState.h"

UUIGameState* UTDGameInstance::GetUIGameState() const
{
    UWorld* const World = GetWorld();
    if (World == nullptr)
    {
        LogInvalidPointer("UTDGameInstance", "GetUIGameState", "World");
        return nullptr;
    }

    ATDGameState* GameState = World->GetGameState<ATDGameState>();
    if (GameState == nullptr)
    {
        LogInvalidPointer("UTDGameInstance", "GetUIGameState", "GameState",
            "Are you currently in a game?");
        return nullptr;
    }

    return GameState->GetUIGameState();
}

void UTDGameInstance::LoadMainMenuMap()
{
    APlayerController* PlayerController = GetFirstLocalPlayerController();
    if (PlayerController == nullptr)
    {
        LogInvalidPointer(
            "UTDGameInstance", "LoadPracticeMap", "PlayerController");
        return;
    }

    PlayerController->ClientTravel(MainMenuMapDirectory, TRAVEL_Absolute);
}

void UTDGameInstance::LoadPracticeMap()
{
    APlayerController* PlayerController = GetFirstLocalPlayerController();
    if (PlayerController == nullptr)
    {
        LogInvalidPointer(
            "UTDGameInstance", "LoadPracticeMap", "PlayerController");
        return;
    }

    PlayerController->ClientTravel(PracticeMapDirectory, TRAVEL_Absolute);
}

void UTDGameInstance::LoadMultiplayerMap()
{
    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        LogInvalidPointer(
            "UTDGameInstance", "LoadMultiplayerMap", "World");
        return;
    }

    UE_LOG(LogTD, Log, TEXT("Starting server with options: %s"),
        *HostLobbySettings.ToOptionsString());
    World->ServerTravel(MultiplayerMapDirectory +
                        HostLobbySettings.ToOptionsString() + "?listen");
}

void UTDGameInstance::SavePlayerGameplaySettings(
    const FPlayerGameplaySettings& Settings)
{
    const FString SlotName = FPlayerGameplaySettings::GetSlotName();
    UPlayerSettingsSave* const Save = GetSave(SlotName);
    if (Save == nullptr)
    {
        LogInvalidPointer("UTDGameInstance", "SavePlayerGameplaySettings",
            "Save");
        return;
    }

    Save->PlayerGameplaySettings = Settings;

    if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        if (!UGameplayStatics::DeleteGameInSlot(SlotName, 0))
        {
            UE_LOG(LogTD, Error,
                TEXT("Failed to delete save while saving new."));
        }
    }

    const bool SavedSuccessfully = UGameplayStatics::SaveGameToSlot(Save,
        SlotName, 0);
    if (SavedSuccessfully)
    {
        OnPlayerGameplaySettingsSaved.Broadcast(Settings);
    }
    else
    {
        UE_LOG(LogTD, Error, TEXT("SavePlayerGameplaySettings failed."));
    }
}

UPlayerSettingsSave* UTDGameInstance::GetSave(const FString& SlotName) const
{
    // Get previous save if it exists, otherwise create new save.
    return UGameplayStatics::DoesSaveGameExist(SlotName, 0)
               ? Cast<UPlayerSettingsSave>(
                   UGameplayStatics::LoadGameFromSlot(SlotName, 0))
               : Cast<UPlayerSettingsSave>(
                   UGameplayStatics::CreateSaveGameObject(
                       UPlayerSettingsSave::StaticClass()));
}

FPlayerGameplaySettings UTDGameInstance::LoadPlayerGameplaySettings()
{
    const FString SlotName = FPlayerGameplaySettings::GetSlotName();
    if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        return FPlayerGameplaySettings();
    }

    UPlayerSettingsSave* Save = Cast<UPlayerSettingsSave>(
        UGameplayStatics::LoadGameFromSlot(SlotName, 0));
    if (Save == nullptr)
    {
        LogInvalidPointer("UTDGameInstance", "LoadPlayerGameplaySettings",
            "Save", "Deleting save...");
        if (!UGameplayStatics::DeleteGameInSlot(SlotName, 0))
        {
            UE_LOG(LogTD, Error, TEXT("Failed to delete save."));
        }
        return FPlayerGameplaySettings();
    }

    return Save->PlayerGameplaySettings;
}

void UTDGameInstance::SavePlayerAudioSettings(
    const FPlayerAudioSettings& Settings)
{
    const FString SlotName = FPlayerAudioSettings::GetSlotName();
    UPlayerSettingsSave* const Save = GetSave(SlotName);
    if (Save == nullptr)
    {
        LogInvalidPointer("UTDGameInstance", "SavePlayerAudioSettings", "Save");
        return;
    }

    if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        if (!UGameplayStatics::DeleteGameInSlot(SlotName, 0))
        {
            UE_LOG(LogTD, Error,
                TEXT("Failed to delete save while saving new."));
        }
    }

    Save->PlayerAudioSettings = Settings;
    const bool SavedSuccessfully = UGameplayStatics::SaveGameToSlot(Save,
        SlotName, 0);
    if (!SavedSuccessfully)
    {
        UE_LOG(LogTD, Error, TEXT("SavePlayerGameplaySettings failed."));
    }
}

FPlayerAudioSettings UTDGameInstance::LoadPlayerAudioSettings()
{
    const FString SlotName = FPlayerAudioSettings::GetSlotName();
    if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        return FPlayerAudioSettings();
    }

    UPlayerSettingsSave* Save = Cast<UPlayerSettingsSave>(
        UGameplayStatics::LoadGameFromSlot(SlotName, 0));
    if (Save == nullptr)
    {
        LogInvalidPointer("UTDGameInstance", "LoadPlayerAudioSettings",
            "Save", "Deleting save...");
        if (!UGameplayStatics::DeleteGameInSlot(SlotName, 0))
        {
            UE_LOG(LogTD, Error, TEXT("Failed to delete save."));
        }
        return FPlayerAudioSettings();
    }

    return Save->PlayerAudioSettings;
}
