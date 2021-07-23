// Copyright 2021, James S. Wang, All rights reserved.

#include "MultiplayerMenu.h"

#include "GameConfiguration.h"
#include "LobbyItem.h"

void UMultiplayerMenu::SetLobbyList(TArray<FLobbyData> LobbyDatas)
{
    if (LobbyList == nullptr || LobbyItemClass == nullptr)
    {
        LogInvalidPointer(
            "UMultiplayerMenu", "SetLobbyList", "LobbyList or LobbyItemClass");
        return;
    }

    LobbyList->ClearListItems();
    for (FLobbyData LobbyData : LobbyDatas)
    {
        ULobbyItem* LobbyItem = CreateWidget<ULobbyItem>(this, LobbyItemClass);
        LobbyItem->SetLobbyItemData(LobbyData);
        LobbyList->AddItem(LobbyItem);
    }
    SetJoinLobbyText(LobbyList->GetNumItems() > 0);
}

void UMultiplayerMenu::SetJoinLobbyText(bool WereLobbiesFound)
{
    if (JoinLobbyText == nullptr)
    {
        LogInvalidPointer(
            "UMultiplayerMenu", "SetJoinLobbyText", "JoinLobbyText");
        return;
    }

    if (WereLobbiesFound)
    {
        JoinLobbyText->SetText(FText::FromString("JOIN LOBBY"));
    }
    else
    {
        JoinLobbyText->SetText(FText::FromString("NO LOBBIES FOUND"));
    }
}
