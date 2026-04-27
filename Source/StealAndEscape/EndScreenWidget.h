/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar

File Name: EndScreenWidget.h
Author: Kushal Poudel and Alok Poudel
Last Modified: April 26, 2026

Description: Header file for EndScreenWidget.cpp . This is the widget shown when the game
			 ends , either by winning or by getting caught. Has buttons for submiting score ,
			 retrying the level , going to main menu and quiting the game.
*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EndScreenWidget.generated.h"

// Forward declarations so we donot need to include the full headers here.
class UButton;
class UTextBlock;
class UNameEntryWidget;

UCLASS()
class STEALANDESCAPE_API UEndScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// NativeConstruct is called when widget is created , we use it to bind button clickss. 
	virtual void NativeConstruct() override;

	// Called by GameMode when player wins the game , shows win layout with score
	UFUNCTION(BlueprintCallable, Category = "EndScreen")
		void ShowWinScreen(int32 ItemsCollected, int32 ItemsRequired, float TimeTaken, int32 FinalScore);

	// Called by GameMode when player gets caught , shows lose layout without score
	UFUNCTION(BlueprintCallable, Category = "EndScreen")
		void ShowLoseScreen(int32 ItemsCollected, int32 ItemsRequired, float TimeTaken);

protected:
	/* These are the UI elements binded from the blueprint widget . BindWidget meta tag
	   makes sure these names match exactly with the widget names in WBP_EndScreen
	*/
	UPROPERTY(meta = (BindWidget)) UTextBlock* Txt_Title;
	UPROPERTY(meta = (BindWidget)) UTextBlock* Txt_Stats;
	UPROPERTY(meta = (BindWidget)) UTextBlock* Txt_Time;
	UPROPERTY(meta = (BindWidget)) UTextBlock* Txt_Score;

	/* Buttons binded from the blueprint widget */
	UPROPERTY(meta = (BindWidget)) UButton* Btn_SubmitScore;
	UPROPERTY(meta = (BindWidget)) UButton* Btn_Retry;
	UPROPERTY(meta = (BindWidget)) UButton* Btn_MainMenu;
	UPROPERTY(meta = (BindWidget)) UButton* Btn_Quit;

	// Name of the main menu level which is loaded when player clicks main menu button 
	UPROPERTY(EditDefaultsOnly, Category = "Levels")
		FName MainMenuLevelName = TEXT("L_MainMenu");

	/* These colors are used for the title text , green for win and red for lose .
	   These areSet in blueprint defaults so we can change them without recompling code
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Colors")
		FLinearColor WinColor = FLinearColor(0.2f, 1.0f, 0.3f, 1.0f);
	UPROPERTY(EditDefaultsOnly, Category = "Colors")
		FLinearColor LoseColor = FLinearColor(1.0f, 0.2f, 0.2f, 1.0f);

	// Name entry popup widget class , set in blueprint and opened when submit score is clicked
	UPROPERTY(EditDefaultsOnly, Category = "UI")
		TSubclassOf<UNameEntryWidget> NameEntryWidgetClass;

	// Button click sound effect , setted in blueprint defaults under Audio catagory 
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
		USoundBase* ButtonClickSound;

private:
	/* Cached values from the win screen so they can be used later when player clicks
	   submit score button to save thier score to the leaderboard. 
	*/
	int32 CachedItems = 0;
	float CachedTime = 0.f;
	int32 CachedScore = 0;

	// Button click handler funtions , each one plays click sound and does its action
	UFUNCTION() void OnSubmitScoreClicked();
	UFUNCTION() void OnRetryClicked();
	UFUNCTION() void OnMainMenuClicked();
	UFUNCTION() void OnQuitClicked();

	// Helper that unpauses game and removes widget before loading anew level
	void PrepareForLevelTransition();

	// Helper that converts seconds into MM:SS string format for the time display
	FString FormatTime(float Seconds) const;

	// Helper that plays the button click sound for audio feedback.
	void PlayClickSound();
};