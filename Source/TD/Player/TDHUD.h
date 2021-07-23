// Copyright 2021, James S. Wang, All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "TDHUD.generated.h"

/**
 * @brief The player's HUD when they are in play.
 */
UCLASS()
class TD_API ATDHUD : public AHUD
{
    GENERATED_BODY()

public:
    /**
     * @brief Draws the crosshair.
     */
    UFUNCTION(BlueprintCallable, Category = "Game HUD")
    virtual void DrawHUD() override;

protected:
    /**
     * @brief Creates the Game HUD widget on the client.
     */
    virtual void BeginPlay() override;

private:
    /**
     * @brief The texture to use to draw the crosshair.
     */
    UPROPERTY(EditDefaultsOnly, Category = "Crosshair")
    UTexture2D* CrosshairTexture = nullptr;

    /**
     * @brief The widget blueprint of the Game HUD.
     */
    UPROPERTY(EditDefaultsOnly, Category = "Game HUD")
    TSubclassOf<UUserWidget> GameHUDClass = nullptr;

    /**
     * @brief Reference to the Game HUD widget to show/hide it.
     */
    UPROPERTY()
    UUserWidget* GameHUD = nullptr;

    /**
     * @brief Shows/Hides the GameHUD widget.
     */
    UFUNCTION(BlueprintCallable, Category = "Game HUD")
    void ShowGameHUD();
};
