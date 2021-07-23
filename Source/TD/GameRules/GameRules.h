// Copyright 2021, James S. Wang, All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "GameRules.generated.h"

class AGameState;
class ATDCharacter;
class AOrb;
class ATDGameState;

/**
 * @brief Replicated actor that contains all rules dictating whether something
 * in the game is/isn't allowed. It should remain stateless aside from its
 * reference to the game state to keep it consistent with both the client and
 * the server. This way the client can locally predict whether certain actions
 * should be permitted.
 */
UCLASS()
class TD_API AGameRules : public AInfo
{
    GENERATED_BODY()

#pragma region Initialization

public:
    AGameRules();
    void SetGameState(const ATDGameState* GameState);
    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    UPROPERTY(Replicated)
    const ATDGameState* TDGameState = nullptr;

#pragma endregion

#pragma region Rules

public:
    /**
     * @brief Check for whether a player can cast an orb.
     */
    virtual bool CanCastOrb() const;

    /**
     * @brief Check for whether a player can perform a Pull.
     */
    virtual bool CanPull() const;

    /**
     * @brief Check for whether a player can perform a Push.
     */
    virtual bool CanPush() const;

    /**
     * @brief Check for whether a player can push or pull an orb.
     * @param Player The player who wants to telekinese the orb.
     * @param Orb The orb to telekinese.
     */
    virtual bool CanTelekineseOrb(const ATDCharacter* Player,
        const AOrb* Orb) const;

protected:
    /**
     * @brief Check for whether the game is in active play.
     */
    virtual bool IsInPlay() const;

#pragma endregion
};
