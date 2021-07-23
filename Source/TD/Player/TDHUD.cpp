// Copyright 2021, James S. Wang, All rights reserved.

#include "TDHUD.h"

#include "Engine/Canvas.h"
#include "GameConfiguration.h"
#include "Blueprint/UserWidget.h"

void ATDHUD::DrawHUD()
{
    Super::DrawHUD();
    if (CrosshairTexture == nullptr)
    {
        LogInvalidPointer("ATDHUD", "DrawHUD", "CrosshairTexture",
            "Did you forget to set the crosshair texture?");
        return;
    }

    // Offset by half of the texture's dimensions so it's drawn in the center
    // of the Canvas.
    const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);
    const FVector2D CrosshairDrawPosition(Center.X - 8.0f, Center.Y - 8.0f);

    FCanvasTileItem TileItem(CrosshairDrawPosition, CrosshairTexture->Resource,
        FLinearColor::White);
    TileItem.BlendMode = SE_BLEND_Translucent;
    Canvas->DrawItem(TileItem);
}

void ATDHUD::BeginPlay()
{
    Super::BeginPlay();
    if (GameHUDClass == nullptr)
    {
        LogInvalidPointer("ATDHUD", "BeginPlay", "GameHUDClass",
            "Did you remember to set the TDHUD's Game HUD class?");
        return;
    }

    GameHUD = CreateWidget<UUserWidget>(
        GetOwningPlayerController(), GameHUDClass);
}

void ATDHUD::ShowGameHUD()
{
    if (GameHUD != nullptr)
    {
        if (!GameHUD->IsInViewport())
        {
            GameHUD->AddToViewport();
        }
        else
        {
            GameHUD->RemoveFromViewport();
        }
    }
}
