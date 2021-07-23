// Fill out your copyright notice in the Description page of Project Settings.

#include "BFPlayerCharacter.h"

#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "BFPlayerMovement.h"
#include "Net/UnrealNetwork.h"

static TAutoConsoleVariable<int32> CVarBunnyhop(TEXT("move.Bunnyhopping"), 0,
    TEXT("Enable normal bunnyhopping.\n"), ECVF_Default);

ABFPlayerCharacter::ABFPlayerCharacter(
    const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<UBFPlayerMovement>(
        CharacterMovementComponentName))
{
    JumpMaxCount = 2;
    // Can (undesirably) fly if JumpMaxCount > 1 and this is 0
    JumpMaxHoldTime = 0.01f;
}

void ABFPlayerCharacter::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ABFPlayerCharacter, IsMovementEnabled)
}

bool ABFPlayerCharacter::CanJumpInternal_Implementation() const
{
    FWallJumpResult WallJump;
    return Super::CanJumpInternal_Implementation() || IsWallJump(WallJump);
}

void ABFPlayerCharacter::OnJumped_Implementation()
{
    ModifiedPBOnJumped();
    FWallJumpResult WallJump;
    if (IsWallJump(WallJump))
    {
        const FVector ReflectedVelocity =
            CalculateWallJumpReflectedVelocity(WallJump);
        GetCharacterMovement()->Velocity = ReflectedVelocity;
        SetLastWallsJumped(WallJump);
        // Regain double jump on wall jump
        JumpCurrentCount = 1;
    }
}

void ABFPlayerCharacter::ModifiedPBOnJumped()
{
    LastJumpTime = GetWorld()->GetTimeSeconds();
    if (GetCharacterMovement()->bCheatFlying)
    {
        MovementPtr->NoClipVerticalMoveMode = 1;
    }
    else if (GetWorld()->GetTimeSeconds() >= LastJumpBoostTime + MaxJumpTime)
    {
        LastJumpBoostTime = GetWorld()->GetTimeSeconds();
        // Boost forward speed on jump
        FVector Facing = GetActorForwardVector();
        // Give it its backward/forward direction
        float ForwardSpeed;

        // This part is overridden to use the replicated input vector.
        // FVector Input =
        //	MovementPtr->GetLastInputVector().GetClampedToMaxSize2D(1.0f) *
        //	MovementPtr->GetMaxAcceleration();
        UBFPlayerMovement* PlayerMovementPtr =
            Cast<UBFPlayerMovement>(GetCharacterMovement());
        FVector Input =
            PlayerMovementPtr->LastControlInputVector.GetClampedToMaxSize2D(
                1.0f) *
            MovementPtr->GetMaxAcceleration();

        ForwardSpeed = Input | Facing;
        // Adjust how much the boost is
        float SpeedBoostPerc = (bIsSprinting || bIsCrouched) ? 0.1f : 0.5f;
        // How much we are boosting by
        float SpeedAddition = FMath::Abs(ForwardSpeed * SpeedBoostPerc);
        // We can only boost up to this much
        float MaxBoostedSpeed =
            GetCharacterMovement()->GetMaxSpeed() +
            GetCharacterMovement()->GetMaxSpeed() * SpeedBoostPerc;
        // Calculate new speed
        float NewSpeed =
            SpeedAddition + GetMovementComponent()->Velocity.Size2D();
        float SpeedAdditionNoClamp = SpeedAddition;

        // Scale the boost down if we are going over
        if (NewSpeed > MaxBoostedSpeed)
        {
            SpeedAddition -= NewSpeed - MaxBoostedSpeed;
        }

        if (ForwardSpeed <
            -MovementPtr->GetMaxAcceleration() * FMath::Sin(0.6981f))
        {
            // Boost backwards if we're going backwards
            SpeedAddition *= -1.0f;
            SpeedAdditionNoClamp *= -1.0f;
        }

        // Boost our velocity
        FVector JumpBoostedVel =
            GetMovementComponent()->Velocity + Facing * SpeedAddition;
        float JumpBoostedSizeSq = JumpBoostedVel.SizeSquared2D();
        if (CVarBunnyhop->GetInt() != 0)
        {
            FVector JumpBoostedUnclampVel = GetMovementComponent()->Velocity +
                                            Facing * SpeedAdditionNoClamp;
            float JumpBoostedUnclampSizeSq =
                JumpBoostedUnclampVel.SizeSquared2D();
            if (JumpBoostedUnclampSizeSq > JumpBoostedSizeSq)
            {
                JumpBoostedVel = JumpBoostedUnclampVel;
                JumpBoostedSizeSq = JumpBoostedUnclampSizeSq;
            }
        }
        if (GetMovementComponent()->Velocity.SizeSquared2D() <
            JumpBoostedSizeSq)
        {
            GetMovementComponent()->Velocity = JumpBoostedVel;
        }
    }
}

