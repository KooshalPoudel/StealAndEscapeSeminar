/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: EndScreenWidget.h
Author: Kushal Poudel and Alok Poudel
Last Modified: April 16, 2026

Description: End-of-game screen widget shown when the player wins (escaped with
             all items) or loses (caught by a guard). Displays a title that
             changes based on the outcome, a subtitle showing stats like items
             collected, and three action buttons: Retry, Main Menu, and Quit.

             Spawned by StealAndEscapeGameMode on win or lose. The widget is
             added to the viewport while the game is paused so the end screen
             pauses gameplay and freezes guard movement.

             The title text and color are set dynamically via ShowWinScreen or
             ShowLoseScreen before the widget is added to the viewport. The
             title TextBlock and subtitle TextBlock are bound by name so the
             UMG asset must contain widgets named exactly Txt_Title and
             Txt_Stats for the binding to resolve.
*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EndScreenWidget.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class STEALANDESCAPE_API UEndScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/* Configure this widget for a WIN outcome before adding to viewport.
	   Sets title to ESCAPED and the stats line to the items collected. */
	UFUNCTION(BlueprintCallable, Category = "EndScreen")
		void ShowWinScreen(int32 ItemsCollected, int32 ItemsRequired);

	/* Configure this widget for a LOSE outcome before adding to viewport.
	   Sets title to BUSTED and the stats line to the items collected. */
	UFUNCTION(BlueprintCallable, Category = "EndScreen")
		void ShowLoseScreen(int32 ItemsCollected, int32 ItemsRequired);

protected:
	/* ======================================================================
	   BOUND WIDGETS - must exist in the UMG asset with these exact names.
	   ====================================================================== */

	// Large title text - set to ESCAPED or BUSTED depending on outcome
	UPROPERTY(meta = (BindWidget))
		UTextBlock* Txt_Title;

	// Smaller subtitle text showing items collected
	UPROPERTY(meta = (BindWidget))
		UTextBlock* Txt_Stats;

	// Retry - reloads the current level
	UPROPERTY(meta = (BindWidget))
		UButton* Btn_Retry;

	// Main Menu - opens L_MainMenu
	UPROPERTY(meta = (BindWidget))
		UButton* Btn_MainMenu;

	// Quit - exits the game
	UPROPERTY(meta = (BindWidget))
		UButton* Btn_Quit;

	/* ======================================================================
	   CONFIGURATION - set on the Widget Blueprint defaults.
	   ====================================================================== */

	// Name of the main menu level to return to when Main Menu is clicked
	UPROPERTY(EditDefaultsOnly, Category = "Levels")
		FName MainMenuLevelName = TEXT("L_MainMenu");

	// Color applied to the title text on a win (green by default)
	UPROPERTY(EditDefaultsOnly, Category = "Colors")
		FLinearColor WinColor = FLinearColor(0.2f, 1.0f, 0.3f, 1.0f);

	// Color applied to the title text on a loss (red by default)
	UPROPERTY(EditDefaultsOnly, Category = "Colors")
		FLinearColor LoseColor = FLinearColor(1.0f, 0.2f, 0.2f, 1.0f);

private:
	/* Button click handlers. UFUNCTION is required for AddDynamic to bind them. */
	UFUNCTION() void OnRetryClicked();
	UFUNCTION() void OnMainMenuClicked();
	UFUNCTION() void OnQuitClicked();

	/* Common cleanup before loading a new level.
	   Unpauses the game and clears UI-only input mode so the next level works
	   normally. Without these the new level would load paused or with the UI
	   input mode stuck and the player could not move. */
	void PrepareForLevelTransition();
};
