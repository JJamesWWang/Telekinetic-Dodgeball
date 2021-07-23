// Copyright 2021, James S. Wang, All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TDWidget.generated.h"

UCLASS()
class TD_API UTDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /**
     * @brief Use this instead of AddToViewport, enables advanced cleanup.
     * @param Widget The created UTDWidget to add to the viewport.
     */
    UFUNCTION(BlueprintCallable, Category = "UI")
    void AddCreatedWidgetToViewport(UTDWidget* Widget);

    /**
     * @brief Sets the creator of this widget to the provided one.
     */
    void SetCreator(UTDWidget* Widget);

    /**
     * @brief Removes this widget from its creator and its created widgets.
     * Does the same thing as RemoveFromViewport from a BP POV. this widget
     */
    virtual void RemoveFromParent() override;

private:
    /**
     * @brief All TDWidgets that this widget has created.
     */
    UPROPERTY()
    TArray<UTDWidget*> CreatedWidgets;

    /**
     * @brief The TDWidget that created this widget, or nullptr if none.
     */
    UPROPERTY()
    UTDWidget* Creator = nullptr;

    /**
     * @brief Set on removal to ensure recursion isn't infinite.
     */
    bool IsMarkedForRemoval = false;

    /**
     * @brief Removes an existing created widget from the viewport.
     */
    void RemoveCreatedWidgetFromViewport(UTDWidget* Widget);

    /**
     * @brief Removes all created widgets from the viewport.
     */
    void RemoveAllCreatedWidgetsFromViewport();
};
