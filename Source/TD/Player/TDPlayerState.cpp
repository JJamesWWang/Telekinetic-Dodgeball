// Copyright 2021, James S. Wang, All rights reserved.

#include "TDPlayerState.h"

#include "Net/UnrealNetwork.h"

void ATDPlayerState::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ATDPlayerState, Team);
}

ETeamIndex ATDPlayerState::GetTeam() const
{
    return Team;
}

void ATDPlayerState::SetTeam(const ETeamIndex TeamIndex)
{
    Team = TeamIndex;
}

bool ATDPlayerState::GetIsEliminated() const
{
    return IsEliminated;
}

void ATDPlayerState::SetIsEliminated(const bool NewIsEliminated)
{
    IsEliminated = NewIsEliminated;
}

void ATDPlayerState::ResetRoundState()
{
    SetIsEliminated(false);
}
