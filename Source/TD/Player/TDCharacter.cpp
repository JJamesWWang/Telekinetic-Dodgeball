// Copyright 2021, James S. Wang, All rights reserved.

#include "TDCharacter.h"

#include "DrawDebugHelpers.h"
#include "TDCharacterASC.h"
#include "TDTypes.h"
#include "GameConfiguration.h"
#include "TDPlayerState.h"
#include "Telekinetic.h"
#include "Components/CapsuleComponent.h"
#include "TDGameInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameState/TDGameState.h"
#include "Orb/Orb.h"
#include "Sound/SoundCue.h"

#pragma region Initialization

ATDCharacter::ATDCharacter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    ASC = CreateDefaultSubobject<UTDCharacterASC>(TEXT("ASC"));
    ASC->SetIsReplicated(true);
    ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(
        TEXT("StaticMesh"));
    StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    StaticMesh->SetupAttachment(GetCapsuleComponent());

    GetCapsuleComponent()->SetCollisionProfileName(TEXT("Player"));
}

void ATDCharacter::InitASC(AController* NewController)
{
    if (ASC == nullptr)
    {
        LogInvalidPointer("ATDCharacter", "PossessedBy", "ASC");
        return;
    }

    ASC->InitAbilityActorInfo(this, this);
    SetOwner(NewController);
}

void ATDCharacter::SetMeshTeamMaterial()
{
    uint8 Team = Int(GetTeam());
    if (MeshTeamMaterials.Num() <= Team)
    {
        LogInvalidPointer("ATDCharacter", "PostActorCreated",
            "MeshTeamMaterials",
            "Did you add a material for each team? Remember that 0 is None");
        return;
    }

    StaticMesh->SetMaterial(0, MeshTeamMaterials[Team]);
}

void ATDCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    InitASC(NewController);
    SetMeshTeamMaterial();
}

void ATDCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    SetMeshTeamMaterial();
}

void ATDCharacter::BeginPlay()
{
    Super::BeginPlay();
    if (ASC == nullptr)
    {
        LogInvalidPointer("ATDCharacter", "BeginPlay", "ASC");
        return;
    }
    ASC->GrantDefaultAbilities();

    UTDGameInstance* GameInstance = Cast<UTDGameInstance>(GetGameInstance());
    if (GameInstance == nullptr)
    {
        LogInvalidPointer("ATDCharacter", "BeginPlay", "GameInstance");
        return;
    }
    Settings = GameInstance->LoadPlayerGameplaySettings();
    GameInstance->OnPlayerGameplaySettingsSaved.AddUObject(this,
        &ATDCharacter::OnGameplaySettingsSaved);
}

void ATDCharacter::OnGameplaySettingsSaved(
    const FPlayerGameplaySettings& NewSettings)
{
    Settings = NewSettings;
}

void ATDCharacter::Eliminate()
{
    if (IsActorBeingDestroyed())
    {
        return;
    }

    ATDPlayerState* TDPlayerState = GetPlayerState<ATDPlayerState>();
    TDPlayerState->SetIsEliminated(true);
    PlayEliminatedCue();

    Destroy();
}

void ATDCharacter::PlayEliminatedCue() const
{
    if (EliminatedCue == nullptr)
    {
        LogInvalidPointer("ATDCharacter", "PlayEliminatedCue", "EliminatedCue");
        return;
    }

    ATDGameState* GameState = GetWorld()->GetGameState<ATDGameState>();
    if (GameState == nullptr)
    {
        LogInvalidPointer("ATDCharacter", "PlayEliminatedCue", "GameState");
        return;
    }

    GameState->PlayReplicatedCueAtLocation(EliminatedCue,
        GetActorLocation());
}

#pragma endregion

#pragma region Gameplay Ability System

UAbilitySystemComponent* ATDCharacter::GetAbilitySystemComponent() const
{
    return ASC;
}

bool ATDCharacter::Push()
{
    FHitResult Hit;
    ITelekinetic* ATelekinetic = GetTelekineticTrace(Hit);
    if (ATelekinetic != nullptr)
    {
        ATelekinetic->OnPushed(this, Hit);
    }
    return ATelekinetic != nullptr;
}

bool ATDCharacter::Pull()
{
    FHitResult Hit;
    ITelekinetic* ATelekinetic = GetTelekineticTrace(Hit);
    if (ATelekinetic != nullptr)
    {
        ATelekinetic->OnPulled(this, Hit);
    }
    return ATelekinetic != nullptr;
}

ITelekinetic* ATDCharacter::GetTelekineticTrace(FHitResult& Hit) const
{
    UWorld* World = GetWorld();
    if (World == nullptr || GetController() == nullptr)
    {
        LogInvalidPointer("ATDCharacter", "TraceTelekinetic",
            "World or Controller");
        return nullptr;
    }

    FVector Start;
    FRotator Rotation;
    GetController()->GetPlayerViewPoint(Start, Rotation);
    FVector End = Start + Rotation.Vector() * TelekineticTraceLength;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    World->LineTraceSingleByChannel(Hit, Start, End,
        ECC_Telekinetic, QueryParams);
    return Cast<ITelekinetic>(Hit.GetActor());
}

