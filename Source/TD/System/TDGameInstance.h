// Copyright 2021, James S. Wang, All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TDTypes.h"
#include "LobbySystem/LSGameInstance.h"
#include "GameFramework/SaveGame.h"

#include "TDGameInstance.generated.h"

class UPlayerSettingsSave;
class UUIGameState;

/**
 * @brief Handles loading of maps, saving of player settings, and provides UI
 * helpers.
 */
UCLASS()
class TD_API UTDGameInstance : public ULSGameInstance
{
    GENERATED_BODY()

#pragma region UI Helpers

public:
    /**
     * @brief Retrieves the UI Game State from the game state. This method
     * exists so UI is abstracted from the game state.
     * @return The UI Game State or nullptr if not playing in a match.
     */
    UFUNCTION(BlueprintPure)
    UUIGameState* GetUIGameState() const;

#pragma endregion

#pragma region Loading Maps

private:
    /** @see IMainMenuInterface */

    const FString MainMenuMapDirectory = "/Game/TD/Maps/MainMenu";
    const FString PracticeMapDirectory = "/Game/TD/Maps/PracticeMap";
    const FString MultiplayerMapDirectory = "/Game/TD/Maps/MultiplayerMap";

    virtual void LoadMainMenuMap() override;
    virtual void LoadPracticeMap() override;
    virtual void LoadMultiplayerMap() override;

#pragma endregion

#pragma region Saving and Loading Settings

public:
    DECLARE_EVENT_OneParam(ATDGameInstance, FPlayerGameplaySettingsSavedEvent,
        const FPlayerGameplaySettings& /* Settings */)

    /**
     * @brief Callback for when player gameplay settings are saved, implying
     * that previously stored settings may have changed.
     */
    FPlayerGameplaySettingsSavedEvent OnPlayerGameplaySettingsSaved;

    /**
     * @brief Saves and overrides the provided gameplay settings.
     * @param Settings The new player gameplay settings to be saved.
     */
    UFUNCTION(BlueprintCallable)
    void SavePlayerGameplaySettings(const FPlayerGameplaySettings& Settings);

    /**
     * @brief Gets the updated player gameplay settings.
     */
    UFUNCTION(BlueprintCallable)
    FPlayerGameplaySettings LoadPlayerGameplaySettings();

    /**
     * @brief Saves and overrides the provided audio settings.
     * @param Settings The new player audio settings to be saved.
     */
    UFUNCTION(BlueprintCallable)
    void SavePlayerAudioSettings(const FPlayerAudioSettings& Settings);

    /**
     * @brief Gets the updated player audio settings.
     */
    UFUNCTION(BlueprintCallable)
    FPlayerAudioSettings LoadPlayerAudioSettings();

private:
    UPlayerSettingsSave* GetSave(const FString& SlotName) const;

#pragma endregion
};

/**
 * @brief SaveGame subclass that contains all of the player's local settings.
 */
UCLASS()
class TD_API UPlayerSettingsSave : public USaveGame
{
    GENERATED_BODY()

public:
    /* @see FPlayerGameplaySettings */
    UPROPERTY(SaveGame)
    FPlayerGameplaySettings PlayerGameplaySettings;

    /* @see FPlayerAudioSettings */
    UPROPERTY(SaveGame)
    FPlayerAudioSettings PlayerAudioSettings;
};
