// Copyright 2021, James S. Wang, All rights reserved.

#pragma once

#include "LSWidget.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "CoreMinimal.h"

#include "MultiplayerMenu.generated.h"

class ULobbyItem;

/**
 * @brief A struct representing data to be displayed through the LobbyItem
 * widget. This should include everything the player needs to see when they are
 * searching for lobbies as these represent the results.
 */
USTRUCT(BlueprintType)
struct TD_API FLobbyData
{
    GENERATED_BODY()

    /**
     * @brief The name of the lobby.
     */
    UPROPERTY(BlueprintReadOnly)
    FString LobbyName;

    /**
     * @brief The number of players currently in the lobby.
     */
    UPROPERTY(BlueprintReadOnly)
    int32 CurrentPlayers;

    /**
     * @brief The maximum number of players allowed in the lobby.
     */
    UPROPERTY(BlueprintReadOnly)
    int32 MaxPlayers;
};

/**
 * @brief The widget dedicated to the MultiplayerMenu. It allows players to host
 * and join lobbies.
 */
UCLASS()
class TD_API UMultiplayerMenu : public ULSWidget
{
    GENERATED_BODY()

public:
    /**
     * @brief (Re)sets the rows of the LobbyList and creates now rows (of
     * LobbyItems) with the provided lobby data.
     * @param LobbyDatas TArray of FLobbyData that represents each row's data.
     */
    void SetLobbyList(TArray<FLobbyData> LobbyDatas);

private:
    /**
     * @brief The class of the LobbyItem, set through Blueprint.
     */
    UPROPERTY(EditDefaultsOnly, Category = "LobbyList")
    TSubclassOf<ULobbyItem> LobbyItemClass = nullptr;

    /**
     * @brief The UListView widget that holds all of the LobbyItem rows.
     */
    UPROPERTY(
        BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
    UListView* LobbyList = nullptr;

    /**
     * @brief The title text for the "Join Lobby" part of the UI. This is
     * initially set to "Searching for lobbies..." and then is dynamically set
     * to either "Join Lobby" or "No Lobbies Found" depending on if there were
     * lobby search results.
     */
    UPROPERTY(BlueprintReadOnly,
        meta = (BindWidget, AllowPrivateAccess = "true"))
    UTextBlock* JoinLobbyText = nullptr;

    /**
     * @brief Sets JoinLobbyText to either "Join Lobby" or "No Lobbies Found"
     * @param WereLobbiesFound If at least one lobby was found.
     */
    void SetJoinLobbyText(bool WereLobbiesFound);
};
