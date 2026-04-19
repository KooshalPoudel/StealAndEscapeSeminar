/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: EndScreenWidget.h
Author: Kushal Poudel and Alok Poudel
Last Modified: April 19, 2026
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

	UFUNCTION(BlueprintCallable, Category = "EndScreen")
		void ShowWinScreen(int32 ItemsCollected, int32 ItemsRequired, float TimeTaken, int32 FinalScore);

	UFUNCTION(BlueprintCallable, Category = "EndScreen")
		void ShowLoseScreen(int32 ItemsCollected, int32 ItemsRequired, float TimeTaken);

protected:
	UPROPERTY(meta = (BindWidget)) UTextBlock* Txt_Title;
	UPROPERTY(meta = (BindWidget)) UTextBlock* Txt_Stats;
	UPROPERTY(meta = (BindWidget)) UTextBlock* Txt_Time;
	UPROPERTY(meta = (BindWidget)) UTextBlock* Txt_Score;
	UPROPERTY(meta = (BindWidget)) UButton* Btn_SubmitScore;
	UPROPERTY(meta = (BindWidget)) UButton* Btn_Retry;
	UPROPERTY(meta = (BindWidget)) UButton* Btn_MainMenu;
	UPROPERTY(meta = (BindWidget)) UButton* Btn_Quit;

	UPROPERTY(EditDefaultsOnly, Category = "Levels")
		FName MainMenuLevelName = TEXT("L_MainMenu");

	UPROPERTY(EditDefaultsOnly, Category = "Colors")
		FLinearColor WinColor = FLinearColor(0.2f, 1.0f, 0.3f, 1.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Colors")
		FLinearColor LoseColor = FLinearColor(1.0f, 0.2f, 0.2f, 1.0f);

	UPROPERTY(EditDefaultsOnly, Category = "UI")
		TSubclassOf<UNameEntryWidget> NameEntryWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
		USoundBase* ButtonClickSound;

private:
	int32 CachedItems = 0;
	float CachedTime = 0.f;
	int32 CachedScore = 0;

	UFUNCTION() void OnSubmitScoreClicked();
	UFUNCTION() void OnRetryClicked();
	UFUNCTION() void OnMainMenuClicked();
	UFUNCTION() void OnQuitClicked();

	void PrepareForLevelTransition();
	FString FormatTime(float Seconds) const;
	void PlayClickSound();
};
