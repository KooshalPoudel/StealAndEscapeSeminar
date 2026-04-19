/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: MainMenuWidget.cpp
Author: Kushal Poudel and Alok Poudel
Last Modified: April 19, 2026

Description: Main menu implementation with leaderboard, button click sounds,
             and working master volume slider that controls Ambient Sound actors.
*/

#include "MainMenuWidget.h"
#include "LeaderboardSaveGame.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Slider.h"
#include "Components/CheckBox.h"
#include "Components/Widget.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameUserSettings.h"
#include "Sound/AmbientSound.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Play)         Btn_Play->OnClicked.AddDynamic(this, &UMainMenuWidget::OnPlayClicked);
	if (Btn_Leaderboard)  Btn_Leaderboard->OnClicked.AddDynamic(this, &UMainMenuWidget::OnLeaderboardClicked);
	if (Btn_Options)      Btn_Options->OnClicked.AddDynamic(this, &UMainMenuWidget::OnOptionsClicked);
	if (Btn_Credits)      Btn_Credits->OnClicked.AddDynamic(this, &UMainMenuWidget::OnCreditsClicked);
	if (Btn_Quit)         Btn_Quit->OnClicked.AddDynamic(this, &UMainMenuWidget::OnQuitClicked);

	if (Btn_Level1)           Btn_Level1->OnClicked.AddDynamic(this, &UMainMenuWidget::OnLevel1Clicked);
	if (Btn_Level2)           Btn_Level2->OnClicked.AddDynamic(this, &UMainMenuWidget::OnLevel2Clicked);
	if (Btn_Level3)           Btn_Level3->OnClicked.AddDynamic(this, &UMainMenuWidget::OnLevel3Clicked);
	if (Btn_LevelSelectBack)  Btn_LevelSelectBack->OnClicked.AddDynamic(this, &UMainMenuWidget::OnLevelSelectBackClicked);

	if (Btn_TabLevel1)        Btn_TabLevel1->OnClicked.AddDynamic(this, &UMainMenuWidget::OnTabLevel1Clicked);
	if (Btn_TabLevel2)        Btn_TabLevel2->OnClicked.AddDynamic(this, &UMainMenuWidget::OnTabLevel2Clicked);
	if (Btn_TabLevel3)        Btn_TabLevel3->OnClicked.AddDynamic(this, &UMainMenuWidget::OnTabLevel3Clicked);
	if (Btn_LeaderboardBack)  Btn_LeaderboardBack->OnClicked.AddDynamic(this, &UMainMenuWidget::OnLeaderboardBackClicked);

	if (Slider_MasterVolume) Slider_MasterVolume->OnValueChanged.AddDynamic(this, &UMainMenuWidget::OnMasterVolumeChanged);
	if (Check_Fullscreen)    Check_Fullscreen->OnCheckStateChanged.AddDynamic(this, &UMainMenuWidget::OnFullscreenChanged);
	if (Btn_OptionsBack)     Btn_OptionsBack->OnClicked.AddDynamic(this, &UMainMenuWidget::OnOptionsBackClicked);

	if (Btn_CreditsBack) Btn_CreditsBack->OnClicked.AddDynamic(this, &UMainMenuWidget::OnCreditsBackClicked);

	ShowPanel(Panel_Main);
	InitializeOptionsFromSettings();
}

/* =============================================================================
   MAIN PANEL
   ============================================================================= */

void UMainMenuWidget::OnPlayClicked()         { PlayClickSound(); ShowPanel(Panel_LevelSelect); }

void UMainMenuWidget::OnLeaderboardClicked()
{
	PlayClickSound();
	ShowLevelScores(Level1Name, Level1Title);
	ShowPanel(Panel_Leaderboard);
}

void UMainMenuWidget::OnOptionsClicked()      { PlayClickSound(); ShowPanel(Panel_Options); }
void UMainMenuWidget::OnCreditsClicked()      { PlayClickSound(); ShowPanel(Panel_Credits); }

void UMainMenuWidget::OnQuitClicked()
{
	PlayClickSound();
	APlayerController* PC = GetOwningPlayer();
	if (PC) PC->ConsoleCommand(TEXT("quit"));
}

