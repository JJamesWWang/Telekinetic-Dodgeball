// Copyright 2021, James S. Wang, All rights reserved.

#include "UI/TDWidget.h"

void UTDWidget::AddCreatedWidgetToViewport(UTDWidget* Widget)
{
    CreatedWidgets.Emplace(Widget);
    Widget->SetCreator(this);
    Widget->AddToScreen(nullptr, 0);
}

void UTDWidget::SetCreator(UTDWidget* Widget)
{
    Creator = Widget;
}

void UTDWidget::RemoveFromParent()
{
    // 2-pass recursion -> this is called first
    if (Creator != nullptr && !IsMarkedForRemoval)
    {
        IsMarkedForRemoval = true;
        // Tell the parent to remove this widget instead.
        Creator->RemoveCreatedWidgetFromViewport(this);
        return;
    }

    // Then this is called
    RemoveAllCreatedWidgetsFromViewport();
    Super::RemoveFromParent();
}

void UTDWidget::RemoveCreatedWidgetFromViewport(UTDWidget* Widget)
{
    CreatedWidgets.Remove(Widget);
    Widget->RemoveFromParent();
}

void UTDWidget::RemoveAllCreatedWidgetsFromViewport()
{
    for (int i = CreatedWidgets.Num() - 1; i >= 0; --i)
    {
        CreatedWidgets[i]->RemoveFromViewport();
    }
}
