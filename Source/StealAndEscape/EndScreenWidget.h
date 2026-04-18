/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: EndScreenWidget.h
Author: Kushal Poudel and Alok Poudel
Last Modified: April 18, 2026

Description: End-of-game screen widget. Displays title, item stats, time,
             score, and action buttons. On win a Submit Score button appears
             which opens the NameEntryWidget popup for leaderboard submission.

             Update (April 18 2026): Added Btn_SubmitScore and NameEntryWidgetClass
             for the leaderboard submission flow.
*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EndScreenWidget.generated.h"

class UButton;
class UTextBlock;
class UNameEntryWidget;

UCLASS()
class STEALANDESCAPE_API UEndScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/* Configure for WIN - shows items, time, score, and Submit Score button */
	UFUNCTION(BlueprintCallable, Category = "EndScreen")
		void ShowWinScreen(int32 ItemsCollected, int32 ItemsRequired, float TimeTaken, int32 FinalScore);

	/* Configure for LOSE - shows items and time, hides score and Submit */
	UFUNCTION(BlueprintCallable, Category = "EndScreen")
		void ShowLoseScreen(int32 ItemsCollected, int32 ItemsRequired, float TimeTaken);

protected:
	UPROPERTY(meta = (BindWidget))
		UTextBlock* Txt_Title;

	UPROPERTY(meta = (BindWidget))
		UTextBlock* Txt_Stats;

	UPROPERTY(meta = (BindWidget))
		UTextBlock* Txt_Time;

	UPROPERTY(meta = (BindWidget))
		UTextBlock* Txt_Score;

	UPROPERTY(meta = (BindWidget))
		UButton* Btn_SubmitScore;

	UPROPERTY(meta = (BindWidget))
		UButton* Btn_Retry;

	UPROPERTY(meta = (BindWidget))
		UButton* Btn_MainMenu;

	UPROPERTY(meta = (BindWidget))
		UButton* Btn_Quit;

	UPROPERTY(EditDefaultsOnly, Category = "Levels")
		FName MainMenuLevelName = TEXT("L_MainMenu");

	UPROPERTY(EditDefaultsOnly, Category = "Colors")
		FLinearColor WinColor = FLinearColor(0.2f, 1.0f, 0.3f, 1.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Colors")
		FLinearColor LoseColor = FLinearColor(1.0f, 0.2f, 0.2f, 1.0f);

	/* Widget class for the name entry popup. Set in WBP_EndScreen Class Defaults. */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
		TSubclassOf<UNameEntryWidget> NameEntryWidgetClass;

private:
	/* Cached values for when Submit is clicked */
	int32 CachedItems = 0;
	float CachedTime = 0.f;
	int32 CachedScore = 0;

	UFUNCTION() void OnSubmitScoreClicked();
	UFUNCTION() void OnRetryClicked();
	UFUNCTION() void OnMainMenuClicked();
	UFUNCTION() void OnQuitClicked();

	void PrepareForLevelTransition();
	FString FormatTime(float Seconds) const;
};
