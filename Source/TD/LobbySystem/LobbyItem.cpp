// Copyright 2021, James S. Wang, All rights reserved.

#include "LobbyItem.h"

#include "GameConfiguration.h"

void ULobbyItem::SetLobbyName(FString LobbyName)
{
    if (LobbyNameText == nullptr)
    {
        LogInvalidPointer("ULobbyItem", "SetLobbyName", "LobbyNameText");
        return;
    }

    LobbyNameText->SetText(FText::FromString(LobbyName));
}

void ULobbyItem::SetPlayerCount(int32 CurrentPlayers, int32 MaxPlayers)
{
    if (PlayerCountText == nullptr)
    {
        LogInvalidPointer("ULobbyItem", "SetPlayerCount", "PlayerCountText");
        return;
    }

    PlayerCountText->SetText(FText::FromString(
        FString::Printf(TEXT("%d/%d"), CurrentPlayers, MaxPlayers)));
}
