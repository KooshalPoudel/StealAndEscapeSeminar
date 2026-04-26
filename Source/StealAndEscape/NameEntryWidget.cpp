/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: NameEntryWidget.cpp
Author: Kushal Poudel and Alok Poudel
Last Modified: April 19, 2026
*/

#include "NameEntryWidget.h"
#include "LeaderboardSaveGame.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void UNameEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Submit) Btn_Submit->OnClicked.AddDynamic(this, &UNameEntryWidget::OnSubmitClicked);
	if (Btn_Cancel) Btn_Cancel->OnClicked.AddDynamic(this, &UNameEntryWidget::OnCancelClicked);
}

void UNameEntryWidget::ConfigureForScore(FName InLevelName, int32 InScore, float InTimeTaken, int32 InItemsCollected)
{
	LevelName = InLevelName;
	Score = InScore;
	TimeTaken = InTimeTaken;
	ItemsCollected = InItemsCollected;

	if (Txt_ScorePreview)
	{
		int32 Minutes = FMath::FloorToInt(TimeTaken / 60.f);
		int32 Seconds = FMath::FloorToInt(TimeTaken) % 60;
		FString Preview = FString::Printf(TEXT("Score: %d   Time: %02d:%02d   Items: %d"),
			Score, Minutes, Seconds, ItemsCollected);
		Txt_ScorePreview->SetText(FText::FromString(Preview));
	}
}

void UNameEntryWidget::OnSubmitClicked()
{
	PlayClickSound();

	FString TypedName;
	if (Input_PlayerName)
	{
		TypedName = Input_PlayerName->GetText().ToString().TrimStartAndEnd();
	}
	if (TypedName.IsEmpty())
	{
		TypedName = TEXT("Anonymous");
	}

	ULeaderboardSaveGame* Leaderboard = nullptr;
	if (UGameplayStatics::DoesSaveGameExist(ULeaderboardSaveGame::SlotName, 0))
	{
		Leaderboard = Cast<ULeaderboardSaveGame>(
			UGameplayStatics::LoadGameFromSlot(ULeaderboardSaveGame::SlotName, 0));
	}
	if (!Leaderboard)
	{
		Leaderboard = Cast<ULeaderboardSaveGame>(
			UGameplayStatics::CreateSaveGameObject(ULeaderboardSaveGame::StaticClass()));
	}
	if (!Leaderboard)
	{
		RemoveFromParent();
		return;
	}

	FLeaderboardEntry NewEntry;
	NewEntry.PlayerName = TypedName;
	NewEntry.Score = Score;
	NewEntry.TimeTaken = TimeTaken;
	NewEntry.ItemsCollected = ItemsCollected;
	NewEntry.DateString = FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M"));

	Leaderboard->AddEntry(LevelName, NewEntry);

	UGameplayStatics::SaveGameToSlot(Leaderboard, ULeaderboardSaveGame::SlotName, 0);

	RemoveFromParent();
}

void UNameEntryWidget::OnCancelClicked()
{
	PlayClickSound();
	RemoveFromParent();
}

void UNameEntryWidget::PlayClickSound()
{
	if (ButtonClickSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), ButtonClickSound);
	}
}
