// Copyright 2021, James S. Wang, All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TDGA.h"
#include "PushGA.generated.h"

class USoundCue;
/**
 * @brief The Push gameplay ability forces a ITelekinetic actor away from the
 * caster. @see ITelekinetic
 */
UCLASS()
class TD_API UPushGA : public UTDGA
{
    GENERATED_BODY()

public:
    UPushGA();

    /**
     * @brief Calls Push for the activating player.
     * @see ATDCharacter::Push
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
     * @brief Calls Push for the player if allowed.
     * @return Whether the push succeeded.
     */
    bool Push() const;

    /**
     * @brief Plays the telekinese miss sound effect.
     */
    void PlayTelekineseMissCue() const;
};
