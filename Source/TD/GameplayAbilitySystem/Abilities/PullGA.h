// Copyright 2021, James S. Wang, All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TDGA.h"
#include "PullGA.generated.h"

class USoundCue;
/**
 * @brief The Pull gameplay ability forces a ITelekinetic actor towards the
 * caster. @see ITelekinetic
 */
UCLASS()
class TD_API UPullGA : public UTDGA
{
    GENERATED_BODY()

public:
    UPullGA();

    /**
     * @brief Calls Pull for the activating player.
     * @see ATDCharacter::Pull
     */
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Telekinesis")
    USoundCue* TelekineseMiss = nullptr;

private:
    /**
     * @brief Calls Pull for the player if allowed.
     * @return Whether the pull succeeded.
     */
    bool Pull() const;

    /**
     * @brief Plays the telekinese miss sound effect.
     */
    void PlayTelekineseMissCue() const;
};
