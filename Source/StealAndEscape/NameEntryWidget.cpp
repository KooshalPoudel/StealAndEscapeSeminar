/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar

File Name: NameEntryWidget.cpp
Author: Kushal Poudel and Alok Poudel
Last Modified: April 26, 2026

Description: Implementation of the name entry popup which is shown when player clicks submit
			 score on the win screen. Player types thier name and we save the score to the
			 leaderboard savegame slot.
*/

#include "NameEntryWidget.h"
#include "LeaderboardSaveGame.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

// NativeConstruct binds the submit and cancel button click events to thier handler funtions. 
void UNameEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (Btn_Submit) Btn_Submit->OnClicked.AddDynamic(this, &UNameEntryWidget::OnSubmitClicked);
	if (Btn_Cancel) Btn_Cancel->OnClicked.AddDynamic(this, &UNameEntryWidget::OnCancelClicked);
}

/* ConfigureForScore is called by EndScreenWidget right after the popup is created . It
   stores the score data for later and shows a preview text so player can see what they
   are submiting before they type thier name.
*/
void UNameEntryWidget::ConfigureForScore(FName InLevelName, int32 InScore, float InTimeTaken, int32 InItemsCollected)
{
	// Here Caching all the score data so we can use it when submit is clicked 
	LevelName = InLevelName;
	Score = InScore;
	TimeTaken = InTimeTaken;
	ItemsCollected = InItemsCollected;
	// Building a preview string that shows score , time and items in one line 
	if (Txt_ScorePreview)
	{
		int32 Minutes = FMath::FloorToInt(TimeTaken / 60.f);
		int32 Seconds = FMath::FloorToInt(TimeTaken) % 60;
		FString Preview = FString::Printf(TEXT("Score: %d   Time: %02d:%02d   Items: %d"),
			Score, Minutes, Seconds, ItemsCollected);
		Txt_ScorePreview->SetText(FText::FromString(Preview));
	}
}

/* OnSubmitClicked is called when player clicks the submit button. We grab the typed name ,
   load or create the leaderboard savegame , add the new entry and save it back to the
   slot. If player didnot type any name we just use "Anonymous" as defult.
*/
void UNameEntryWidget::OnSubmitClicked()
{
	PlayClickSound();
	// Getting the name typed by player and triming any extra spaces 
	FString TypedName;
	if (Input_PlayerName)
	{
		TypedName = Input_PlayerName->GetText().ToString().TrimStartAndEnd();
	}
	// If player left the box empty we just use Anonymous as defult name 
	if (TypedName.IsEmpty())
	{
		TypedName = TEXT("Anonymous");
	}
	// Trying to load existing leaderboard from save slot. 
	ULeaderboardSaveGame* Leaderboard = nullptr;
	if (UGameplayStatics::DoesSaveGameExist(ULeaderboardSaveGame::SlotName, 0))
	{
		Leaderboard = Cast<ULeaderboardSaveGame>(
			UGameplayStatics::LoadGameFromSlot(ULeaderboardSaveGame::SlotName, 0));
	}
	// If no save existed we create a brand new leaderboard savegame object
	if (!Leaderboard)
	{
		Leaderboard = Cast<ULeaderboardSaveGame>(
			UGameplayStatics::CreateSaveGameObject(ULeaderboardSaveGame::StaticClass()));
	}
	// Safety check , if it still failed somehow we just close the popup and bail out!
	if (!Leaderboard)
	{
		RemoveFromParent();
		return;
	}
	// Filling out the new entry with all the cached score data plus the current date!
	FLeaderboardEntry NewEntry;
	NewEntry.PlayerName = TypedName;
	NewEntry.Score = Score;
	NewEntry.TimeTaken = TimeTaken;
	NewEntry.ItemsCollected = ItemsCollected;
	NewEntry.DateString = FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M"));
	// Adding entry to the leaderboard and saving back to the slot 
	Leaderboard->AddEntry(LevelName, NewEntry);
	UGameplayStatics::SaveGameToSlot(Leaderboard, ULeaderboardSaveGame::SlotName, 0);
	// Closing the popup since we are done 
	RemoveFromParent();
}

// OnCancelClicked just closes the popup without saving anything.. 
void UNameEntryWidget::OnCancelClicked()
{
	PlayClickSound();
	RemoveFromParent();
}

// Small helper that plays the button click sound for audio feedback
void UNameEntryWidget::PlayClickSound()
{
	if (ButtonClickSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), ButtonClickSound);
	}
}