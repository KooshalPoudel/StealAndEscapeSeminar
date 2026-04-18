/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: EndScreenWidget.cpp
Author: Kushal Poudel and Alok Poudel
Last Modified: April 18, 2026

Description: Implementation of the end screen. Added Submit Score button that
             spawns the NameEntryWidget popup for leaderboard submission.
             Button is visible on win, hidden on lose, and disabled after
             one submission to prevent duplicates.
*/

#include "EndScreenWidget.h"
#include "NameEntryWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

void UEndScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_SubmitScore) Btn_SubmitScore->OnClicked.AddDynamic(this, &UEndScreenWidget::OnSubmitScoreClicked);
	if (Btn_Retry)       Btn_Retry->OnClicked.AddDynamic(this, &UEndScreenWidget::OnRetryClicked);
	if (Btn_MainMenu)    Btn_MainMenu->OnClicked.AddDynamic(this, &UEndScreenWidget::OnMainMenuClicked);
	if (Btn_Quit)        Btn_Quit->OnClicked.AddDynamic(this, &UEndScreenWidget::OnQuitClicked);
}

void UEndScreenWidget::ShowWinScreen(int32 ItemsCollected, int32 ItemsRequired, float TimeTaken, int32 FinalScore)
{
	// Cache for Submit button
	CachedItems = ItemsCollected;
	CachedTime = TimeTaken;
	CachedScore = FinalScore;

	if (Txt_Title)
	{
		Txt_Title->SetText(FText::FromString(TEXT("ESCAPED!")));
		Txt_Title->SetColorAndOpacity(FSlateColor(WinColor));
	}

	if (Txt_Stats)
	{
		FString Stats = FString::Printf(TEXT("Items Collected: %d / %d"),
			ItemsCollected, ItemsRequired);
		Txt_Stats->SetText(FText::FromString(Stats));
	}

	if (Txt_Time)
	{
		FString TimeStr = FString::Printf(TEXT("Time: %s"), *FormatTime(TimeTaken));
		Txt_Time->SetText(FText::FromString(TimeStr));
	}

	if (Txt_Score)
	{
		FString ScoreStr = FString::Printf(TEXT("Score: %d"), FinalScore);
		Txt_Score->SetText(FText::FromString(ScoreStr));
		Txt_Score->SetVisibility(ESlateVisibility::Visible);
	}

	// Show Submit Score button on win
	if (Btn_SubmitScore)
	{
		Btn_SubmitScore->SetVisibility(ESlateVisibility::Visible);
		Btn_SubmitScore->SetIsEnabled(true);
	}
}

void UEndScreenWidget::ShowLoseScreen(int32 ItemsCollected, int32 ItemsRequired, float TimeTaken)
{
	if (Txt_Title)
	{
		Txt_Title->SetText(FText::FromString(TEXT("BUSTED!")));
		Txt_Title->SetColorAndOpacity(FSlateColor(LoseColor));
	}

	if (Txt_Stats)
	{
		FString Stats = FString::Printf(TEXT("Items Collected: %d / %d"),
			ItemsCollected, ItemsRequired);
		Txt_Stats->SetText(FText::FromString(Stats));
	}

	if (Txt_Time)
	{
		FString TimeStr = FString::Printf(TEXT("Time: %s"), *FormatTime(TimeTaken));
		Txt_Time->SetText(FText::FromString(TimeStr));
	}

	// Hide score and Submit button on lose
	if (Txt_Score)
	{
		Txt_Score->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (Btn_SubmitScore)
	{
		Btn_SubmitScore->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UEndScreenWidget::OnSubmitScoreClicked()
{
	if (!NameEntryWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("EndScreenWidget - NameEntryWidgetClass is not set! Assign it on WBP_EndScreen Class Defaults."));
		return;
	}

	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	// Create the name entry popup and configure it with the cached score data
	UNameEntryWidget* Popup = CreateWidget<UNameEntryWidget>(PC, NameEntryWidgetClass);
	if (!Popup) return;

	// Current level name matches what OpenLevel uses
	FName LevelName = FName(*UGameplayStatics::GetCurrentLevelName(GetWorld(), true));
	Popup->ConfigureForScore(LevelName, CachedScore, CachedTime, CachedItems);

	// High Z-order so it sits on top of the end screen
	Popup->AddToViewport(100);

	// Disable Submit button so the player cannot spam multiple entries
	if (Btn_SubmitScore)
	{
		Btn_SubmitScore->SetIsEnabled(false);
	}
}

void UEndScreenWidget::OnRetryClicked()
{
	PrepareForLevelTransition();
	FString CurrentLevel = UGameplayStatics::GetCurrentLevelName(GetWorld(), true);
	UGameplayStatics::OpenLevel(GetWorld(), FName(*CurrentLevel));
}

void UEndScreenWidget::OnMainMenuClicked()
{
	PrepareForLevelTransition();
	UGameplayStatics::OpenLevel(GetWorld(), MainMenuLevelName);
}

void UEndScreenWidget::OnQuitClicked()
{
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		UGameplayStatics::SetGamePaused(GetWorld(), false);
		PC->ConsoleCommand(TEXT("quit"));
	}
}

void UEndScreenWidget::PrepareForLevelTransition()
{
	UGameplayStatics::SetGamePaused(GetWorld(), false);
	RemoveFromParent();
}

FString UEndScreenWidget::FormatTime(float Seconds) const
{
	if (Seconds < 0.f) Seconds = 0.f;
	int32 Minutes = FMath::FloorToInt(Seconds / 60.f);
	int32 Secs = FMath::FloorToInt(Seconds) % 60;
	return FString::Printf(TEXT("%02d:%02d"), Minutes, Secs);
}
