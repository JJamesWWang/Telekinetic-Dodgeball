// Copyright 2021, James S. Wang, All rights reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "MainMenuInterface.h"

#include "LSWidget.generated.h"

class IMainMenuInterface;

/**
 * @brief The base widget class for all UI widgets.
 * It contains lifecycle methods like Create and Destroy that's common among
 * all widgets. It also holds onto a IMainMenuInterface for its children to use
 * if they need functionality specific to the game.
 */
UCLASS()
class TD_API ULSWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /**
     * @brief Does all of the setup required for this widget to be used.
     * By default, this will add the widget to the viewport and make it
     * focusable. Then it will set the input mode to UIOnly and show the mouse
     * cursor.
     */
    UFUNCTION(BlueprintCallable, Category = "Initialization")
    virtual void Create();

    /**
     * @brief Destroys the widget and cleans it up.
     * By default, this will remove the widget from the viewport and make it no
     * longer focusable. Then it will set the input mode to GameOnly and hide
     * the mouse cursor.
     */
    UFUNCTION(BlueprintCallable, Category = "Lifecycle")
    virtual void Destroy();

    /**
     * @brief Usually called when loading a new level (and so the old one is
     * getting destroyed). By default, this will Destroy() this widget before
     * calling the parent implementation.
     * @param InLevel The level that the game is currently in
     * @param InWorld The world the game is currently in
     */
    virtual void OnLevelRemovedFromWorld(
        ULevel* InLevel, UWorld* InWorld) override;

    /**
     * @brief Sets the MainMenuInterface so that we can call those functions
     * when necessary.
     * @param Interface The implementer of the IMainMenuInterface
     */
    void SetMainMenuInterface(TScriptInterface<IMainMenuInterface> Interface)
    {
        MainMenuInterface = Interface;
    };

private:
    /**
     * @brief The implementer of IMainMenuInterface that we can call methods
     * from. Using TScriptInterface allows us to use this in blueprints.
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite,
        Category = "IMainMenuInterface", meta = (AllowPrivateAccess = "true"))
    TScriptInterface<IMainMenuInterface> MainMenuInterface = nullptr;

    /**
     * @brief Gets the local player's PlayerController for setting things like
     * mouse cursor visibility.
     * @return The local player's PlayerController
     */
    APlayerController* GetPlayerController() const;
};
