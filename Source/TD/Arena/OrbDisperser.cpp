// Copyright 2021, James S. Wang, All rights reserved.

#include "Arena/OrbDisperser.h"

#include "GameConfiguration.h"
#include "Components/BoxComponent.h"
#include "GameState/TDGameState.h"
#include "Orb/Orb.h"
#include "Sound/SoundCue.h"

#pragma region Initialization

AOrbDisperser::AOrbDisperser()
{
    Collider = CreateDefaultSubobject<UBoxComponent>(TEXT("Collider"));
    SetRootComponent(Collider);

    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(
        TEXT("Static Mesh"));
    StaticMesh->SetupAttachment(Collider);
}

void AOrbDisperser::BeginPlay()
{
    Super::BeginPlay();
    Collider->OnComponentBeginOverlap.AddDynamic(this,
        &AOrbDisperser::OnOverlapped);
}

#pragma endregion

#pragma region Dispersing

void AOrbDisperser::OnOverlapped(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    AOrb* Orb = Cast<AOrb>(OtherActor);
    if (Orb != nullptr)
    {
        PlayDispersedCue(Orb->GetActorLocation());
        Orb->Destroy();
    }
}

void AOrbDisperser::PlayDispersedCue(const FVector& Location) const
{
    if (DispersedCue == nullptr)
    {
        LogInvalidPointer("AOrbDisperser", "PlayDispersedCue", "DispersedCue");
        return;
    }

    ATDGameState* GameState = GetWorld()->GetGameState<ATDGameState>();
    if (GameState == nullptr)
    {
        LogInvalidPointer("AOrbDisperser", "PlayDispersedCue", "GameState");
        return;
    }

    GameState->PlayReplicatedCueAtLocation(DispersedCue, Location);
}

#pragma endregion
