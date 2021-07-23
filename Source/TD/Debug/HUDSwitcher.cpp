// Copyright 2021, James S. Wang, All rights reserved.

#include "HUDSwitcher.h"

#include "DebugGameStateHUD.h"
#include "GameConfiguration.h"
#include "Player/TDHUD.h"

void AHUDSwitcher::BeginPlay()
{
    Super::BeginPlay();
    if (TDHUDClass == nullptr || DebugGameStateHUDClass == nullptr)
    {
        LogInvalidPointer("AHUDSwitcher", "BeginPlay", "HUD Class",
            "Did you set all of the HUD classes for the HUD switcher?");
    }
}

void AHUDSwitcher::ShowTDHUD()
{
    if (TDHUDClass != nullptr)
    {
        GetOwningPlayerController()->ClientSetHUD(TDHUDClass);
    }
}

void AHUDSwitcher::ShowDebugGameStateHUD()
{
    if (DebugGameStateHUDClass != nullptr)
    {
        GetOwningPlayerController()->ClientSetHUD(DebugGameStateHUDClass);
    }
}
