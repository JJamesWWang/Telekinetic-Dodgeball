// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PBMovement/PBPlayerMovement.h"
#include "CoreMinimal.h"
#include "BFPlayerMovementReplication.h"

#include "BFPlayerMovement.generated.h"

struct FBFCharacterNetworkMoveDataContainer;

/**
 * @brief Overridden movement from PB Source-style movement that implements the
 * crouch sliding and fast falling abilities. Multiplayer compatible.
 */
UCLASS()
class TD_API UBFPlayerMovement : public UPBPlayerMovement
{
    GENERATED_BODY()

public:
    /**
     * @brief The network move container for our custom replicated data.
     */
    FBFCharacterNetworkMoveDataContainer NetworkMoveDataContainer;

    UBFPlayerMovement();

    /**
     * @brief Crouches and sets ground friction to 0 for sliding.
     * @param bClientSimulation Whether this is a simulating client.
     */
    virtual void Crouch(bool bClientSimulation) override;

    /**
     * @brief Un-crouches and restores ground friction to 4.
     * @param bClientSimulation Whether this is a simulating client.
     */
    virtual void UnCrouch(bool bClientSimulation) override;

    /**
     * @brief The gravity to apply to the character when fast-falling.
     */
    UPROPERTY(EditAnywhere)
    float FastFallGravity = 2.0f;

    /**
     * @brief The last axis value for fast-falling.
     */
    float LastFastFallValue = 0.0f;

    /**
     * @brief Stores the fast-fall value into @see LastFastFallValue.
     * @param Value The axis value for the fast-fall input.
     */
    void FastFall(float Value);

    /**
     * @brief Event triggered at the end of a movement update.
     * Sets the gravity scale according to the last fast-fall value and
     * also stores the input vector for replication.
     *
     * @param DeltaSeconds The time that's passed since the last tick.
     * @param OldLocation The old location of the character.
     * @param OldVelocity The old velocity of the character.
     */
    virtual void OnMovementUpdated(float DeltaSeconds,
        const FVector& OldLocation, const FVector& OldVelocity) override;

    /**
     * @brief This is the player's last movement input as a vector. This isn't
     * natively replicated across the network, but we need to because the PB
     * Source jumping uses it for b-hopping.
     */
    FVector LastControlInputVector = FVector::ZeroVector;

    /**
     * @brief Get prediction data for a client game. Allocates a new @see
     * FNetworkPredictionData_Client_BFCharacter (so we use our custom data
     * struct).
     * @return The pointer to the prediction data.
     */
    virtual FNetworkPredictionData_Client* GetPredictionData_Client()
    const override;

    /**
     * @brief Process a move at the given time stamp, given the compressed flags
     * representing various events that occurred (ie jump). Retrieves data from
     * current network move data and sets the last fall value and input vector.
     *
     * @param ClientTimeStamp Time stamp for this move.
     * @param DeltaTime The time that's passed.
     * @param CompressedFlags Flags from a saved move to set state.
     * @param NewAccel The character's new acceleration.
     */
    virtual void MoveAutonomous(float ClientTimeStamp, float DeltaTime,
        uint8 CompressedFlags, const FVector& NewAccel) override;
};