/* =============================================================================
   LEVEL SELECT
   ============================================================================= */

void UMainMenuWidget::OnLevel1Clicked()          { PlayClickSound(); LaunchLevel(Level1Name); }
void UMainMenuWidget::OnLevel2Clicked()          { PlayClickSound(); LaunchLevel(Level2Name); }
void UMainMenuWidget::OnLevel3Clicked()          { PlayClickSound(); LaunchLevel(Level3Name); }
void UMainMenuWidget::OnLevelSelectBackClicked() { PlayClickSound(); ShowPanel(Panel_Main); }

/* =============================================================================
   LEADERBOARD TABS
   ============================================================================= */

void UMainMenuWidget::OnTabLevel1Clicked() { PlayClickSound(); ShowLevelScores(Level1Name, Level1Title); }
void UMainMenuWidget::OnTabLevel2Clicked() { PlayClickSound(); ShowLevelScores(Level2Name, Level2Title); }
void UMainMenuWidget::OnTabLevel3Clicked() { PlayClickSound(); ShowLevelScores(Level3Name, Level3Title); }
void UMainMenuWidget::OnLeaderboardBackClicked() { PlayClickSound(); ShowPanel(Panel_Main); }

/* =============================================================================
   OPTIONS
   ============================================================================= */

void UMainMenuWidget::OnMasterVolumeChanged(float NewValue)
{
	// Find all Ambient Sound actors in the current level and set their volume
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAmbientSound::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		AAmbientSound* AmbSound = Cast<AAmbientSound>(Actor);
		if (AmbSound && AmbSound->GetAudioComponent())
		{
			AmbSound->GetAudioComponent()->SetVolumeMultiplier(NewValue);
		}
	}
}

void UMainMenuWidget::OnFullscreenChanged(bool bIsChecked)
{
	PlayClickSound();
	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
	if (!Settings) return;
	Settings->SetFullscreenMode(bIsChecked ? EWindowMode::WindowedFullscreen : EWindowMode::Windowed);
	Settings->ApplySettings(false);
	Settings->SaveSettings();
}

void UMainMenuWidget::OnOptionsBackClicked() { PlayClickSound(); ShowPanel(Panel_Main); }
void UMainMenuWidget::OnCreditsBackClicked() { PlayClickSound(); ShowPanel(Panel_Main); }

/* =============================================================================
   HELPERS
   ============================================================================= */

void UMainMenuWidget::ShowPanel(UWidget* PanelToShow)
{
	if (Panel_Main)         Panel_Main->SetVisibility(ESlateVisibility::Collapsed);
	if (Panel_LevelSelect)  Panel_LevelSelect->SetVisibility(ESlateVisibility::Collapsed);
	if (Panel_Leaderboard)  Panel_Leaderboard->SetVisibility(ESlateVisibility::Collapsed);
	if (Panel_Options)      Panel_Options->SetVisibility(ESlateVisibility::Collapsed);
	if (Panel_Credits)      Panel_Credits->SetVisibility(ESlateVisibility::Collapsed);

	if (PanelToShow) PanelToShow->SetVisibility(ESlateVisibility::Visible);
}

void UMainMenuWidget::LaunchLevel(FName LevelName)
{
	if (LevelName.IsNone()) return;
	RemoveFromParent();
	UGameplayStatics::OpenLevel(GetWorld(), LevelName);
}

void UMainMenuWidget::InitializeOptionsFromSettings()
{
	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
	if (!Settings) return;

	if (Check_Fullscreen)
	{
		const bool bIsFullscreen =
			Settings->GetFullscreenMode() == EWindowMode::Fullscreen ||
			Settings->GetFullscreenMode() == EWindowMode::WindowedFullscreen;
		Check_Fullscreen->SetIsChecked(bIsFullscreen);
	}

	if (Slider_MasterVolume)
	{
		Slider_MasterVolume->SetValue(1.0f);
	}
}

void UMainMenuWidget::PlayClickSound()
{
	if (ButtonClickSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), ButtonClickSound);
	}
}

