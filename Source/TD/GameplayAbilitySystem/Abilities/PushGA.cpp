// Copyright 2021, James S. Wang, All rights reserved.

#include "PushGA.h"

#include "AbilitySystemComponent.h"
#include "GameConfiguration.h"
#include "GameRules/GameRules.h"
#include "GameState/TDGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Player/TDCharacter.h"
#include "Sound/SoundCue.h"

UPushGA::UPushGA()
{
    AbilityInputID = ETDAbilityInputID::Push;
    AbilityName = "Push";
}

void UPushGA::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    if (!CommitCheck(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
    }

    bool WasSuccessful = Push();
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

bool UPushGA::Push() const
{
    ATDCharacter* Character = Cast<ATDCharacter>(GetAvatarActorFromActorInfo());
    AGameRules* GameRules = GetGameRules();
    if (Character == nullptr || GameRules == nullptr)
    {
        LogInvalidPointer("UCastGA", "PredictPush", "Character or GameRules");
        return false;
    }

    if (!GameRules->CanPush())
    {
        return false;
    }

    return Character->Push();
}

void UPushGA::PlayTelekineseMissCue() const
{
    if (TelekineseMiss == nullptr)
    {
        LogInvalidPointer("UPushGA", "PlayTelekineseMissCue", "TelekineseMiss");
        return;
    }

    GetGameState()->PlayLocalCue(TelekineseMiss);
}