FVector ABFPlayerCharacter::CalculateWallJumpReflectedVelocity(
    const FWallJumpResult& WallJump) const
{
    const FVector PlayerVelocity = GetCharacterMovement()->Velocity;

    // u - 2 proj_v(u) gives you a reflection off of v
    FVector ReflectedVelocity =
        PlayerVelocity -
        2 * FVector::DotProduct(PlayerVelocity, WallJump.JumpDirection) *
        WallJump.JumpDirection;

    const float JumpDirectionDot = FVector::DotProduct(ReflectedVelocity,
        WallJump.JumpDirection);

    // If in the jump direction and fast enough, return without adjustment.
    if (JumpDirectionDot > 0.0f && JumpDirectionDot >= MinWallJumpSpeed)
    {
        return ReflectedVelocity;
    }

    // Add speed in the wall jump direction until it reaches the min speed.
    const FVector MinWallJumpVelocity =
        MinWallJumpSpeed * WallJump.JumpDirection;
    ReflectedVelocity += MinWallJumpVelocity - FVector::DotProduct(
        ReflectedVelocity, WallJump.JumpDirection) * WallJump.JumpDirection;
    return ReflectedVelocity;
}

bool ABFPlayerCharacter::IsWallJump(FWallJumpResult& WallJump) const
{
    // Can't wall jump if on ground
    if (JumpCurrentCountPreJump == 0)
    {
        return false;
    }

    bool Result = HasHitWall(
        false, &WallJump.ForwardHit, &WallJump.BackwardHit,
        &WallJump.LeftHit, &WallJump.RightHit);
    Result =
        Result && !LastWallsJumped.Contains(WallJump.ForwardHit.GetActor());
    Result = Result && !LastWallsJumped.Contains(
                 WallJump.BackwardHit.GetActor());
    Result = Result && !LastWallsJumped.Contains(WallJump.LeftHit.GetActor());
    Result = Result && !LastWallsJumped.Contains(WallJump.RightHit.GetActor());
    WallJump.JumpDirection =
        (WallJump.ForwardHit.ImpactNormal + WallJump.BackwardHit.ImpactNormal +
         WallJump.LeftHit.ImpactNormal + WallJump.RightHit.ImpactNormal)
        .GetSafeNormal();
    WallJump.IsValidWallJump = Result;
    return Result;
}

bool ABFPlayerCharacter::HasHitWall(bool IsTest, FHitResult* ForwardHit,
    FHitResult* BackwardHit, FHitResult* LeftHit, FHitResult* RightHit) const
{
    if (WallChannels.Num() == 0)
    {
        return false;
    }

    const UWorld* World = GetWorld();
    const FVector Location = GetActorLocation();
    const FVector ForwardWall =
        Location + GetActorForwardVector() * WallJumpLineTraceLength;
    const FVector BackwardWall =
        Location - GetActorForwardVector() * WallJumpLineTraceLength;
    const FVector LeftWall =
        Location - GetActorRightVector() * WallJumpLineTraceLength;
    const FVector RightWall =
        Location + GetActorRightVector() * WallJumpLineTraceLength;

    FCollisionObjectQueryParams QueryParams;
    for (ECollisionChannel Channel : WallChannels)
    {
        QueryParams.AddObjectTypesToQuery(Channel);
    }
    QueryParams.DoVerify();

    const bool HitForwardWall =
        IsTest
            ? World->LineTraceTestByObjectType(
                Location, ForwardWall, QueryParams)
            : World->LineTraceSingleByObjectType(
                *ForwardHit, Location, ForwardWall, QueryParams);
    const bool HitBackwardWall =
        IsTest
            ? World->LineTraceTestByObjectType(
                Location, BackwardWall, QueryParams)
            : World->LineTraceSingleByObjectType(
                *BackwardHit, Location, BackwardWall, QueryParams);
    const bool HitLeftWall =
        IsTest
            ? World->LineTraceTestByObjectType(Location, LeftWall, QueryParams)
            : World->LineTraceSingleByObjectType(
                *LeftHit, Location, LeftWall, QueryParams);
    const bool HitRightWall =
        IsTest
            ? World->LineTraceTestByObjectType(Location, RightWall, QueryParams)
            : World->LineTraceSingleByObjectType(
                *RightHit, Location, RightWall, QueryParams);

    return HitForwardWall || HitBackwardWall || HitLeftWall || HitRightWall;
}

