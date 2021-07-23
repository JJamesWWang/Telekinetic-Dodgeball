// Copyright 2021, James S. Wang, All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "TDCharacterASC.generated.h"

class UTDGA;

/**
 * @brief The player character's ability system component.
 */
UCLASS()
class TD_API UTDCharacterASC : public UAbilitySystemComponent
{
    GENERATED_BODY()

public:
    /**
     * @brief Grants all of the DefaultAbilities to the player.
     * @see DefaultAbilities
     */
    void GrantDefaultAbilities();

protected:
    /**
     * @brief Abilities that the player should start with.
     */
    UPROPERTY(EditAnywhere)
    TArray<TSubclassOf<UTDGA>> DefaultAbilities;

private:
    /**
     * @brief Preventative bool to make sure abilities are only granted once.
     */
    bool HaveGrantedDefaultAbilities = false;
};
