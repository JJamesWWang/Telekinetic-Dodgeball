// Copyright 2021, James S. Wang, All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "TDTypes.h"
#include "TeamAssignable.h"
#include "GameFramework/PlayerState.h"
#include "TDPlayerState.generated.h"

class USoundCue;
class ATDController;
class ATDCharacter;

/**
 * @brief Information about the player, replicated to everyone.
 */
UCLASS()
class TD_API ATDPlayerState : public APlayerState, public ITeamAssignable
{
    GENERATED_BODY()

public:
    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    /* @see ITeamAssignable */
    virtual ETeamIndex GetTeam() const override;
    void SetTeam(const ETeamIndex TeamIndex);

    /**
     * @brief Gets whether this player is eliminated.
     */
    bool GetIsEliminated() const;

    /**
     * @brief Sets whether this player is eliminated.
     */
    void SetIsEliminated(const bool NewIsEliminated);

    /**
     * @brief Resets all variables that might change during the round state
     * back to their defaults.
     */
    void ResetRoundState();

private:
    /**
     * @brief The team that the player belongs to.
     */
    UPROPERTY(Replicated)
    ETeamIndex Team = ETeamIndex::None;

    /**
     * @brief Whether this player has been eliminated.
     */
    bool IsEliminated = false;
};
