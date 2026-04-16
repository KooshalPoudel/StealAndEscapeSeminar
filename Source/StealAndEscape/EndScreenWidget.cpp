/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: EndScreenWidget.cpp
Author: Kushal Poudel and Alok Poudel
Last Modified: April 16, 2026

Description: Implementation of the end-of-game screen. Configures win / lose
             appearance and handles the three button actions (retry current
             level, return to main menu, quit game).

             Key detail: before loading a level we must unpause the game and
             reset input mode. If we do not, the next level will load paused
             or the character will be unable to receive input because the
             UI-only input mode from this widget is still active.
*/

#include "EndScreenWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

void UEndScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind all three buttons to their click handlers. AddDynamic requires a
	// UFUNCTION because OnClicked is a dynamic multicast delegate.
	if (Btn_Retry)    Btn_Retry->OnClicked.AddDynamic(this, &UEndScreenWidget::OnRetryClicked);
	if (Btn_MainMenu) Btn_MainMenu->OnClicked.AddDynamic(this, &UEndScreenWidget::OnMainMenuClicked);
	if (Btn_Quit)     Btn_Quit->OnClicked.AddDynamic(this, &UEndScreenWidget::OnQuitClicked);
}

void UEndScreenWidget::ShowWinScreen(int32 ItemsCollected, int32 ItemsRequired)
{
	// Set the title text and color for a win
	if (Txt_Title)
	{
		Txt_Title->SetText(FText::FromString(TEXT("ESCAPED!")));
		Txt_Title->SetColorAndOpacity(FSlateColor(WinColor));
	}

	// Subtitle shows how many items the player collected
	if (Txt_Stats)
	{
		FString Stats = FString::Printf(TEXT("Items Collected: %d / %d"),
			ItemsCollected, ItemsRequired);
		Txt_Stats->SetText(FText::FromString(Stats));
	}
}

void UEndScreenWidget::ShowLoseScreen(int32 ItemsCollected, int32 ItemsRequired)
{
	// Set the title text and color for a loss
	if (Txt_Title)
	{
		Txt_Title->SetText(FText::FromString(TEXT("BUSTED!")));
		Txt_Title->SetColorAndOpacity(FSlateColor(LoseColor));
	}

	// Subtitle still shows items collected so the player sees how close they were
	if (Txt_Stats)
	{
		FString Stats = FString::Printf(TEXT("Items Collected: %d / %d"),
			ItemsCollected, ItemsRequired);
		Txt_Stats->SetText(FText::FromString(Stats));
	}
}

/* =============================================================================
   BUTTON HANDLERS
   ============================================================================= */

void UEndScreenWidget::OnRetryClicked()
{
	PrepareForLevelTransition();

	// Reload the current level. Using GetCurrentLevelName gives us the short
	// name (without the /Game/ path prefix) which OpenLevel can accept.
	FString CurrentLevel = UGameplayStatics::GetCurrentLevelName(GetWorld(), true);
	UGameplayStatics::OpenLevel(GetWorld(), FName(*CurrentLevel));
}

void UEndScreenWidget::OnMainMenuClicked()
{
	PrepareForLevelTransition();

	// Load the main menu level - level name is configurable on the widget
	UGameplayStatics::OpenLevel(GetWorld(), MainMenuLevelName);
}

void UEndScreenWidget::OnQuitClicked()
{
	// ConsoleCommand("quit") is the simplest way to exit the game from UI
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		// Unpause first so the quit console command is processed (paused games
		// ignore most world ticks which can block shutdown on some platforms)
		UGameplayStatics::SetGamePaused(GetWorld(), false);
		PC->ConsoleCommand(TEXT("quit"));
	}
}

/* =============================================================================
   HELPERS
   ============================================================================= */

void UEndScreenWidget::PrepareForLevelTransition()
{
	/* Unpause the game before loading a new level. If we leave it paused the
	   next level will load paused too and the player will see a frozen scene. */
	UGameplayStatics::SetGamePaused(GetWorld(), false);

	// Remove this widget from the viewport so it does not overlap the new level
	RemoveFromParent();
}
