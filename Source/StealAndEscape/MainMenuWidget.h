/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: MainMenuWidget.h
Author: Kushal Poudel and Alok Poudel
Last Modified: April 24, 2026

Description: Main menu widget with leaderboard, button click sounds, and
			 working master volume slider.

Updated: Added Btn_Tutorial button and OnTutorialClicked handler. When
		 clicked, loads the TutorialLevel (configurable via TutorialLevelName).
*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UButton;
class UWidget;
class UTextBlock;
class USlider;
class UCheckBox;
class UVerticalBox;

UCLASS()
class STEALANDESCAPE_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(meta = (BindWidget)) UButton* Btn_Play;
	UPROPERTY(meta = (BindWidget)) UButton* Btn_Tutorial;
	UPROPERTY(meta = (BindWidget)) UButton* Btn_Leaderboard;
	UPROPERTY(meta = (BindWidget)) UButton* Btn_Options;
	UPROPERTY(meta = (BindWidget)) UButton* Btn_Credits;
	UPROPERTY(meta = (BindWidget)) UButton* Btn_Quit;

	UPROPERTY(meta = (BindWidget)) UWidget* Panel_Main;
	UPROPERTY(meta = (BindWidget)) UWidget* Panel_LevelSelect;
	UPROPERTY(meta = (BindWidget)) UWidget* Panel_Leaderboard;
	UPROPERTY(meta = (BindWidget)) UWidget* Panel_Options;
	UPROPERTY(meta = (BindWidget)) UWidget* Panel_Credits;

	UPROPERTY(meta = (BindWidget)) UButton* Btn_Level1;
	UPROPERTY(meta = (BindWidget)) UButton* Btn_Level2;
	UPROPERTY(meta = (BindWidget)) UButton* Btn_Level3;
	UPROPERTY(meta = (BindWidget)) UButton* Btn_LevelSelectBack;

	UPROPERTY(meta = (BindWidget)) UButton* Btn_TabLevel1;
	UPROPERTY(meta = (BindWidget)) UButton* Btn_TabLevel2;
	UPROPERTY(meta = (BindWidget)) UButton* Btn_TabLevel3;
	UPROPERTY(meta = (BindWidget)) UTextBlock* Txt_LeaderboardHeader;
	UPROPERTY(meta = (BindWidget)) UVerticalBox* ScoreRowsContainer;
	UPROPERTY(meta = (BindWidget)) UButton* Btn_LeaderboardBack;

	UPROPERTY(meta = (BindWidget)) USlider* Slider_MasterVolume;
	UPROPERTY(meta = (BindWidget)) UCheckBox* Check_Fullscreen;
	UPROPERTY(meta = (BindWidget)) UButton* Btn_OptionsBack;

	UPROPERTY(meta = (BindWidget)) UButton* Btn_CreditsBack;

	/* Level that the Tutorial button loads.
	   Default value is TutorialLevel which should match the filename of
	   the tutorial level asset in Content/TopDownCPP/Maps/. If your tutorial
	   level has a different name, change this in BP_MainMenuWidget class defaults.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Levels") FName TutorialLevelName = TEXT("TutorialLevel");
	UPROPERTY(EditDefaultsOnly, Category = "Levels") FName Level1Name = TEXT("Level01");
	UPROPERTY(EditDefaultsOnly, Category = "Levels") FName Level2Name = TEXT("Level02");
	UPROPERTY(EditDefaultsOnly, Category = "Levels") FName Level3Name = TEXT("Level03");
	UPROPERTY(EditDefaultsOnly, Category = "Levels") FString Level1Title = TEXT("Level 1");
	UPROPERTY(EditDefaultsOnly, Category = "Levels") FString Level2Title = TEXT("Level 2");
	UPROPERTY(EditDefaultsOnly, Category = "Levels") FString Level3Title = TEXT("Level 3");

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
		USoundBase* ButtonClickSound;

private:
	UFUNCTION() void OnPlayClicked();
	UFUNCTION() void OnTutorialClicked();
	UFUNCTION() void OnLeaderboardClicked();
	UFUNCTION() void OnOptionsClicked();
	UFUNCTION() void OnCreditsClicked();
	UFUNCTION() void OnQuitClicked();

	UFUNCTION() void OnLevel1Clicked();
	UFUNCTION() void OnLevel2Clicked();
	UFUNCTION() void OnLevel3Clicked();
	UFUNCTION() void OnLevelSelectBackClicked();

	UFUNCTION() void OnTabLevel1Clicked();
	UFUNCTION() void OnTabLevel2Clicked();
	UFUNCTION() void OnTabLevel3Clicked();
	UFUNCTION() void OnLeaderboardBackClicked();

	UFUNCTION() void OnMasterVolumeChanged(float NewValue);
	UFUNCTION() void OnFullscreenChanged(bool bIsChecked);
	UFUNCTION() void OnOptionsBackClicked();

	UFUNCTION() void OnCreditsBackClicked();

	void ShowPanel(UWidget* PanelToShow);
	void LaunchLevel(FName LevelName);
	void InitializeOptionsFromSettings();

	void ShowLevelScores(FName LevelName, const FString& LevelTitle);
	UWidget* CreateScoreRow(int32 Rank, const FString& Name, int32 Score, float Time);
	UWidget* CreateHeaderRow();
	FString FormatTime(float Seconds) const;

	void PlayClickSound();
};