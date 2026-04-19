/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: PauseMenuWidget.h
Author: Kushal Poudel and Alok Poudel
Last Modified: April 19, 2026
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
	UPROPERTY(meta = (BindWidget)) UButton* Btn_Resume;
	UPROPERTY(meta = (BindWidget)) UButton* Btn_MainMenu;
	UPROPERTY(meta = (BindWidget)) UButton* Btn_Quit;

	UPROPERTY(EditDefaultsOnly, Category = "Levels")
		FName MainMenuLevelName = TEXT("L_MainMenu");

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
		USoundBase* ButtonClickSound;

private:
	UFUNCTION() void OnResumeClicked();
	UFUNCTION() void OnMainMenuClicked();
	UFUNCTION() void OnQuitClicked();

	void PrepareForLevelTransition();
	void PlayClickSound();
};
