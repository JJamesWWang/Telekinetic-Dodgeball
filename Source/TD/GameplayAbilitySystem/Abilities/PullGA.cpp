// Copyright 2021, James S. Wang, All rights reserved.

#include "PullGA.h"

#include "AbilitySystemComponent.h"
#include "GameConfiguration.h"
#include "GameRules/GameRules.h"
#include "GameState/TDGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Player/TDCharacter.h"
#include "Sound/SoundCue.h"

UPullGA::UPullGA()
{
    AbilityInputID = ETDAbilityInputID::Pull;
    AbilityName = "Pull";
}

void UPullGA::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    if (!CommitCheck(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
    }

    bool WasSuccessful = Pull();
    if (WasSuccessful)
    {
        CommitExecute(Handle, ActorInfo, ActivationInfo);
        ActorInfo->AbilitySystemComponent->NotifyAbilityCommit(this);
    }
    else
    {
        PlayTelekineseMissCue();
    }
    EndAbility(Handle, ActorInfo, ActivationInfo, true, !WasSuccessful);
}

bool UPullGA::Pull() const
{
    ATDCharacter* Character = Cast<ATDCharacter>(GetAvatarActorFromActorInfo());
    AGameRules* GameRules = GetGameRules();
    if (Character == nullptr)
    {
        LogInvalidPointer("UCastGA", "PredictPull", "Character");
        return false;
    }

    if (!GameRules->CanPull())
    {
        return false;
    }

    return Character->Pull();
}

void UPullGA::PlayTelekineseMissCue() const
{
    if (TelekineseMiss == nullptr)
    {
        LogInvalidPointer("UPullGA", "PlayTelekineseMissCue", "TelekineseMiss");
        return;
    }

    GetGameState()->PlayLocalCue(TelekineseMiss);
}
