// Copyright 2021, James S. Wang, All rights reserved.

#include "Orb.h"

#include "EngineUtils.h"
#include "GameConfiguration.h"
#include "OrbMovement.h"
#include "Components/SphereComponent.h"
#include "GameModes/TDGameMode.h"
#include "GameRules/GameRules.h"
#include "GameState/TDGameState.h"
#include "Net/UnrealNetwork.h"
#include "Player/TDCharacter.h"
#include "Sound/SoundCue.h"

#pragma region Initialization

AOrb::AOrb()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    SetReplicateMovement(true);

    CreateCollider();
    SetRootComponent(Collider);
    CreateStaticMesh();
    CreateMovement();
}

void AOrb::CreateCollider()
{
    Collider = CreateDefaultSubobject<USphereComponent>(TEXT("Collider"));
    Collider->BodyInstance.SetCollisionProfileNameDeferred("Orb");
    Collider->CanCharacterStepUpOn = ECB_Yes;
    Collider->SetCollisionProfileName(TEXT("Orb"));
}

void AOrb::CreateStaticMesh()
{
    InternalMesh =
        CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InternalMesh"));
    InternalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    InternalMesh->SetupAttachment(Collider);

    ExternalMesh = CreateDefaultSubobject<UStaticMeshComponent>(
        TEXT("ExternalMesh"));
    ExternalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ExternalMesh->SetupAttachment(Collider);
}

void AOrb::CreateMovement()
{
    Movement = CreateDefaultSubobject<UOrbMovement>(TEXT("Orb Movement"));
    Movement->UpdatedComponent = Collider;
}

void AOrb::PostActorCreated()
{
    if (HasAuthority())
    {
        SetInitialTeam();
    }
    Super::PostActorCreated();
}

void AOrb::InitPlayerVForce(const FVector& Velocity) const
{
    Movement->SetPlayerVForce(Velocity);
}

void AOrb::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AOrb, Team);
}

void AOrb::OnRep_Team()
{
    OnTeamSet();
}

void AOrb::SetInitialTeam()
{
    ATDCharacter* Caster = Cast<ATDCharacter>(GetOwner());
    if (Caster == nullptr)
    {
        LogInvalidPointer("AOrb", "SetInitialTeam", "Caster", "",
            GetLocalRole());
        return;
    }
    Team = Caster->GetTeam();
    OnTeamSet();
}

void AOrb::OnTeamSet()
{
    if (InternalMeshTeamMaterials.Num() <= Int(Team) ||
        ExternalMeshTeamMaterials.Num() <= Int(Team))
    {
        LogInvalidPointer("AOrb", "SetInitialTeam",
            "InternalMeshTeamMaterials or ExternalMeshTeamMaterials",
            "Did you add a material for each team? Remember that 0 is None");
        return;
    }

    InternalMesh->SetMaterial(0, InternalMeshTeamMaterials[Int(Team)]);
    ExternalMesh->SetMaterial(0, ExternalMeshTeamMaterials[Int(Team)]);
}

void AOrb::BeginPlay()
{
    Super::BeginPlay();
    Movement->OnProjectileImpact.AddUObject(this, &AOrb::OnOrbImpact);
    Movement->OnProjectileBounce.AddDynamic(this, &AOrb::OnOrbBounce);
}

#pragma endregion

#pragma region Collision

void AOrb::OnOrbImpact(const FHitResult& Hit, const FVector& OrbVelocity)
{
    if (!HasAuthority())
    {
        return;
    }

    UWorld* const World = GetWorld();
    if (World == nullptr)
    {
        LogInvalidPointer("AOrb", "OnOrbImpact", "World");
        return;
    }

    ATDGameMode* TDGameMode = Cast<ATDGameMode>(World->GetAuthGameMode());
    if (TDGameMode == nullptr)
    {
        LogInvalidPointer("AOrb", "OnOrbImpact", "TDGameMode");
        return;
    }

    TDGameMode->HandleOrbImpact(this, Hit);
}

void AOrb::OnOrbBounce(const FHitResult& Hit, const FVector& OldVelocity)
{
    AOrb* OtherOrb = Cast<AOrb>(Hit.GetActor());
    if (OtherOrb == nullptr)
    {
        PlayBounceCue();
    }
}

void AOrb::PlayBounceCue() const
{
    if (!HasAuthority())
    {
        return;
    }

    if (BounceCue == nullptr)
    {
        LogInvalidPointer("AOrb", "PlayBouncecue", "BounceCue");
        return;
    }

    ATDGameState* GameState = GetWorld()->GetGameState<ATDGameState>();
    if (GameState == nullptr)
    {
        LogInvalidPointer("AOrb", "PlayBounceCue", "GameState");
        return;
    }

    GameState->PlayReplicatedCueAtLocation(BounceCue, GetActorLocation());
}

#pragma endregion

#pragma region ITelekinetic

void AOrb::OnPushed(ATDCharacter* Player, const FHitResult& Hit)
{
    if (CanBeTelekinesedBy(Player))
    {
        Movement->OnPushed(Player, Hit);
        if (HasAuthority())
        {
            PlayTelekineseHitCue();
        }
    }
}

void AOrb::OnPulled(ATDCharacter* Player, const FHitResult& Hit)
{
    if (CanBeTelekinesedBy(Player))
    {
        Movement->OnPulled(Player, Hit);
        if (HasAuthority())
        {
            PlayTelekineseHitCue();
        }
    }
}

bool AOrb::CanBeTelekinesedBy(ATDCharacter* Player) const
{
    TActorIterator<AGameRules> It(GetWorld());
    AGameRules* GameRules = *It;
    if (GameRules == nullptr)
    {
        LogInvalidPointer("AOrb", "CanBeTelekinesedBy", "GameRules");
        return false;
    }
    return GameRules->CanTelekineseOrb(Player, this);
}

void AOrb::PlayTelekineseHitCue() const
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

ETeamIndex AOrb::GetTeam() const
{
    return Team;
}

void AOrb::SetTeam(const ETeamIndex TeamIndex)
{
    Team = TeamIndex;
    OnTeamSet();
}

#pragma endregion
