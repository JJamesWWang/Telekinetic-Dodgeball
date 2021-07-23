// Copyright 2021, James S. Wang, All rights reserved.

#include "OrbState.h"

#include "EngineUtils.h"
#include "GameConfiguration.h"
#include "GameState/TDGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Orb/Orb.h"
#include "Player/TDCharacter.h"
#include "Sound/SoundCue.h"

#pragma region Initialization

UOrbState::UOrbState()
{
    PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicatedByDefault(true);
    ConstructorHelpers::FObjectFinder<USoundCue> OrbCollisionCueFile(
        TEXT("/Game/TD/SFX/A_OrbCollided_Cue.A_OrbCollided_Cue"));
    OrbCollisionCue = OrbCollisionCueFile.Object;
}

void UOrbState::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    TSet<FOrbCollision> OrbCollisionsCopy = TSet<FOrbCollision>(OrbCollisions);
    for (const FOrbCollision& OrbCollision : OrbCollisionsCopy)
    {
        if (OrbCollision.CollidedOrbs.Num() != 2)
        {
            OrbCollisions.Remove(OrbCollision);
            continue;
        }

        AOrb* OrbA = OrbCollision.CollidedOrbs[0];
        AOrb* OrbB = OrbCollision.CollidedOrbs[1];

        if (OrbA == nullptr || OrbB == nullptr)
        {
            OrbCollisions.Remove(OrbCollision);
            continue;
        }

        if (FVector::DistSquared(OrbA->GetActorLocation(),
                OrbB->GetActorLocation()) >
            FMath::Square<float>(OrbA->GetSimpleCollisionRadius() +
                                 OrbB->GetSimpleCollisionRadius() +
                                 COLLISION_CLEAR_DIST))
        {
            OrbCollisions.Remove(OrbCollision);
        }
    }
}

#pragma endregion

#pragma region Game State Events

void UOrbState::HandleRoundIsWaitingToStart()
{
    ResetPlayerOrbs();
}

void UOrbState::HandleMatchHasEnded()
{
    ResetPlayerOrbs();
}

#pragma endregion

#pragma region Orb Management

void UOrbState::ResetPlayerOrb(ATDCharacter* Player, AOrb* NewOrb)
{
    if (Player == nullptr || NewOrb == nullptr)
    {
        LogInvalidPointer("UOrbState", "ResetPlayerOrb", "Player or NewOrb");
        return;
    }

    AOrb** FoundOrb = PlayerOrbs.Find(Player);
    if (FoundOrb != nullptr)
    {
        AOrb* OldOrb = *FoundOrb;
        if (OldOrb != nullptr && IsValid(OldOrb))
        {
            OldOrb->Destroy();
        }
    }

    PlayerOrbs.Emplace(Player, NewOrb);
}

void UOrbState::ResetPlayerOrbs() const
{
    for (TActorIterator<AOrb> It(GetWorld()); It; ++It)
    {
        AOrb* Orb = *It;
        if (IsValid(Orb) && !Orb->IsActorBeingDestroyed())
        {
            Orb->Destroy();
        }
    }
}

#pragma endregion

#pragma region Orb Collisions

void UOrbState::HandleOrbImpact(AOrb* InstigatorOrb, const FHitResult& Hit)
{
    AOrb* HitOrb = Cast<AOrb>(Hit.GetActor());
    FOrbCollision OrbCollision = CreateOrbCollision(InstigatorOrb, HitOrb);
    if (HitOrb != nullptr &&
        HitOrb->GetTeam() != InstigatorOrb->GetTeam() &&
        !OrbCollisions.Contains(OrbCollision))
    {
        SwapOrbTeams(InstigatorOrb, HitOrb);
        OrbCollisions.Emplace(OrbCollision);
        PlayOrbCollisionCue(Hit.Location);
    }
}

void UOrbState::SwapOrbTeams(AOrb* InstigatorOrb, AOrb* HitOrb) const
{
    ETeamIndex ITeam = InstigatorOrb->GetTeam();
    ETeamIndex HTeam = HitOrb->GetTeam();
    InstigatorOrb->SetTeam(HTeam);
    HitOrb->SetTeam(ITeam);
}

FOrbCollision UOrbState::CreateOrbCollision(AOrb* InstigatorOrb,
    AOrb* HitOrb) const
{
    FOrbCollision OrbCollision;
    TArray<AOrb*> CollidedOrbs;
    CollidedOrbs.Emplace(InstigatorOrb);
    CollidedOrbs.Emplace(HitOrb);
    OrbCollision.CollidedOrbs = CollidedOrbs;
    return OrbCollision;
}

void UOrbState::PlayOrbCollisionCue(
    const FVector& Location) const
{
    if (OrbCollisionCue == nullptr)
    {
        LogInvalidPointer("UOrbState", "PlayOrbCollisionCue",
            "OrbCollisionCue");
        return;
    }

    TDGameState->PlayReplicatedCueAtLocation(OrbCollisionCue, Location);
}

#pragma endregion
