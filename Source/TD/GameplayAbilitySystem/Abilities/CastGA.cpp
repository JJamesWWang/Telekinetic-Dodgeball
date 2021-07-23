// Copyright 2021, James S. Wang, All rights reserved.

#include "CastGA.h"

#include "AbilitySystemComponent.h"
#include "Player/TDCharacter.h"
#include "GameConfiguration.h"
#include "GameRules/GameRules.h"
#include "GameState/TDGameState.h"
#include "GameState/Components/OrbState.h"
#include "Kismet/GameplayStatics.h"
#include "Orb/Orb.h"
#include "Sound/SoundCue.h"

UCastGA::UCastGA()
{
    AbilityInputID = ETDAbilityInputID::Cast;
    AbilityName = "Cast";
}

void UCastGA::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    if (!CommitCheck(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
    }

    bool WasSuccessful = CastOrb(HasAuthority(&ActivationInfo));
    if (WasSuccessful)
    {
        CommitExecute(Handle, ActorInfo, ActivationInfo);
        ActorInfo->AbilitySystemComponent->NotifyAbilityCommit(this);
    }
    EndAbility(Handle, ActorInfo, ActivationInfo, true, !WasSuccessful);
}

bool UCastGA::CastOrb(bool HasAuthority) const
{
    ATDCharacter* Character = Cast<ATDCharacter>(
        GetAvatarActorFromActorInfo());
    AGameRules* GameRules = GetGameRules();
    if (Character == nullptr || GameRules == nullptr)
    {
        LogInvalidPointer("UCastGA", "ResetPlayerOrb",
            "GameMode or Character");
        return false;
    }

    if (!GameRules->CanCastOrb())
    {
        return false;
    }

    if (HasAuthority)
    {
        AOrb* Orb = Character->CastOrb();
        UOrbState* OrbState = GetGameState()->GetOrbStateComponent();
        OrbState->ResetPlayerOrb(Character, Orb);
        PlayCastCue(Orb->GetActorLocation());
    }
    return true;
}

void UCastGA::PlayCastCue(const FVector& Location) const
{
    if (CastCue == nullptr)
    {
        LogInvalidPointer("UCastGA", "PlayCastCue", "CastCue");
        return;
    }

    GetGameState()->PlayReplicatedCueAtLocation(CastCue, Location);
}
