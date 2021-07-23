// Fill out your copyright notice in the Description page of Project Settings.

#include "BFPlayerMovement.h"

#include "GameFramework/Character.h"

UBFPlayerMovement::UBFPlayerMovement()
    : Super()
{
    MaxWalkSpeed = 750.0f;       // Same as wall jump speed.
    MaxWalkSpeedCrouched = 0.0f; // Don't allow crouch walking
    JumpZVelocity = 381.0f;      // Some magic value that felt right

    SetNetworkMoveDataContainer(NetworkMoveDataContainer);
}

void UBFPlayerMovement::Crouch(bool bClientSimulation)
{
    GroundFriction = 0.0f;
    Super::Crouch(bClientSimulation);
}

void UBFPlayerMovement::UnCrouch(bool bClientSimulation)
{
    GroundFriction = 4.0f;
    Super::UnCrouch(bClientSimulation);
}

void UBFPlayerMovement::FastFall(float Value)
{
    LastFastFallValue = Value;
}

void UBFPlayerMovement::OnMovementUpdated(
    float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
    Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
    GravityScale = FMath::Max<float>(1.0f, FastFallGravity * LastFastFallValue);
    LastControlInputVector = GetCharacterOwner()->GetLastMovementInputVector();
}

FNetworkPredictionData_Client*
UBFPlayerMovement::GetPredictionData_Client() const
{
    if (ClientPredictionData == nullptr)
    {
        UBFPlayerMovement* MutableThis = const_cast<UBFPlayerMovement*>(this);
        MutableThis->ClientPredictionData =
            new FNetworkPredictionData_Client_BFCharacter(*this);
    }

    return ClientPredictionData;
}

void UBFPlayerMovement::MoveAutonomous(float ClientTimeStamp, float DeltaTime,
    uint8 CompressedFlags, const FVector& NewAccel)
{
    FBFCharacterNetworkMoveData* Move =
        static_cast<FBFCharacterNetworkMoveData*>(GetCurrentNetworkMoveData());
    if (Move != nullptr)
    {
        LastFastFallValue = Move->FastFallValue;
        LastControlInputVector = Move->LastControlInputVector;
    }
    Super::MoveAutonomous(
        ClientTimeStamp, DeltaTime, CompressedFlags, NewAccel);
}
