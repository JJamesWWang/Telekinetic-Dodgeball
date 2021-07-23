// Copyright 2021, James S. Wang, All rights reserved.

#include "LSGameMode.h"

#include "GameConfiguration.h"

void ALSGameMode::InitGame(
    const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);
    if (!ErrorMessage.IsEmpty())
    {
        return;
    }

    if (!Settings.IsInitialized)
    {
        Settings.ParseOptions(Options);
        UE_LOG(LobbySystem, Log, TEXT("Host Lobby Settings set: %s"),
            *Settings.ToOptionsString());
    }
}

bool ALSGameMode::ReadyToStartMatch_Implementation()
{
    // If bDelayed Start is set, wait for a manual match start
    if (bDelayedStart)
    {
        return false;
    }

    if (GetMatchState() == MatchState::WaitingToStart)
    {
        if (NumPlayers + NumBots >= Settings.MinPlayers)
        {
            if (StartMatchDelay <= 0.0f)
            {
                return true;
            }

            if (!StartMatchDelayTimerHandle.IsValid())
            {
                FTimerDelegate Callback;
                Callback.BindLambda([&]()
                {
                    HasStartMatchDelayPassed = true;
                });
                GetWorldTimerManager().SetTimer(StartMatchDelayTimerHandle,
                    Callback, StartMatchDelay, false);
            }
            return HasStartMatchDelayPassed;
        }
    }
    return false;
}

void ALSGameMode::StartMatch()
{
    Super::StartMatch();
    StartMatchDelayTimerHandle.Invalidate();
    HasStartMatchDelayPassed = false;
}
