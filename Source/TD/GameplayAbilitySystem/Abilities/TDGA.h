// Copyright 2021, James S. Wang, All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "TDTypes.h"
#include "Abilities/GameplayAbility.h"
#include "TDGA.generated.h"

class AGameRules;
class ATDGameState;
class ATDGameMode;

/**
 * @brief Base class for gameplay abilities, defines an input id and a name.
 */
UCLASS()
class TD_API UTDGA : public UGameplayAbility
{
    GENERATED_BODY()

public:
    /**
     * @brief Gets the input ID of this gameplay ability.
     * @return The input ID of this ability, as an int32.
     */
    int32 GetAbilityInputID() const;

    /**
     * @brief Gets the name of this gameplay ability.
     * @return The name of this ability.
     */
    FString GetAbilityName() const;

protected:
    /**
     * @brief The input ID enum value for this ability.
     */
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability")
    ETDAbilityInputID AbilityInputID = ETDAbilityInputID::None;

    /**
     * @brief The name of the ability.
     */
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability")
    FString AbilityName = "Unnamed Ability";

    /**
     * @brief Gets the TDGameMode or nullptr if not the server.
     */
    ATDGameMode* GetGameMode() const;

    /**
     * @brief Gets the TDGameState.
     */
    ATDGameState* GetGameState() const;

    /**
     * @brief Gets the Game Rules for the current game mode.
     */
    AGameRules* GetGameRules() const;
};
