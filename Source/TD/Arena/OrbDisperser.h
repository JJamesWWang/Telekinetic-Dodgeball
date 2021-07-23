// Copyright 2021, James S. Wang, All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Actor.h"
#include "OrbDisperser.generated.h"

class USoundCue;
class UBoxComponent;

/**
 * @brief The OrbDisperser destroys orbs that go through it.
 */
UCLASS()
class TD_API AOrbDisperser : public AActor
{
    GENERATED_BODY()

#pragma region Initialization

public:
    AOrbDisperser();

protected:
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* StaticMesh = nullptr;

    UPROPERTY(VisibleAnywhere)
    UBoxComponent* Collider = nullptr;

    /**
     * @brief Binds to the Collider's OnComponentBeginOverlap
     */
    virtual void BeginPlay() override;

#pragma endregion

#pragma region Dispersing

protected:
    /**
     * @brief The sound effect to play when an orb is being dispersed.
     */
    UPROPERTY(EditAnywhere)
    USoundCue* DispersedCue = nullptr;

private:
    /**
     * @brief If the other actor is an orb, destroy it.
     */
    UFUNCTION()
    void OnOverlapped(UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor, UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex, bool bFromSweep,
        const FHitResult& SweepResult);

    /**
     * @brief Plays the dispersed sound effect at the provided location.
     */
    void PlayDispersedCue(const FVector& Location) const;

#pragma endregion
};
