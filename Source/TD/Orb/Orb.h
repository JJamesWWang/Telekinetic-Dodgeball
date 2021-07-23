// Copyright 2021, James S. Wang, All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "TeamAssignable.h"
#include "Telekinetic.h"
#include "GameFramework/Actor.h"
#include "Orb.generated.h"

class USoundCue;
class USphereComponent;
class UOrbMovement;

/**
 * @brief An Orb is basically a dodgeball; a player throws one through casting,
 * other players can change its direction via telekinesis, and if one of your
 * team's orbs hits an opponent, they are eliminated.
 */
UCLASS()
class TD_API AOrb : public AActor, public ITelekinetic, public ITeamAssignable
{
    GENERATED_BODY()

#pragma region Initialization

public:
    AOrb();
    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    /**
     * @brief Sets the team on the server after the Orb is cast.
     */
    virtual void PostActorCreated() override;

    /**
     * @brief Initializes the movement component's player velocity force.
     * @param Velocity The player's velocity as a force.
     */
    void InitPlayerVForce(const FVector& Velocity) const;

protected:
    /**
     * @brief Binds to the OrbMovement components' OnProjectileImpact event.
     */
    virtual void BeginPlay() override;

    /**
	 * @brief The sphere the bullet uses to detect collision.
	 */
    UPROPERTY(VisibleAnywhere)
    USphereComponent* Collider = nullptr;

    /**
     * @brief The internal particle-like mesh for the orb.
     */
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* InternalMesh = nullptr;

    /**
     * @brief The external skeleton-like mesh for the orb.
     */
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* ExternalMesh = nullptr;

    /**
	 * @brief The component used to dictate how the orb moves.
	 */
    UPROPERTY(VisibleAnywhere)
    UOrbMovement* Movement = nullptr;

private:
    void CreateCollider();
    void CreateStaticMesh();
    void CreateMovement();

#pragma endregion

#pragma region Collision

protected:
    /**
     * @brief The sound effect to play when colliding with a non-orb actor.
     */
    UPROPERTY(EditAnywhere, Category = "Collision")
    USoundCue* BounceCue = nullptr;

private:
    /**
     * @brief Requests the TDGameMode to eliminate a player if the orb hits a
     * player of a different team from this orb.
     */
    void OnOrbImpact(const FHitResult& Hit, const FVector& OrbVelocity);

    /**
     * @brief Plays the bounce cue if not colliding with another Orb.
     */
    UFUNCTION()
    void OnOrbBounce(const FHitResult& Hit, const FVector& OldVelocity);

private:
    /**
     * @brief Plays the bounce sound effect.
     */
    void PlayBounceCue() const;

#pragma endregion

#pragma region ITelekinetic

public:
    /** 
     * @brief Calls OrbMovement::PushTowards in the line trace direction.
     * @see OrbMovement::PushTowards
     */
    virtual void OnPushed(ATDCharacter* Player, const FHitResult& Hit) override;

    /**
     * @brief Calls OrbMovement::PullTowards in the reversed line trace
     * @see OrbMovement::PullTowards
     */
    virtual void OnPulled(ATDCharacter* Player, const FHitResult& Hit) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Telekinesis")
    USoundCue* TelekineseHit = nullptr;

private:
    /**
     * @brief Check for whether the this orb can be telekinesed by the player.
     */
    bool CanBeTelekinesedBy(ATDCharacter* Player) const;

    /**
     * @brief Plays the sound effect for being telekinesed.
     */
    void PlayTelekineseHitCue() const;

#pragma endregion

#pragma region ITeamAssignable

public:
    /**
     * @brief Initially the same as the team of the character that spawned it.
     * Swaps teams when it collides with an orb (to the other orb's team).
     */
    virtual ETeamIndex GetTeam() const override;

    /**
     * @brief Sets the orb's team to the provided ETeamIndex.
     */
    void SetTeam(const ETeamIndex TeamIndex);

protected:
    /**
     * @brief The corresponding internal material to use for each ETeamIndex.
     * Index 0 is ETeamIndex::None. @see ETeamIndex.
     */
    UPROPERTY(EditAnywhere, Category = "Team")
    TArray<UMaterialInstance*> InternalMeshTeamMaterials;

    /**
     * @brief The corresponding external material to use for each ETeamIndex.
     * Index 0 is ETeamIndex::None. @see ETeamIndex.
     */
    UPROPERTY(EditAnywhere, Category = "Team")
    TArray<UMaterialInstance*> ExternalMeshTeamMaterials;

private:
    /**
     * @brief The team that this orb belongs to.
     */
    UPROPERTY(Replicated, ReplicatedUsing=OnRep_Team)
    ETeamIndex Team = ETeamIndex::None;

    UFUNCTION()
    void OnRep_Team();

    /**
     * @brief Sets the team to the same team as the player who cast this orb.
     */
    void SetInitialTeam();

    /**
     * @brief Callback for after the orb's team is set.
     */
    void OnTeamSet();

#pragma endregion
};
