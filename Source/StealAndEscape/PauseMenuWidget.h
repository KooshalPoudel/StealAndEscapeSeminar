/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: PauseMenuWidget.h
Author: Kushal Poudel and Alok Poudel
Last Modified: April 16, 2026

Description: In-game pause menu widget. Shown by StealAndEscapePlayerController
             when the player presses Escape during gameplay. Offers three
             actions: Resume (closes menu and unpauses), Main Menu (returns
             to the title screen), and Quit (exits the application).

             The widget does not pause the game itself - that is handled by
             the player controller which spawns the widget so pause state
             stays centralized. When the Resume button is clicked the widget
             calls back into the controller to unpause and clean up.
*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.generated.h"

class UButton;
class AStealAndEscapePlayerController;

UCLASS()
class STEALANDESCAPE_API UPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	/* ======================================================================
	   BOUND BUTTONS - must exist in the UMG asset with these exact names.
	   ====================================================================== */

	UPROPERTY(meta = (BindWidget))
		UButton* Btn_Resume;

	UPROPERTY(meta = (BindWidget))
		UButton* Btn_MainMenu;

	UPROPERTY(meta = (BindWidget))
		UButton* Btn_Quit;

	/* ======================================================================
	   CONFIGURATION - set on the Widget Blueprint defaults.
	   ====================================================================== */

	// Name of the main menu level to return to when Main Menu is clicked
	UPROPERTY(EditDefaultsOnly, Category = "Levels")
		FName MainMenuLevelName = TEXT("L_MainMenu");

private:
	/* Button click handlers. UFUNCTION is required for AddDynamic binding. */
	UFUNCTION() void OnResumeClicked();
	UFUNCTION() void OnMainMenuClicked();
	UFUNCTION() void OnQuitClicked();

	/* Shared cleanup before loading a new level. Unpauses the game and
	   removes this widget from the viewport so the next level loads with a
	   clean UI state. */
	void PrepareForLevelTransition();
};
