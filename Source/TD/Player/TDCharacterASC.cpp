// Copyright 2021, James S. Wang, All rights reserved.

#include "TDCharacterASC.h"

#include "GameConfiguration.h"
#include "GameplayAbilitySystem/Abilities/TDGA.h"

void UTDCharacterASC::GrantDefaultAbilities()
{
    if (GetOwnerRole() != ROLE_Authority || HaveGrantedDefaultAbilities)
    {
        return;
    }

    for (const TSubclassOf<UTDGA>& AbilityClass : DefaultAbilities)
    {
        UTDGA* Ability = AbilityClass.GetDefaultObject();
        GiveAbility(FGameplayAbilitySpec(Ability, 1,
            Ability->GetAbilityInputID(), GetOwnerActor()));

        UE_LOG(LogTDGAS, Log, TEXT("Granted ability: %s"),
            *Ability->GetAbilityName())
    }
    HaveGrantedDefaultAbilities = true;
}
