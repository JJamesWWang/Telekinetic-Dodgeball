// Copyright 2021, James S. Wang, All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "TDGameStateComponent.h"
#include "TDTypes.h"

#include "OrbState.generated.h"

class USoundCue;
class ATDCharacter;
class AOrb;

#define COLLISION_CLEAR_DIST 10.0f

USTRUCT()
struct FOrbCollision
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<AOrb*> CollidedOrbs;

    bool operator==(const FOrbCollision& Other) const
    {
        if (CollidedOrbs.Num() != 2 || Other.CollidedOrbs.Num() != 2)
        {
            return false;
        }

        for (AOrb* Orb : CollidedOrbs)
        {
            if (Orb == nullptr || !Other.CollidedOrbs.Contains(Orb))
            {
                return false;
            }
        }
        return true;
    }
};

FORCEINLINE uint32 GetTypeHash(const FOrbCollision& OrbCollision)
{
    return FCrc::MemCrc32(&OrbCollision, sizeof(FOrbCollision));
}

/**
 * @brief This game state component handles everything dealing with Orbs.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TD_API UOrbState : public UTDGameStateComponent
{
    GENERATED_BODY()

#pragma region Initialization

public:
    UOrbState();
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

#pragma endregion

#pragma region Game State Events
    /** @see UTDGameStateComponent */

protected:
    virtual void HandleRoundIsWaitingToStart() override;
    virtual void HandleMatchHasEnded() override;

#pragma endregion

#pragma region Orb Management

public:
    /**
     * @brief Despawns a player's orb if they have already cast one.
     * @param Player The player that casted a new orb.
     * @param NewOrb The new orb that the player casted.
     */
    void ResetPlayerOrb(ATDCharacter* Player, AOrb* NewOrb);

    /**
     * @brief Despawns all orbs.
     */
    void ResetPlayerOrbs() const;

private:
    /**
     * @brief Maps players to the orb that they have casted so that we know what
     * orb to destroy when they cast a new one.
     */
    UPROPERTY()
    TMap<ATDCharacter*, AOrb*> PlayerOrbs;

#pragma endregion

#pragma region Orb Collisions

public:
    /**
     * @brief Called when an Orb impacts something else.
     */
    void HandleOrbImpact(AOrb* InstigatorOrb, const FHitResult& Hit);

    /**
     * @brief Swaps the teams of two orbs that hit each other.
     * @param InstigatorOrb The orb that caused the hit.
     * @param HitOrb The orb that is being hit.
     */
    void SwapOrbTeams(AOrb* InstigatorOrb, AOrb* HitOrb) const;

protected:
    /**
     * @brief The sound effect to play when colliding with other orbs.
     */
    UPROPERTY(EditAnywhere, Category = "Collision")
    USoundCue* OrbCollisionCue = nullptr;

private:
    /**
     * @brief Maps orbs that have swapped teams to their original teams.
     */
    UPROPERTY()
    TMap<AOrb*, ETeamIndex> OrbsOriginalTeams;

    UPROPERTY()
    TArray<FOrbCollision> OrbCollisions;

    UPROPERTY()
    TSet<FOrbCollision> AcknowledgedOrbCollisions;

    /**
     * @brief Creates an FOrbCollision from collision parameters.
     */
    FOrbCollision CreateOrbCollision(AOrb* InstigatorOrb, AOrb* HitOrb) const;

    void PlayOrbCollisionCue(const FVector& Location) const;

#pragma endregion
};