/* =============================================================================
   LEADERBOARD DISPLAY
   ============================================================================= */

void UMainMenuWidget::ShowLevelScores(FName LevelName, const FString& LevelTitle)
{
	if (Txt_LeaderboardHeader)
		Txt_LeaderboardHeader->SetText(FText::FromString(LevelTitle));

	if (!ScoreRowsContainer) return;
	ScoreRowsContainer->ClearChildren();

	UWidget* Header = CreateHeaderRow();
	if (Header) ScoreRowsContainer->AddChildToVerticalBox(Header);

	TArray<FLeaderboardEntry> Entries;
	if (UGameplayStatics::DoesSaveGameExist(ULeaderboardSaveGame::SlotName, 0))
	{
		ULeaderboardSaveGame* Loaded = Cast<ULeaderboardSaveGame>(
			UGameplayStatics::LoadGameFromSlot(ULeaderboardSaveGame::SlotName, 0));
		if (Loaded) Entries = Loaded->GetEntries(LevelName);
	}

	for (int32 i = 0; i < 10; i++)
	{
		if (Entries.IsValidIndex(i))
		{
			UWidget* Row = CreateScoreRow(i + 1, Entries[i].PlayerName, Entries[i].Score, Entries[i].TimeTaken);
			if (Row) ScoreRowsContainer->AddChildToVerticalBox(Row);
		}
		else
		{
			UWidget* Row = CreateScoreRow(i + 1, TEXT("---"), 0, 0.f);
			if (Row) ScoreRowsContainer->AddChildToVerticalBox(Row);
		}
	}
}

UWidget* UMainMenuWidget::CreateHeaderRow()
{
	UHorizontalBox* Row = NewObject<UHorizontalBox>(this);
	if (!Row) return nullptr;

	auto AddColumn = [&](const FString& Text, float FillWidth)
	{
		UTextBlock* Col = NewObject<UTextBlock>(this);
		Col->SetText(FText::FromString(Text));
		FSlateFontInfo FontInfo = Col->Font;
		FontInfo.Size = 18;
		Col->SetFont(FontInfo);
		Col->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.8f, 0.f, 1.f)));
		UHorizontalBoxSlot* Slot = Row->AddChildToHorizontalBox(Col);
		if (Slot) Slot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	};

	AddColumn(TEXT("Rank"), 1.f);
	AddColumn(TEXT("Name"), 5.f);
	AddColumn(TEXT("Score"), 2.f);
	AddColumn(TEXT("Time"), 2.f);

	return Row;
}

UWidget* UMainMenuWidget::CreateScoreRow(int32 Rank, const FString& Name, int32 Score, float Time)
{
	UHorizontalBox* Row = NewObject<UHorizontalBox>(this);
	if (!Row) return nullptr;

	FString TimeStr = (Score > 0) ? FormatTime(Time) : TEXT("---");
	FString ScoreStr = (Score > 0) ? FString::FromInt(Score) : TEXT("---");

	auto AddColumn = [&](const FString& Text, float FillWidth)
	{
		UTextBlock* Col = NewObject<UTextBlock>(this);
		Col->SetText(FText::FromString(Text));
		FSlateFontInfo FontInfo = Col->Font;
		FontInfo.Size = 16;
		Col->SetFont(FontInfo);
		Col->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		UHorizontalBoxSlot* Slot = Row->AddChildToHorizontalBox(Col);
		if (Slot) Slot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	};

	AddColumn(FString::Printf(TEXT("%d."), Rank), 1.f);
	AddColumn(Name, 5.f);
	AddColumn(ScoreStr, 2.f);
	AddColumn(TimeStr, 2.f);

	return Row;
}

FString UMainMenuWidget::FormatTime(float Seconds) const
{
	if (Seconds < 0.f) Seconds = 0.f;
	int32 Minutes = FMath::FloorToInt(Seconds / 60.f);
	int32 Secs = FMath::FloorToInt(Seconds) % 60;
	return FString::Printf(TEXT("%02d:%02d"), Minutes, Secs);
}
