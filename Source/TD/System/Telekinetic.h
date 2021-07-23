// Copyright 2021, James S. Wang, All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Telekinetic.generated.h"

class ATDCharacter;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UTelekinetic : public UInterface
{
    GENERATED_BODY()
};

/**
 * @brief Telekinetic actors can be pushed and pulled by players.
 * From this point forward, let "to telekinese" mean to push or pull or engage
 * in any other "telekinetic" action. This word is also a noun - a "telekinese"
 * is a push or a pull.
 */
class TD_API ITelekinetic
{
    GENERATED_BODY()

public:
    /**
     * @brief Called when the player tries to push this actor.
     * @param Player The player character that pushed this actor.
     * @param Hit The hit result from the push line trace.
     */
    virtual void OnPushed(ATDCharacter* Player, const FHitResult& Hit) = 0;

    /**
     * @brief Called when the player tries to pull this actor.
     * @param Player The player character that pulled this actor.
     * @param Hit The hit result from the pull line trace.
     */
    virtual void OnPulled(ATDCharacter* Player, const FHitResult& Hit) = 0;
};
