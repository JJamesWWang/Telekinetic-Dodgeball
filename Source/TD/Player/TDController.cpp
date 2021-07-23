// Copyright 2021, James S. Wang, All rights reserved.

#include "Player/TDController.h"

#include "AbilitySystemComponent.h"
#include "EngineUtils.h"
#include "GameConfiguration.h"
#include "TDCharacter.h"
#include "TDPlayerState.h"
#include "Blueprint/UserWidget.h"
#include "GameModes/TDGameMode.h"
#include "UI/TDWidget.h"

#pragma region Initialization

ATDController::ATDController()
{
    //bAutoManageActiveCameraTarget = false;
}

void ATDController::AcknowledgePossession(APawn* P)
{
    Super::AcknowledgePossession(P);

    ATDCharacter* Char = Cast<ATDCharacter>(P);
    if (Char != nullptr)
    {
        Char->GetAbilitySystemComponent()->InitAbilityActorInfo(Char, Char);
    }
}

void ATDController::SetupInputComponent()
{
    Super::SetupInputComponent();
    InputComponent->BindAction("EscapeMenu", IE_Pressed, this,
        &ATDController::ToggleEscapeMenu);
}

void ATDController::BeginPlay()
{
    if (IsLocalPlayerController())
    {
        if (EscapeMenuClass == nullptr)
        {
            LogInvalidPointer("ATDController", "BeginPlay", "EscapeMenuClass",
                "Did you set the Escape Menu subclass in the player controller?");
            return;
        }

        EscapeMenu = CreateWidget<UTDWidget>(this, EscapeMenuClass,
            TEXT("Escape Menu"));
    }

    Super::BeginPlay();
}

ATDGameMode* ATDController::GetGameMode() const
{
    UWorld* const World = GetWorld();
    if (World == nullptr)
    {
        LogInvalidPointer("ATDController", "RequestJoinBlueTeam", "World");
        return nullptr;
    }

    return Cast<ATDGameMode>(World->GetAuthGameMode());
}

void ATDController::ToggleEscapeMenu()
{
    if (EscapeMenu == nullptr)
    {
        LogInvalidPointer("ATDController", "ToggleEscapeMenu", "EscapeMenu");
        return;
    }

    if (EscapeMenu->IsInViewport())
    {
        EscapeMenu->RemoveFromViewport();
        SetInputMode(FInputModeGameOnly());
        SetShowMouseCursor(false);
    }
    else
    {
        EscapeMenu->AddToViewport();
        SetInputMode(FInputModeGameAndUI());
        SetShowMouseCursor(true);
    }
}

#pragma endregion

#pragma region RPCs

void ATDController::Client_NotifyEliminated_Implementation()
{
    ATDPlayerState* TDPlayerState = GetPlayerState<ATDPlayerState>();
    if (TDPlayerState == nullptr)
    {
        LogInvalidPointer("ATDController", "Client_NotifyEliminated",
            "TDPlayerState");
        return;
    }

    //for (TActorIterator<ATDCharacter> It(GetWorld()); It; ++It)
    //{
    //    ATDCharacter* OtherPlayer = *It;
    //    ATDPlayerState* OtherPlayerState = OtherPlayer->GetPlayerState<
    //        ATDPlayerState>();
    //    if (OtherPlayerState->GetTeam() == TDPlayerState->GetTeam() &&
    //        !OtherPlayerState->GetIsEliminated())
    //    {
    //        PlayerCameraManager->SetViewTarget(OtherPlayer);
    //    }
    //}
}

void ATDController::Server_JoinTeam_Implementation(const ETeamIndex Team)
{
    ATDGameMode* GameMode = GetGameMode();
    if (GameMode == nullptr)
    {
        LogInvalidPointer("ATDController", "RequestJoinBlueTeam",
            "GameMode");
        return;
    }

    GameMode->SwitchPlayerToTeam(this, Team);
}

#pragma endregion
