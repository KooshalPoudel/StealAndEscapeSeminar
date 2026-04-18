/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: NameEntryWidget.h
Author: Kushal Poudel and Alok Poudel
Last Modified: April 18, 2026

Description: Popup widget shown after a win when the player clicks Submit Score.
             Contains a text box for the player name, a Submit button, and a
             Cancel button. On Submit the widget writes the score into the
             leaderboard save file then closes itself.

             Uses UEditableTextBox which renders with a visible border and
             background so the input field stands out visually.
*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NameEntryWidget.generated.h"

class UButton;
class UEditableTextBox;
class UTextBlock;

UCLASS()
class STEALANDESCAPE_API UNameEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/* Pre-load this widget with score details before adding to viewport. */
	UFUNCTION(BlueprintCallable, Category = "Leaderboard")
		void ConfigureForScore(FName InLevelName, int32 InScore, float InTimeTaken, int32 InItemsCollected);

protected:
	// Text box where player types their name - has visible border
	UPROPERTY(meta = (BindWidget))
		UEditableTextBox* Input_PlayerName;

	// Submit button - writes the score and closes
	UPROPERTY(meta = (BindWidget))
		UButton* Btn_Submit;

	// Cancel button - closes without saving
	UPROPERTY(meta = (BindWidget))
		UButton* Btn_Cancel;

	// Preview text showing the score the player is about to submit
	UPROPERTY(meta = (BindWidget))
		UTextBlock* Txt_ScorePreview;

private:
	FName LevelName;
	int32 Score = 0;
	float TimeTaken = 0.f;
	int32 ItemsCollected = 0;

	UFUNCTION() void OnSubmitClicked();
	UFUNCTION() void OnCancelClicked();
};
