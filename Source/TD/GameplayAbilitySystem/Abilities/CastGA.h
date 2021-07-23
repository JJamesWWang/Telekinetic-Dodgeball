// Copyright 2021, James S. Wang, All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TDGA.h"
#include "CastGA.generated.h"

class USoundCue;
class AOrb;
/**
 * @brief The Cast gameplay ability represents the player spawning an AOrb
 * that they and their teammates can push and pull around.
 */
UCLASS()
class TD_API UCastGA : public UTDGA
{
    GENERATED_BODY()

public:
    UCastGA();

    /**
     * @brief Casts the orb and handles effected results.
     */
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

protected:
    /**
     * @brief Sound effect for casting an orb.
     */
    UPROPERTY(EditAnywhere, Category = "Cast")
    USoundCue* CastCue = nullptr;

private:
    /**
     * @brief Casts an orb for the casting player if allowed.
     * @return Whether the cast succeeded.
     */
    bool CastOrb(bool HasAuthority) const;

    /**
     * @brief Plays the cast cue sound effect at the location.
     */
    void PlayCastCue(const FVector& Location) const;
};
