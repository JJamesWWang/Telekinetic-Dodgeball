// Copyright 2021, James S. Wang, All rights reserved.

#include "OrbMovement.h"

#include "DrawDebugHelpers.h"
#include "Propellable.h"
#include "GameConfiguration.h"
#include "Player/TDCharacter.h"

UOrbMovement::UOrbMovement()
{
    bShouldBounce = true;
    bRotationFollowsVelocity = true;
    ProjectileGravityScale = 0.0f;
    Bounciness = 1.0f;
    Friction = 0.0f;
}

void UOrbMovement::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (PlayerVForce != FVector::ZeroVector)
    {
        Velocity += PlayerVForce * DeltaTime;
        Velocity = LimitVelocity(Velocity);
    }
}

void UOrbMovement::HandleImpact(const FHitResult& Hit, float TimeSlice,
    const FVector& MoveDelta)
{
    bool bStopSimulating = false;

    if (bShouldBounce)
    {
        OnProjectileImpact.Broadcast(Hit, Velocity);

        // Wrap the "bounce" code with a check for if we should push this
        // actor. If we shouldn't push, then proceed as normal, otherwise the
        // projectile movement code will defer to a call to HandleDeflection()
        // We then override HandleDeflection() to push instead of deflect.
        UPrimitiveComponent* Component = Hit.GetComponent();
        if (Component == nullptr || !ChannelsToPropel.Contains(
                Component->GetCollisionObjectType()))
        {
            const FVector OldVelocity = Velocity;
            Velocity = ComputeBounceResult(Hit, TimeSlice, MoveDelta);

            // Trigger bounce events
            OnProjectileBounce.Broadcast(Hit, OldVelocity);

            // Event may modify velocity or threshold, so check velocity
            // threshold now.
            Velocity = LimitVelocity(Velocity);
            if (IsVelocityUnderSimulationThreshold())
            {
                bStopSimulating = true;
            }
        }
    }
    else
    {
        bStopSimulating = true;
    }

    if (bStopSimulating)
    {
        StopSimulating(Hit);
    }
}

bool UOrbMovement::HandleDeflection(FHitResult& Hit, const FVector& OldVelocity,
    const uint32 NumBounces, float& SubTickTimeRemaining)
{
    PropelActor(Hit, OldVelocity);
    return true;
}

void UOrbMovement::PropelActor(FHitResult& Hit,
    const FVector& OldVelocity) const
{
    IPropellable* Actor = Cast<IPropellable>(Hit.GetActor());
    if (Actor != nullptr)
    {
        Actor->OnPropelled(OldVelocity);
    }
}

void UOrbMovement::OnPushed(ATDCharacter* Player, const FHitResult& Hit)
{
    if (Player == nullptr)
    {
        LogInvalidPointer("UOrbMovement", "OnPushed", "Player");
        return;
    }

    AActor* Owner = GetOwner();
    const FVector PushDirection = (Owner->GetActorLocation() - Hit.ImpactPoint).
        GetSafeNormal();
    Telekinese(Player, Hit, PushDirection);
}

void UOrbMovement::OnPulled(ATDCharacter* Player, const FHitResult& Hit)
{
    if (Player == nullptr)
    {
        LogInvalidPointer("UOrbMovement", "OnPushed", "Player");
        return;
    }

    AActor* Owner = GetOwner();
    const FVector PullDirection = (Hit.ImpactPoint - Owner->GetActorLocation()).
        GetSafeNormal();
    Telekinese(Player, Hit, PullDirection);
}

void UOrbMovement::SetPlayerVForce(const FVector& Force)
{
    PlayerVForce = Force * VForceMultiplier;
}

void UOrbMovement::Telekinese(ATDCharacter* Player,
    const FHitResult& Hit, const FVector& ForceDirection)
{
    Velocity = LimitVelocity(CalculateRedirectVelocity(Hit, ForceDirection));
    SetPlayerVForce(Player->GetVelocity());
}

FVector UOrbMovement::CalculateRedirectVelocity(const FHitResult& Hit,
    const FVector& ForceDirection) const
{
    FVector NewVelocity = Velocity;
    const float SpeedFactor = CalculateSpeedFactor(Hit);

    // Add the telekinetic force in the force direction
    NewVelocity += ForceDirection * TelekineticSpeed * SpeedFactor;
    const float ForceDirectionDot = FVector::DotProduct(NewVelocity,
        ForceDirection);

    // If in the right direction and fast enough, return without adjustment.
    if (ForceDirectionDot > 0.0f && ForceDirectionDot >= InitialSpeed)
    {
        return NewVelocity;
    }

    // Else add speed in the force direction until it reaches the min speed.
    const FVector MinRedirectVelocity =
        ForceDirection * InitialSpeed * SpeedFactor;
    NewVelocity += MinRedirectVelocity - FVector::DotProduct(NewVelocity,
        ForceDirection) * ForceDirection;
    return NewVelocity;
}

float UOrbMovement::CalculateSpeedFactor(const FHitResult& Hit) const
{
    AActor* Owner = GetOwner();
    const FVector TraceDirection = (Hit.TraceEnd - Hit.TraceStart).
        GetSafeNormal();
    const FVector OrbDirection = (Owner->GetActorLocation() - Hit.TraceStart).
        GetSafeNormal();
    return FVector::DotProduct(TraceDirection, OrbDirection);
}
