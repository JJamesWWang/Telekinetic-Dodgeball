// Copyright 2021, James S. Wang, All rights reserved.

/**
 * Contains various configurations for the game that can be set.
 */

#pragma once

DECLARE_LOG_CATEGORY_EXTERN(LobbySystem, Log, All)

DECLARE_LOG_CATEGORY_EXTERN(BFMovement, Log, All)

/* Use for general logs */
DECLARE_LOG_CATEGORY_EXTERN(LogTD, Log, All)

/* Use for the gameplay ability system */
DECLARE_LOG_CATEGORY_EXTERN(LogTDGAS, Log, All)

#define ECC_Player ECC_GameTraceChannel1
#define ECC_Orb ECC_GameTraceChannel2
#define ECC_Telekinetic ECC_GameTraceChannel3

/**
 * @brief Logs a warning that a method was returned from early due to a nullptr.
 * @param ClassName The name of the class.
 * @param MethodName The name of the method.
 * @param PointerName The name of the pointer that was null.
 * @param Hint An error message to explain what the issue may be.
 * @param Role The actor/component owner's role in the network.
 */
inline void LogInvalidPointer(
    const FString& ClassName, const FString& MethodName,
    const FString& PointerName, const FString& Hint = "",
    ENetRole Role = ROLE_None)
{
    const UEnum* RoleEnum = StaticEnum<ENetRole>();
    UE_LOG(LogTD, Warning,
        TEXT("%s: Returned early from %s::%s because %s was invalid. %s"),
        *RoleEnum->GetNameStringByValue(Role), *ClassName, *MethodName,
        *PointerName, *Hint);
}
