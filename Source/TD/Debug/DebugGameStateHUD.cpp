// Copyright 2021, James S. Wang, All rights reserved.

#include "DebugGameStateHUD.h"

#include "GameConfiguration.h"
#include "GameState/RoundState.h"
#include "GameState/TDGameState.h"
#include "GameState/Components/ScoreState.h"
#include "GameState/Components/TeamState.h"

void ADebugGameStateHUD::DrawHUD()
{
    Super::DrawHUD();
    UWorld* const World = GetWorld();
    if (World == nullptr)
    {
        LogInvalidPointer("ADebugGameStateHUD", "DrawHUD", "World");
        return;
    }

    ATDGameState* GameState = World->GetGameState<ATDGameState>();
    if (GameState == nullptr)
    {
        LogInvalidPointer("ADebugGameStateHUD", "DrawHUD", "GameState");
        return;
    }

    HorizontalOffset = 300.0f;
    X = 50.0f;
    LineHeight = 16.0f;
    AddText(TEXT("Match State"), FText::FromName(GameState->MatchState));
    X = 100.0f;
    AddBool(TEXT("ShouldMatchTimerTick"),
        GameState->RoundStateComponent->ShouldMatchTimerTick);
    LineHeight = 32.0f;
    AddFloat(TEXT("SecondsUntilMatchEnds"), GameState->SecondsUntilMatchEnds);

    X = 50.0f;
    LineHeight = 16.0f;
    AddText(TEXT("Round State"),
        FText::FromName(GameState->RoundStateComponent->RoundState));
    X = 100.0f;
    AddBool(TEXT("ShouldStartRoundTimerTick"),
        GameState->RoundStateComponent->ShouldStartRoundTimerTick);
    AddFloat(TEXT("SecondsUntilRoundStarts"),
        GameState->RoundStateComponent->SecondsUntilRoundStarts);
    AddBool(TEXT("ShouldOvertimeTimerTick"),
        GameState->RoundStateComponent->ShouldOvertimeTimerTick);
    AddFloat(TEXT("OvertimeSecondsPassed"),
        GameState->RoundStateComponent->OvertimeSecondsPassed);
    AddBool(TEXT("ShouldEndOfRoundTimerTick"),
        GameState->RoundStateComponent->ShouldEndOfRoundTimerTick);
    LineHeight = 32.0f;
    AddFloat(TEXT("SecondsUntilNextRoundStarts"),
        GameState->RoundStateComponent->SecondsUntilNextRoundStarts);

    uint8 Index = 0;
    for (uint8 Score : GameState->ScoreStateComponent->TeamScores)
    {
        X = 50.0f;
        LineHeight = 16.0f;
        UEnum* TeamEnum = StaticEnum<ETeamIndex>();
        AddText(TEXT("Team"),
            FText::FromString(TeamEnum->GetNameStringByValue(Index)));
        X = 100.0f;
        LineHeight = 32.0f;
        AddInt(TEXT("Score"), Score);
        Index += 1;
    }
}
