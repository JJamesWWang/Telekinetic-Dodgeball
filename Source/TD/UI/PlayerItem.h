// Copyright 2021, James S. Wang, All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "PlayerItem.generated.h"

class UTextBlock;

/**
 * @brief Struct carrying the data to be displayed in the team player lists
 * in the lobby
 */
USTRUCT(BlueprintType)
struct TD_API FPlayerItemData
{
    GENERATED_BODY()

    /**
     * @brief The name of the player.
     */
    UPROPERTY(BlueprintReadWrite)
    FString PlayerName;
};

/**
 * @brief Widget representing a row in the team player list.
 */
UCLASS()
class TD_API UPlayerItem : public UUserWidget, public IUserObjectListEntry
{
    GENERATED_BODY()

public:
    /**
     * @brief Sets this widget's FPlayerItemData.
     * @param ItemData The item data to set this widget's data to.
     */
    UFUNCTION(BlueprintCallable, Category = "UI Data")
    void SetPlayerItemData(const FPlayerItemData& ItemData)
    {
        Data = ItemData;
    }

protected:
    /**
     * @brief The FPlayerItemData that this widget displays.
     */
    UPROPERTY(BlueprintReadWrite)
    FPlayerItemData Data;
};
