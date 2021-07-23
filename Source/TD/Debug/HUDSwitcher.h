// Copyright 2021, James S. Wang, All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "HUDSwitcher.generated.h"

class ADebugGameStateHUD;
class ATDHUD;

/**
 * @brief Doesn't really work, but it's supposed to allow you to switch between
 * different HUDs through the command line in-game.
 */
UCLASS()
class TD_API AHUDSwitcher : public AHUD
{
    GENERATED_BODY()

protected:
    virtual void BeginPlay() override;

public:
    UFUNCTION(Exec)
    void ShowTDHUD();

    UFUNCTION(Exec)
    void ShowDebugGameStateHUD();

protected:
    UPROPERTY(EditAnywhere)
    TSubclassOf<ATDHUD> TDHUDClass = nullptr;

    UPROPERTY(EditAnywhere)
    TSubclassOf<ADebugGameStateHUD> DebugGameStateHUDClass = nullptr;
};
