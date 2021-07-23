#pragma once

#include "BFPlayerMovementReplication.h"

#include "GameFramework/Character.h"
#include "BFPlayerMovement.h"

class UBFPlayerMovement;

void FSavedMove_BFCharacter::Clear()
{
    Super::Clear();
    FastFallValue = 0.0f;
    LastControlInputVector = FVector::ZeroVector;
}

bool FSavedMove_BFCharacter::CanCombineWith(
    const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
    FSavedMove_BFCharacter* NewMovePtr =
        static_cast<FSavedMove_BFCharacter*>(NewMove.Get());
    if (FastFallValue != NewMovePtr->FastFallValue)
    {
        return false;
    }
    if (LastControlInputVector != NewMovePtr->LastControlInputVector)
    {
        return false;
    }

    return Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void FSavedMove_BFCharacter::SetMoveFor(ACharacter* C, float InDeltaTime,
    FVector const& NewAccel,
    FNetworkPredictionData_Client_Character& ClientData)
{
    Super::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

    UBFPlayerMovement* Movement =
        Cast<UBFPlayerMovement>(C->GetCharacterMovement());
    if (Movement)
    {
        FastFallValue = Movement->LastFastFallValue;
        LastControlInputVector = Movement->LastControlInputVector;
    }
}

void FSavedMove_BFCharacter::PrepMoveFor(ACharacter* C)
{
    Super::PrepMoveFor(C);

    UBFPlayerMovement* Movement =
        Cast<UBFPlayerMovement>(C->GetCharacterMovement());
    if (Movement)
    {
        Movement->LastFastFallValue = FastFallValue;
        Movement->LastControlInputVector = LastControlInputVector;
    }
}

bool FBFCharacterNetworkMoveData::Serialize(
    UCharacterMovementComponent& CharacterMovement, FArchive& Ar,
    UPackageMap* PackageMap, ENetworkMoveType MoveType)
{
    Super::Serialize(CharacterMovement, Ar, PackageMap, MoveType);
    SerializeOptionalValue<float>(Ar.IsSaving(), Ar, FastFallValue, 0.0f);
    SerializeOptionalValue<FVector>(
        Ar.IsSaving(), Ar, LastControlInputVector, FVector::ZeroVector);
    return !Ar.IsError();
}

void FBFCharacterNetworkMoveData::ClientFillNetworkMoveData(
    const FSavedMove_Character& ClientMove, ENetworkMoveType MoveType)
{
    Super::ClientFillNetworkMoveData(ClientMove, MoveType);
    const FSavedMove_BFCharacter& Move =
        static_cast<const FSavedMove_BFCharacter&>(ClientMove);
    FastFallValue = Move.FastFallValue;
    LastControlInputVector = Move.LastControlInputVector;
}

FNetworkPredictionData_Client_BFCharacter::
FNetworkPredictionData_Client_BFCharacter(
    const UCharacterMovementComponent& ClientMovement)
    : FNetworkPredictionData_Client_Character(ClientMovement)
{
}

FSavedMovePtr FNetworkPredictionData_Client_BFCharacter::AllocateNewMove()
{
    return MakeShared<FSavedMove_BFCharacter>();
}

FBFCharacterNetworkMoveDataContainer::FBFCharacterNetworkMoveDataContainer()
{
    NewMoveData = &BFDefaultMoveData[0];
    PendingMoveData = &BFDefaultMoveData[1];
    OldMoveData = &BFDefaultMoveData[2];
}
