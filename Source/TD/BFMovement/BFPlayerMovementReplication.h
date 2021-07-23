#pragma once

#include "GameFramework/CharacterMovementComponent.h"

/**
 * @brief This is the struct that stores the variables that you want to
 * replicate.
 */
class TD_API FSavedMove_BFCharacter : public FSavedMove_Character
{
public:
    /**
     * @brief Axis value for how much fast fall gravity to apply.
     */
    float FastFallValue = 0.0f;

    /**
     * @brief Store player's last input as it's used in jump calculations for
     * PBMovement.
     */
    FVector LastControlInputVector = FVector::ZeroVector;

    /**
     * @brief Called to set up this saved move (when initially created) to make
     * a predictive correction in case a packet is dropped.
     * @param C Character actor to replicate.
     * @param InDeltaTime The time passed.
     * @param NewAccel The new acceleration for this character.
     * @param ClientData Prediction data.
     */
    virtual void SetMoveFor(ACharacter* C, float InDeltaTime,
        FVector const& NewAccel,
        FNetworkPredictionData_Client_Character& ClientData) override;

    /**
     * @brief Called before ClientUpdatePosition uses this SavedMove to make a
     * predictive correction in the case a packet is dropped.
     * @param C The character actor.
     */
    virtual void PrepMoveFor(ACharacter* C) override;

    /**
     * @brief Combine this move with an older move and update relevant state.
     * @param NewMove The new move to try to combine with.
     * @param InCharacter The character actor.
     * @param MaxDelta The farthest behind we're allowed to be after receiving a
     * new server time.
     * @return Whether the new move can be combined with this move.
     */
    virtual bool CanCombineWith(const FSavedMovePtr& NewMove,
        ACharacter* InCharacter, float MaxDelta) const override;

    /**
     * @brief Clear saved move properties, so it can be re-used.
     */
    virtual void Clear() override;

private:
    typedef FSavedMove_Character Super;
};

/**
 * @brief This is the struct that gets sent across the network for replication.
 * It usually contains everything that @see FSavedMove_Character has.
 */
struct TD_API FBFCharacterNetworkMoveData : FCharacterNetworkMoveData
{
public:
    /** @see FSavedMove_BFCharacter */
    float FastFallValue = 0.0f;
    /** @see FSavedMove_BFCharacter */
    FVector LastControlInputVector = FVector::ZeroVector;

    /**
     * @brief Given a FSavedMove_Character from UCharacterMovementComponent,
     * fill in data in this struct with relevant movement data. Note that the
     * instance of the FSavedMove_Character is likely a custom struct of a
     * derived struct of your own, if you have added your own saved move data.
     * @see UCharacterMovementComponent::AllocateNewMove()
     *
     * @param ClientMove The move that contains the desired replication data.
     * @param MoveType Whether the move is old, pending, or new.
     */
    virtual void ClientFillNetworkMoveData(
        const FSavedMove_Character& ClientMove,
        ENetworkMoveType MoveType) override;

    /**
     * @brief Serialize the data in this struct to or from the given FArchive.
     * This packs or unpacks the data in to a variable-sized data stream that is
     * sent over the network from client to server.
     * @see UCharacterMovementComponent::CallServerMovePacked
     *
     * @param CharacterMovement The character movement component.
     * @param Ar The archive to use for serializing.
     * @param PackageMap Maps objects and names to and from indices for network
     * communication.
     * @param MoveType Whether the move is old, pending, or new.
     * @return Whether serialization succeeded.
     */
    virtual bool Serialize(UCharacterMovementComponent& CharacterMovement,
        FArchive& Ar, UPackageMap* PackageMap,
        ENetworkMoveType MoveType) override;

private:
    typedef FCharacterNetworkMoveData Super;
};

/**
 * @brief Prediction data for the client, keeps track of saved moves and
 * acknowledges them. We only override this to change what struct new moves
 * allocate (@see FSavedMove_BFCharacter)
 */
class FNetworkPredictionData_Client_BFCharacter
    : public FNetworkPredictionData_Client_Character
{
public:
    FNetworkPredictionData_Client_BFCharacter(
        const UCharacterMovementComponent& ClientMovement);

    /**
     * @brief Allocates a new saved move.
     * @return The @see FSavedMove_BFCharacter.
     */
    virtual FSavedMovePtr AllocateNewMove() override;

private:
    typedef FNetworkPredictionData_Client_Character Super;
};

/**
 * @brief Struct used for network RPC parameters between client/server by
 * ACharacter and UCharacterMovementComponent. To extend network move data and
 * add custom parameters, you typically override this struct with a custom
 * derived struct and set the CharacterMovementComponent to use your container
 * with UCharacterMovementComponent::SetNetworkMoveDataContainer(). Your derived
 * struct would then typically (in the constructor) replace the NewMoveData,
 * PendingMoveData, and OldMoveData pointers to use your own instances of a
 * struct derived from FCharacterNetworkMoveData, where you add custom fields
 * and implement custom serialization to be able to pack and unpack your own
 * additional data.
 *
 * @see UCharacterMovementComponent::SetNetworkMoveDataContainer()
 */
struct TD_API FBFCharacterNetworkMoveDataContainer
    : FCharacterNetworkMoveDataContainer
{
public:
    FBFCharacterNetworkMoveDataContainer();

private:
    typedef FCharacterNetworkMoveDataContainer Super;

    /**
     * @brief Overridden NetworkMoveData to store custom data.
     */
    FBFCharacterNetworkMoveData BFDefaultMoveData[3];
};
