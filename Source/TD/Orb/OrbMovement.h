// Copyright 2021, James S. Wang, All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "OrbMovement.generated.h"

class ATDCharacter;

/**
 * @brief Defines the movement for AOrb. Handles pushing and telekinetic forces.
 */
UCLASS()
class TD_API UOrbMovement : public UProjectileMovementComponent
{
    GENERATED_BODY()

    DECLARE_EVENT_TwoParams(UOrbMovement, FOnProjectileImpactDelegate,
        const FHitResult& /* Hit */, const FVector& /* OldVelocity */);

public:
    UOrbMovement();

    /**
     * @brief Sets the player's velocity force.
     */
    void SetPlayerVForce(const FVector& Force);

#pragma region Physics Tick

public:
    /**
     * @brief Callback that occurs before FOnProjectileBounceDelegate. This
     * exists because due to the way we've modified Orb movement, OnBounce isn't
     * always called (say, if the Orb wants to push an actor). As such, OnImpact
     * will be the new delegate that's always called upon a collision.
     *
     * @param const FHitResult& ImpactResult: The hit result.
     * @param const FVector& ProjectileVelocity: The velocity of the projectile.
     * This is not the velocity after a bounce, as this is broadcast when we do
     * not know if we should bounce yet.
     */
    FOnProjectileImpactDelegate OnProjectileImpact;

    /**
     * @brief Adds the PlayerVForce each tick if it's nonzero.
     */
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

protected:
    /**
     * @brief Modified to check if this orb should bounce off of the hit actor
     * (no if we want to push the actor) instead of always bouncing.
     */
    virtual void HandleImpact(const FHitResult& Hit, float TimeSlice,
        const FVector& MoveDelta) override;

    /**
     * @brief Calls PropelActor instead of deflecting off of other actors. 
     * @see PropelActor
     */
    virtual bool HandleDeflection(FHitResult& Hit, const FVector& OldVelocity,
        const uint32 NumBounces, float& SubTickTimeRemaining) override;

protected:
    /**
     * @brief Collision channels to push. These must be collide-able with the
     * orb to begin with.
     */
    UPROPERTY(EditAnywhere, Category = Propelling)
    TArray<TEnumAsByte<ECollisionChannel>> ChannelsToPropel;

    /**
     * @brief Calls IPropellable::OnPropelled on the collided actor.
     * @see IPropellable::OnPropelled
     */
    void PropelActor(FHitResult& Hit, const FVector& OldVelocity) const;

#pragma endregion

#pragma region Physics Manipulation

public:
    /** @see ITelekinetic */
    void OnPushed(ATDCharacter* Player, const FHitResult& Hit);
    void OnPulled(ATDCharacter* Player, const FHitResult& Hit);

protected:
    /**
     * @brief How much speed to add to the velocity when pushed or pulled.
     */
    UPROPERTY(EditAnywhere, Category = "Telekinetic Physics")
    float TelekineticSpeed = 1000.0f;

    /**
     * @brief How much the player's velocity affects the orb's trajectory.
     */
    UPROPERTY(EditAnywhere, Category = "Telekinetic Physics")
    float VForceMultiplier = 1.0f;

private:
    /**
     * @brief The player's velocity as a constant force applied to the orb.
     */
    UPROPERTY(VisibleInstanceOnly, Category = "Telekinetic Physics",
        meta = (AllowPrivateAccess = "true"))
    FVector PlayerVForce = FVector::ZeroVector;

    /**
     * @brief Adds a telekinetic force as instigated by a player.
     * @param Player The player that's pushing or pulling the orb. 
     * @param Hit The hit result from the telekinetic line trace.
     * @param ForceDirection The direction to apply the telekinetic force.
     */
    void Telekinese(ATDCharacter* Player, const FHitResult& Hit,
        const FVector& ForceDirection);

    /**
     * @brief Calculates the velocity of an orb after a player redirects it.
     * Adds to the current velocity and then adds until velocity in the
     * direction of the telekinese meets the InitialSpeed.
     *
     * @param Hit The hit result from the telekinetic line trace.
     * @param ForceDirection The direction to apply the telekinetic force.
     * @return The new velocity of the orb after a telekinetic redirect.
     */
    FVector CalculateRedirectVelocity(const FHitResult& Hit,
        const FVector& ForceDirection) const;

    /**
     * @brief Calculates and returns the factor by which the telekinetic speed
     * should be added to the orb's velocity. Finds how much the ImpactPoint
     * aligns with the center of the orb on a scale from 0 to 1 (dot product).
     *
     * @param Hit The hit result from the telekinetic line trace.
     */
    float CalculateSpeedFactor(const FHitResult& Hit) const;

#pragma endregion
};
