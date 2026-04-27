/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar

File Name: NameEntryWidget.h
Author: Kushal Poudel and Alok Poudel
Last Modified: April 26, 2026

Description: Header file for NameEntryWidget.cpp . Popup widget that lets player type thier
			 name and submit thier score to the leaderboard.
*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NameEntryWidget.generated.h"

// Forward declarations so we donot need to include the full headers here. 
class UButton;
class UEditableTextBox;
class UTextBlock;

UCLASS()
class STEALANDESCAPE_API UNameEntryWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	// Called when widget is created , we use itto bind submit and cancel buttons
	virtual void NativeConstruct() override;
	// Called by EndScreenWidget right after creating popup to pass in the score data 
	UFUNCTION(BlueprintCallable, Category = "Leaderboard")
		void ConfigureForScore(FName InLevelName, int32 InScore, float InTimeTaken, int32 InItemsCollected);
protected:
	// UI elements binded from the blueprint widget WBP_NameEntry
	UPROPERTY(meta = (BindWidget)) UEditableTextBox* Input_PlayerName;
	UPROPERTY(meta = (BindWidget)) UButton* Btn_Submit;
	UPROPERTY(meta = (BindWidget)) UButton* Btn_Cancel;
	UPROPERTY(meta = (BindWidget)) UTextBlock* Txt_ScorePreview;
	// Button click sound effect , set in blueprint defaults under Audio catagory!
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
		USoundBase* ButtonClickSound;
private:
	// Cached score data passed in from EndScreenWidget , used when submit is clicked. 
	FName LevelName;
	int32 Score = 0;
	float TimeTaken = 0.f;
	int32 ItemsCollected = 0;
	// Button click handler funtions
	UFUNCTION() void OnSubmitClicked();
	UFUNCTION() void OnCancelClicked();
	// Helper that plays the button click sound 
	void PlayClickSound();
};