AOrb* ATDCharacter::CastOrb()
{
    FVector Location;
    FRotator Rotation;
    GetController()->GetPlayerViewPoint(Location, Rotation);
    UWorld* World = GetWorld();
    if (World == nullptr || OrbClass == nullptr)
    {
        LogInvalidPointer("ATDCharacter", "CastOrb", "World or OrbClass",
            "Did you set the Orb class that this character casts?");
        return nullptr;
    }

    FActorSpawnParameters SpawnParameters;
    SpawnParameters.Owner = this;
    AOrb* Orb = World->SpawnActor<AOrb>(OrbClass,
        Location + Rotation.Vector() * OrbSpawnOffset, Rotation,
        SpawnParameters);
    if (Orb != nullptr)
    {
        Orb->InitPlayerVForce(GetVelocity());
    }
    return Orb;
}

#pragma endregion

#pragma region IPropellable

void ATDCharacter::OnPropelled(const FVector& Velocity)
{
    GetMovementComponent()->Velocity = Velocity;
}

#pragma endregion

#pragma region ITelekinetic

void ATDCharacter::OnPushed(ATDCharacter* Player, const FHitResult& Hit)
{
    const FVector Direction = (Hit.TraceEnd - Hit.TraceStart).GetSafeNormal();
    GetCharacterMovement()->Velocity += TelekineticSpeed * Direction;
    if (HasAuthority())
    {
        PlayTelekineseHitCue();
    }
}

void ATDCharacter::OnPulled(ATDCharacter* Player, const FHitResult& Hit)
{
    const FVector Direction = (Hit.TraceStart - Hit.TraceEnd).GetSafeNormal();
    GetCharacterMovement()->Velocity += TelekineticSpeed * Direction;
    if (HasAuthority())
    {
        PlayTelekineseHitCue();
    }
}

void ATDCharacter::PlayTelekineseHitCue() const
{
    if (TelekineseHit == nullptr)
    {
        LogInvalidPointer("ATDCharacter", "PlayTelekineseHitCue",
            "TelekineseHit");
        return;
    }

    ATDGameState* GameState = GetWorld()->GetGameState<ATDGameState>();
    if (GameState == nullptr)
    {
        LogInvalidPointer("ATDCharacter", "PlayTelekineseHitCue", "GameState");
        return;
    }

    GameState->PlayReplicatedCueAtLocation(TelekineseHit, GetActorLocation());
}

#pragma endregion

#pragma region ITeamAssignable

ETeamIndex ATDCharacter::GetTeam() const
{
    ATDPlayerState* TDPlayerState = GetPlayerState<ATDPlayerState>();
    if (TDPlayerState == nullptr)
    {
        LogInvalidPointer("ATDCharacter", "GetTeam", "PlayerState");
        return ETeamIndex::None;
    }
    return TDPlayerState->GetTeam();
}

#pragma endregion

#pragma region Input

void ATDCharacter::SetupPlayerInputComponent(
    UInputComponent* PlayerInputComponent)
{
    if (PlayerInputComponent == nullptr)
    {
        LogInvalidPointer("ATDCharacter", "SetupPlayerInputComponent",
            "PlayerInputComponent");
        return;
    }

    PlayerInputComponent->BindAction("Jump", IE_Pressed, this,
        &ABFPlayerCharacter::Jump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this,
        &ABFPlayerCharacter::StopJumping);
    PlayerInputComponent->BindAction("Slide", IE_Pressed, this,
        &ABFPlayerCharacter::RequestCrouch);
    PlayerInputComponent->BindAction("Slide", IE_Released, this,
        &ABFPlayerCharacter::RequestUnCrouch);

    PlayerInputComponent->BindAxis("MoveForward", this,
        &ABFPlayerCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this,
        &ABFPlayerCharacter::MoveRight);
    PlayerInputComponent->BindAxis("TurnRight", this,
        &ATDCharacter::TurnRight);
    PlayerInputComponent->BindAxis("LookUp", this,
        &ATDCharacter::LookUp);
    PlayerInputComponent->BindAxis("FastFall", this,
        &ABFPlayerCharacter::FastFall);

    if (ASC == nullptr)
    {
        LogInvalidPointer("ATDCharacter", "SetupPlayerInputComponent", "ASC");
        return;
    }

    ASC->BindAbilityActivationToInputComponent(PlayerInputComponent,
        FGameplayAbilityInputBinds(FString("Confirm"),
            FString("Cancel"), FString("ETDAbilityInputID"),
            static_cast<int32>(ETDAbilityInputID::Confirm),
            static_cast<int32>(ETDAbilityInputID::Cancel)));
}

void ATDCharacter::TurnRight(float Value)
{
    if (Value != 0.0f)
    {
        AddControllerYawInput(
            Value * Settings.Sensitivity * SENSITIVITY_FACTOR);
    }
}

void ATDCharacter::LookUp(float Value)
{
    if (Value != 0.0f)
    {
        AddControllerPitchInput(
            Value * Settings.Sensitivity * SENSITIVITY_FACTOR);
    }
}

#pragma endregion
