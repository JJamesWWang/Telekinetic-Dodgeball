// Copyright 2021, James S. Wang, All rights reserved.

#pragma once

#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "CoreMinimal.h"
#include "MultiplayerMenu.h"

#include "LobbyItem.generated.h"

/**
 * @brief A row of data that represents a lobby found from a search.
 */
UCLASS()
class TD_API ULobbyItem : public UUserWidget, public IUserObjectListEntry
{
    GENERATED_BODY()

public:
    void SetLobbyItemData(FLobbyData Data)
    {
        LobbyData = Data;
    }

private:
    /**
     * @brief All of the data that this row displays.
     */
    UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    FLobbyData LobbyData;

    /**
     * @brief The Text widget that displays the lobby name.
     */
    UPROPERTY(
        BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
    UTextBlock* LobbyNameText = nullptr;

    /**
     * @brief The Text widget that displays the formatted player count.
     */
    UPROPERTY(
        BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
    UTextBlock* PlayerCountText = nullptr;

    /**
     * @brief Sets the text dedicated to the lobby name.
     * @param LobbyName The name of the lobby.
     */
    UFUNCTION(BlueprintCallable)
    void SetLobbyName(FString LobbyName);

    /**
     * @brief Sets the text dedicated to the player count. The format is
     * "{CurrentPlayers}/{MaxPlayers}" e.g. "2/4".
     * @param CurrentPlayers The current number of players in the lobby.
     * @param MaxPlayers The maximum number of players allowed in the lobby.
     */
    UFUNCTION(BlueprintCallable)
    void SetPlayerCount(int32 CurrentPlayers, int32 MaxPlayers);
};
