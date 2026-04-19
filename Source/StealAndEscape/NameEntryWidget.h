/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: NameEntryWidget.h
Author: Kushal Poudel and Alok Poudel
Last Modified: April 19, 2026
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

	UFUNCTION(BlueprintCallable, Category = "Leaderboard")
		void ConfigureForScore(FName InLevelName, int32 InScore, float InTimeTaken, int32 InItemsCollected);

protected:
	UPROPERTY(meta = (BindWidget)) UEditableTextBox* Input_PlayerName;
	UPROPERTY(meta = (BindWidget)) UButton* Btn_Submit;
	UPROPERTY(meta = (BindWidget)) UButton* Btn_Cancel;
	UPROPERTY(meta = (BindWidget)) UTextBlock* Txt_ScorePreview;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
		USoundBase* ButtonClickSound;

private:
	FName LevelName;
	int32 Score = 0;
	float TimeTaken = 0.f;
	int32 ItemsCollected = 0;

	UFUNCTION() void OnSubmitClicked();
	UFUNCTION() void OnCancelClicked();
	void PlayClickSound();
};