void ABFPlayerCharacter::SetLastWallsJumped(const FWallJumpResult& WallJump)
{
    LastWallsJumped.Reset();
    if (WallJump.ForwardHit.IsValidBlockingHit())
    {
        LastWallsJumped.Emplace(WallJump.ForwardHit.GetActor());
    }
    if (WallJump.BackwardHit.IsValidBlockingHit())
    {
        LastWallsJumped.Emplace(WallJump.BackwardHit.GetActor());
    }
    if (WallJump.LeftHit.IsValidBlockingHit())
    {
        LastWallsJumped.Emplace(WallJump.LeftHit.GetActor());
    }
    if (WallJump.RightHit.IsValidBlockingHit())
    {
        LastWallsJumped.Emplace(WallJump.RightHit.GetActor());
    }
}

void ABFPlayerCharacter::OnMovementModeChanged(
    EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
    Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
    if (!GetCharacterMovement()->IsFalling())
    {
        LastWallsJumped.Reset();
    }
}

#pragma region Input

void ABFPlayerCharacter::SetupPlayerInputComponent(
    UInputComponent* PlayerInputComponent)
{
    PlayerInputComponent->BindAction(
        "Jump", IE_Pressed, this, &ABFPlayerCharacter::Jump);
    PlayerInputComponent->BindAction(
        "Jump", IE_Released, this, &ABFPlayerCharacter::StopJumping);
    PlayerInputComponent->BindAction(
        "Crouch", IE_Pressed, this, &ABFPlayerCharacter::RequestCrouch);
    PlayerInputComponent->BindAction(
        "Crouch", IE_Released, this, &ABFPlayerCharacter::RequestUnCrouch);

    PlayerInputComponent->BindAxis(
        "MoveForward", this, &ABFPlayerCharacter::MoveForward);
    PlayerInputComponent->BindAxis(
        "MoveRight", this, &ABFPlayerCharacter::MoveRight);
    PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
    PlayerInputComponent->BindAxis(
        "LookUp", this, &APawn::AddControllerPitchInput);
    PlayerInputComponent->BindAxis(
        "FastFall", this, &ABFPlayerCharacter::FastFall);
}

void ABFPlayerCharacter::SetIsMovementEnabled(bool IsEnabled)
{
    IsMovementEnabled = IsEnabled;
}

void ABFPlayerCharacter::Jump()
{
    if (IsMovementEnabled)
    {
        Super::Jump();
    }
}

void ABFPlayerCharacter::StopJumping()
{
    if (IsMovementEnabled)
    {
        Super::StopJumping();
    }
}

void ABFPlayerCharacter::MoveForward(float Value)
{
    if (Value != 0.0f && IsMovementEnabled)
    {
        AddMovementInput(GetActorForwardVector(), Value);
    }
}

void ABFPlayerCharacter::MoveRight(float Value)
{
    if (Value != 0.0f && IsMovementEnabled)
    {
        AddMovementInput(GetActorRightVector(), Value);
    }
}

void ABFPlayerCharacter::RequestCrouch()
{
    if (IsMovementEnabled)
    {
        Super::Crouch();
    }
}

void ABFPlayerCharacter::RequestUnCrouch()
{
    if (IsMovementEnabled)
    {
        Super::UnCrouch();
    }
}

void ABFPlayerCharacter::FastFall(float Value)
{
    UBFPlayerMovement* Movement = Cast<UBFPlayerMovement>(
        GetCharacterMovement());
    if (IsMovementEnabled)
    {
        Movement->FastFall(Value);
    }
}

#pragma endregion
