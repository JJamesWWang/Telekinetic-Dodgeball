// Copyright 2021, James S. Wang, All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Debug/DebugHUD.h"
#include "DebugGameStateHUD.generated.h"

/**
 * @brief Displays various information about the game state.
 */
UCLASS()
class TD_API ADebugGameStateHUD final : public ADebugHUD
{
    GENERATED_BODY()

    /**
     * @brief Draws variables to the HUD.
     */
    virtual void DrawHUD() override;
};
