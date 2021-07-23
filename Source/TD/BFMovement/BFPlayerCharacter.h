// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PBMovement/PBPlayerCharacter.h"
#include "CoreMinimal.h"

#include "BFPlayerCharacter.generated.h"

/**
 * @brief Contains data from a wall jump line trace. Players can only wall jump
 * from in front, to the left, or to the right of the character. The jump
 * direction is the sum of the normals of the walls that were hit normalized.
 */
USTRUCT()
struct FWallJumpResult
{
    GENERATED_BODY()

    bool IsValidWallJump;
    FHitResult ForwardHit;
    FHitResult LeftHit;
    FHitResult RightHit;
    FHitResult BackwardHit;
    FVector JumpDirection;
};

/**
 * @brief The custom character the player controls. Implements the wall jumping
 * and blasting controls.
 */
UCLASS()
class TD_API ABFPlayerCharacter : public APBPlayerCharacter
{
    GENERATED_BODY()

public:
    ABFPlayerCharacter(const FObjectInitializer& ObjectInitializer);
    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#pragma region WallJumping
    /**
     * How Wall Jumping Works
     *
     * When the player wants to jump, do a line trace on the player's forward,
     * back, left, and right. If it hits a wall, the player receives an
     * additional impulse normal to that wall, a "wall jump".
     */

private:
    /**
     * @brief The collision channels that the character detects as walls that
     * they can jump off of.
     */
    UPROPERTY(EditAnywhere)
    TArray<TEnumAsByte<ECollisionChannel>> WallChannels;

    /**
     * @brief How long of a line trace to perform to the front, left, and right.
     */
    UPROPERTY(EditAnywhere)
    float WallJumpLineTraceLength = 100.0f;

    /**
     * @brief The minimum speed in the wall jump direction the player will
     * receive upon wall jumping.
     */
    UPROPERTY(EditAnywhere)
    float MinWallJumpSpeed = 750.0f;

    /**
     * @brief An array of actors that contains all of the walls that the player
     * last wall jumped on. This is to prevent players from wall jumping off of
     * the same wall in succession.
     */
    UPROPERTY()
    TArray<AActor*> LastWallsJumped;

    /**
     * @brief Checks if the player can jump in the current state. In addition to
     * the original implementation, if the player can wall jump, then this will
     * return true
     * @return Whether the player can jump.
     */
    virtual bool CanJumpInternal_Implementation() const override;

    /**
     * @brief Checks for whether the player can wall jump and does so if so.
     */
    virtual void OnJumped_Implementation() override;

    /**
     * @brief Modifies PB Movement OnJumped_Implementation method to use the
     * replicated last input vector instead of the locally stored input vector.
     * This makes it so that source movement is replicated in multiplayer.
     */
    void ModifiedPBOnJumped();

    /**
     * @brief Calculates the reflected velocity from a wall jump, or the
     * original velocity plus however much is required in the wall jump
     * direction to meet the minimum wall jump speed.
     * @param WallJump The result from the wall jump attempt.
     * @return The new reflected velocity off the wall.
     */
    FVector CalculateWallJumpReflectedVelocity(
        const FWallJumpResult& WallJump) const;

    /**
     * @brief Determines whether a player's jump is a wall jump and sets the
     * WallJump to the result if so.
     * @param WallJump Out parameter for the wall jump result.
     * @return Whether the jump is a wall jump.
     */
    bool IsWallJump(FWallJumpResult& WallJump) const;

    /**
     * @brief Line traces in the forward, left, and right direction of this
     * character to see if it hits any walls, as determined by @see
     * WallChannels.
     *
     * @param IsTest Whether to just test for if the player hits a wall.
     * @param ForwardHit The forward hit result.
     * @param BackwardHit The backward hit result.
     * @param LeftHit The left hit result.
     * @param RightHit The right hit result.
     * @return Whether the player hit a wall.
     */
    bool HasHitWall(bool IsTest = true, FHitResult* ForwardHit = nullptr,
        FHitResult* BackwardHit = nullptr, FHitResult* LeftHit = nullptr,
        FHitResult* RightHit = nullptr) const;

    /**
     * @brief Sets @see LastWallsJumped to the walls the player just jumped off
     * of.
     * @param WallJump The wall jump attempt.
     */
    void SetLastWallsJumped(const FWallJumpResult& WallJump);

    /**
     * @brief Triggered when player's movement mode has changed. Resets the last
     * walls the player jumped off of if they are no longer falling.
     * @param PrevMovementMode The previous movement mode the player was in.
     * @param PreviousCustomMode The previous custom mode the player was in.
     */
    virtual void OnMovementModeChanged(
        EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;

#pragma endregion

#pragma region Input

public:
    virtual void SetupPlayerInputComponent(
        UInputComponent* PlayerInputComponent) override;

    void SetIsMovementEnabled(bool IsEnabled);

    virtual void Jump() override;

    virtual void StopJumping() override;

    virtual void MoveForward(float Value);

    virtual void MoveRight(float Value);

    virtual void RequestCrouch();

    virtual void RequestUnCrouch();

    /**
     * @brief Sets the fast fall axis value for the player movement component.
     * @param Value The axis value for fast fall.
     */
    virtual void FastFall(float Value);

protected:
    UPROPERTY(Replicated)
    bool IsMovementEnabled = true;

#pragma endregion
};
