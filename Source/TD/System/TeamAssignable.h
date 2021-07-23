// Copyright 2021, James S. Wang, All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "TDTypes.h"
#include "UObject/Interface.h"
#include "TeamAssignable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UTeamAssignable : public UInterface
{
    GENERATED_BODY()
};

/**
 * @brief The actor that implements this interface can be assigned to a team.
 */
class TD_API ITeamAssignable
{
    GENERATED_BODY()

public:
    /**
     * @brief Gets the ETeamIndex of the team the actor belongs to.
     * @return The team.
     */
    virtual ETeamIndex GetTeam() const = 0;

    /**
     * @brief Gets the corresponding index of the team in the Teams array.
     * @see ATDGameState::Teams
     * @return The index of the team in the game state Teams variable.
     */
    virtual uint8 GetTeamIndex() const
    {
        return static_cast<uint8>(GetTeam());
    }
};
