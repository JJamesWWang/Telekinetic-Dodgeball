// Copyright 2021, James S. Wang, All rights reserved.

#include "GameRules.h"

#include "GameFramework/GameMode.h"
#include "GameModes/RoundStateMachine.h"
#include "GameState/TDGameState.h"
#include "Net/UnrealNetwork.h"
#include "Orb/Orb.h"
#include "Player/TDCharacter.h"

#pragma region Initialization

AGameRules::AGameRules()
{
    bReplicates = true;
    bAlwaysRelevant = true;
    SetReplicatingMovement(false);

    // Might not be necessary
    bNetLoadOnClient = false;
}

void AGameRules::SetGameState(const ATDGameState* GameState)
{
    TDGameState = GameState;
}

void AGameRules::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AGameRules, TDGameState);
}

#pragma endregion

bool AGameRules::CanCastOrb() const
{
    return IsInPlay();
}

bool AGameRules::CanPull() const
{
    return IsInPlay();
}

bool AGameRules::CanPush() const
{
    return IsInPlay();
}

bool AGameRules::CanTelekineseOrb(const ATDCharacter* Player,
    const AOrb* Orb) const
{
    if (Player == nullptr || Orb == nullptr)
    {
        return false;
    }

    return IsInPlay() && Player->GetTeam() == Orb->GetTeam();
}

bool AGameRules::IsInPlay() const
{
    if (TDGameState == nullptr)
    {
        return false;
    }

    return TDGameState->GetMatchState() == MatchState::InProgress &&
           (TDGameState->GetRoundState() == RoundState::RoundInProgress ||
            TDGameState->GetRoundState() == RoundState::RoundInOvertime);
}
