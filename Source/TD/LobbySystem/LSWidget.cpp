// Copyright 2021, James S. Wang, All rights reserved.

#include "LSWidget.h"

#include "GameConfiguration.h"

void ULSWidget::Create()
{
    bIsFocusable = true;
    AddToViewport();

    APlayerController* PlayerController = GetPlayerController();
    if (PlayerController == nullptr)
    {
        LogInvalidPointer("ULSWidget", "Create", "PlayerController");
        return;
    }

    FInputModeUIOnly UIOnlyInputMode;
    UIOnlyInputMode.SetWidgetToFocus(TakeWidget());
    // Stops annoying locking if in fullscreen
    UIOnlyInputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    PlayerController->SetInputMode(UIOnlyInputMode);
    PlayerController->bShowMouseCursor = true;
}

void ULSWidget::Destroy()
{
    bIsFocusable = false;
    RemoveFromViewport();

    APlayerController* PlayerController = GetPlayerController();
    if (PlayerController == nullptr)
    {
        LogInvalidPointer("ULSWidget", "Destroy", "PlayerController");
        return;
    }

    FInputModeGameOnly GameOnlyInputMode;
    // So that players don't shoot upon clicking out of the menu
    GameOnlyInputMode.SetConsumeCaptureMouseDown(false);
    PlayerController->SetInputMode(GameOnlyInputMode);
    PlayerController->bShowMouseCursor = false;
}

void ULSWidget::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
    Destroy();
    Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}

APlayerController* ULSWidget::GetPlayerController() const
{
    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        LogInvalidPointer("ULSWidget", "GetPlayerController", "World");
        return nullptr;
    }
    return World->GetFirstPlayerController();
}
