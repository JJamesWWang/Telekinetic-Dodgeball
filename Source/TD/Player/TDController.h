// Copyright 2021, James S. Wang, All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TDController.generated.h"

enum class ETeamIndex : uint8;
class ATDGameMode;
class UTDWidget;

/**
 * @brief Player controller for the player character.
 */
UCLASS()
class TD_API ATDController : public APlayerController
{
    GENERATED_BODY()

#pragma region Initialization

public:
    ATDController();

    /**
     * @brief Initializes the player character's ASC on the server.
     */
    virtual void AcknowledgePossession(APawn* P) override;

    /**
     * @brief Shows/Hides the escape menu.
     */
    UFUNCTION(BlueprintCallable, Category = "Player UI")
    void ToggleEscapeMenu();

protected:
    /**
     * @brief The escape menu widget blueprint.
     */
    UPROPERTY(EditDefaultsOnly, Category = "Player UI")
    TSubclassOf<UTDWidget> EscapeMenuClass = nullptr;

    /**
     * @brief Reference to the escape menu widget so we can show/hide it.
     */
    UPROPERTY(BlueprintReadOnly, Category = "Player UI")
    UTDWidget* EscapeMenu = nullptr;

    /**
     * @brief Binds the escape menu button.
     */
    virtual void SetupInputComponent() override;

    /**
     * @brief Creates the escape menu widget on the client.
     */
    virtual void BeginPlay() override;

    /**
     * @brief Gets the TDGameMode or nullptr if not the server.
     */
    ATDGameMode* GetGameMode() const;

#pragma endregion

#pragma region RPCs

public:
    UFUNCTION(Reliable, Client, Category = "Server Notifications")
    void Client_NotifyEliminated();
    void Client_NotifyEliminated_Implementation();

protected:
    /**
     * @brief RPC for a player requesting to join a team.
     * @param Team The ETeamIndex of the team to join.
     */
    UFUNCTION(BlueprintCallable, Reliable, Server, Category = "Player Requests")
    void Server_JoinTeam(const ETeamIndex Team);
    void Server_JoinTeam_Implementation(const ETeamIndex Team);

#pragma endregion
};
