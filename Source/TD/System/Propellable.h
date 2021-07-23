// Copyright 2021, James S. Wang, All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Propellable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPropellable : public UInterface
{
	GENERATED_BODY()
};

/**
 * @brief Propellable actors can be propelled by orbs when the orbs collide with
 * them instead of the orb bouncing off of them.
 */
class IPropellable
{
	GENERATED_BODY()

public:
    /**
     * @brief Called when the orb hits the propellable actor.
     * @param Velocity The velocity of the orb at the moment of impact.
     */
    virtual void OnPropelled(const FVector& Velocity) = 0;
};
