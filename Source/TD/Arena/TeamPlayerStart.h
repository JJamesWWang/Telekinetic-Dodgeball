// Copyright 2021, James S. Wang, All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "TeamAssignable.h"

#include "TeamPlayerStart.generated.h"

enum class ETeamIndex : uint8;

/**
 * A player start that can be assigned to a team; in other words, players on
 * the same team can be selected to spawn on this player start.
 */
UCLASS()
class TD_API ATeamPlayerStart : public APlayerStart, public ITeamAssignable
{
    GENERATED_BODY()

public:
    /* @see ITeamAssignable */
    virtual ETeamIndex GetTeam() const override;

protected:
    /**
     * @brief The team this player start belongs to.
     */
    UPROPERTY(EditAnywhere)
    ETeamIndex Team;
};
