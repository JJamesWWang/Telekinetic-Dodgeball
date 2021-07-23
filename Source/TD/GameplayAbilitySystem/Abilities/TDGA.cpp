// Copyright 2021, James S. Wang, All rights reserved.

#include "TDGA.h"

#include "EngineUtils.h"
#include "GameConfiguration.h"
#include "GameModes/TDGameMode.h"
#include "GameState/TDGameState.h"
#include "GameRules/GameRules.h"

int32 UTDGA::GetAbilityInputID() const
{
    return static_cast<int32>(AbilityInputID);
}

FString UTDGA::GetAbilityName() const
{
    return AbilityName;
}

ATDGameMode* UTDGA::GetGameMode() const
{
    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        LogInvalidPointer("UCastGA", "ResetPlayerOrb", "World");
        return nullptr;
    }

    return Cast<ATDGameMode>(World->GetAuthGameMode());
}

ATDGameState* UTDGA::GetGameState() const
{
    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        LogInvalidPointer("UCastGA", "ResetPlayerOrb", "World");
        return nullptr;
    }

    return Cast<ATDGameState>(World->GetGameState());
}

AGameRules* UTDGA::GetGameRules() const
{
    TActorIterator<AGameRules> GameRulesPtr(GetWorld());
    if (!GameRulesPtr)
    {
        LogInvalidPointer("UTDGA", "GetGameRules", "GameRulesPtr");
        return nullptr;
    }

    return *GameRulesPtr;
}
