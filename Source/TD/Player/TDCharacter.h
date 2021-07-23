// Copyright 2021, James S. Wang, All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "AbilitySystemInterface.h"
#include "Propellable.h"
#include "TDTypes.h"
#include "TeamAssignable.h"
#include "Telekinetic.h"
#include "BFMovement/BFPlayerCharacter.h"
#include "TDCharacter.generated.h"

class ITelekinetic;
class AOrb;
class UTDCharacterASC;

/**
 * @brief A temporary adjustment for default sensitivity;
 * TODO: Find accurate conversion
 */
#define SENSITIVITY_FACTOR (1.0f / 6.6066f)

/**
 * @brief The player's character. Players may Cast orbs and Push and Pull orbs
 * and other players in order to eliminate their opponents.
 */
UCLASS()
class TD_API ATDCharacter
    : public ABFPlayerCharacter, public IAbilitySystemInterface,
      public IPropellable, public ITelekinetic, public ITeamAssignable
{
    GENERATED_BODY()

#pragma region Initialization

public:
    ATDCharacter(const FObjectInitializer& ObjectInitializer);

    /**
     * @brief Sets up the ASC for clients.
     */
    virtual void PossessedBy(AController* NewController) override;

    /**
     * @brief Sets the character's team mesh material on the client.
     */
    virtual void OnRep_PlayerState() override;

    /**
     * @brief Grants the player their default abilities and loads the player's
     * gameplay settings.
     */
    virtual void BeginPlay() override;

private:
    /**
     * @brief The player character's ability system component.
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
        meta = (AllowPrivateAccess = "true"))
    UTDCharacterASC* ASC = nullptr;

    /**
     * @brief The character's static mesh, since there are no animations.
     */
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* StaticMesh = nullptr;

    /**
     * @brief Grants ASC abilities on the server.
     */
    void InitASC(AController* NewController);

    /**
     * @brief Sets the character's team mesh material.
     */
    void SetMeshTeamMaterial();

private:
    /**
     * @brief This player's gameplay settings; used to get sensitivity.
     */
    FPlayerGameplaySettings Settings;

    /**
     * @brief Replaces Settings with the newly saved settings.
     */
    UFUNCTION()
    void OnGameplaySettingsSaved(const FPlayerGameplaySettings& NewSettings);

#pragma endregion

#pragma region GameMode Control

public:
    /**
     * @brief Destroys this character and sets the TDPlayerState to eliminated.
     */
    void Eliminate();

protected:
    /**
     * @brief The sound effect to play when a player is eliminated.
     */
    UPROPERTY(EditAnywhere, Category = "Elimination")
    USoundCue* EliminatedCue = nullptr;

private:
    /**
     * @brief Plays the eliminated sound effect.
     */
    void PlayEliminatedCue() const;

#pragma endregion

#pragma region Gameplay Ability System

public:
    /**
     * @brief Gets the ability system component for this character.
     * @return The ASC.
     */
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    /**
     * @brief Tries to find a Telekinetic actor in the direction of the player's
     * crosshair, and if successful, pushes it.
     * @return Whether the character successfully pushed a telekinetic actor.
     */
    bool Push();

    /**
     * @brief Tries to find a Telekinetic actor in the direction of the player's
     * crosshair, and if successful, pulls it.
     * @return Whether the character successfully pulled a telekinetic actor.
     */
    bool Pull();

    /**
     * @brief Spawns an orb moving in the direction of the player's crosshair.
     * Should only be called on the server and replicated down.
     * It would be better if the client could predict casting of orbs.
     * @return The spawned orb.
     */
    AOrb* CastOrb();

private:
    /**
     * @brief How long to line trace for a telekinetic action (push/pull).
     */
    UPROPERTY(EditAnywhere)
    float TelekineticTraceLength = 5000.0f;

    /**
     * @brief The class of the Orb to spawn when casting.
     */
    UPROPERTY(EditAnywhere)
    TSubclassOf<AOrb> OrbClass = nullptr;

    /**
     * @brief The offset from the character in the direction of the player's
     * crosshair to spawn the orb from.
     */
    UPROPERTY(EditAnywhere)
    float OrbSpawnOffset = 100.0f;

    /**
     * @brief Line traces for Telekinetic and gets the results.
     * @param Hit Out parameter for the hit result of the line trace.
     * @return The hit ITelekinetic actor or nullptr if no hit.
     */
    ITelekinetic* GetTelekineticTrace(FHitResult& Hit) const;

#pragma endregion

#pragma region IPropellable

public:
    /**
     * @brief Players will gain velocity in the direction of the orb
     * propelling them until it matches the linear velocity of the orb.
     * Velocity in other directions are not lost.
     */
    virtual void OnPropelled(const FVector& Velocity) override;

#pragma endregion

#pragma region ITelekinetic
    /** @see ITelekinetic */

public:
    /**
     * @brief Sets the velocity of this player to 1000 * the push direction.
     */
    virtual void OnPushed(ATDCharacter* Player, const FHitResult& Hit) override;

    /**
     * @brief Sets the velocity of this player to 1000 * the pull direction.
     */
    virtual void OnPulled(ATDCharacter* Player, const FHitResult& Hit) override;

protected:
    /**
     * @brief The speed added to this character when it's pushed or pulled.
     */
    UPROPERTY(EditAnywhere)
    float TelekineticSpeed = 1000.0f;

    UPROPERTY(EditAnywhere, Category = "Telekinesis")
    USoundCue* TelekineseHit = nullptr;

private:
    /**
     * @brief Plays the sound effect for being telekinesed.
     */
    void PlayTelekineseHitCue() const;

#pragma endregion

#pragma region ITeamAssignable
    /** See @ITeamAssignable */

public:
    virtual ETeamIndex GetTeam() const override;

protected:
    /**
     * @brief The corresponding material to use for each ETeamIndex.
     * Index 0 is ETeamIndex::None. @see ETeamIndex.
     */
    UPROPERTY(EditAnywhere, Category = "Team")
    TArray<UMaterialInstance*> MeshTeamMaterials;

#pragma endregion

#pragma region Input

private:
    virtual void SetupPlayerInputComponent(
        UInputComponent* PlayerInputComponent) override;
    void TurnRight(float Value);
    void LookUp(float Value);

#pragma endregion
};
