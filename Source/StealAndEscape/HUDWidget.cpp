/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: HUDWidget.cpp
Author: Kushal Poudel and Alok Poudel
Last Modified: April 18, 2026

Description: Implementation of the gameplay HUD. Reads item count, elapsed
             time, and live score from the GameMode every tick and updates
             its three text blocks positioned in the top-right corner.
*/

#include "HUDWidget.h"
#include "StealAndEscapeGameMode.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

void UHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Set initial display so it is not blank for the first frame
	if (Txt_Items) Txt_Items->SetText(FText::FromString(TEXT("Items: 0 / 0")));
	if (Txt_Time)  Txt_Time->SetText(FText::FromString(TEXT("Time: 00:00")));
	if (Txt_Score) Txt_Score->SetText(FText::FromString(TEXT("Score: 0")));
}

void UHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Pull fresh values from the GameMode each frame. Cast may fail on the
	// main menu level where a different GameMode runs - in that case we
	// just skip the update and leave the last values shown.
	AStealAndEscapeGameMode* GM = Cast<AStealAndEscapeGameMode>(
		UGameplayStatics::GetGameMode(GetWorld()));
	if (!GM) return;

	// Items counter
	if (Txt_Items)
	{
		FString ItemsStr = FString::Printf(TEXT("Items: %d / %d"),
			GM->CollectedItems, GM->RequiredItems);
		Txt_Items->SetText(FText::FromString(ItemsStr));
	}

	// Timer
	if (Txt_Time)
	{
		FString TimeStr = FString::Printf(TEXT("Time: %s"), *FormatTime(GM->ElapsedTime));
		Txt_Time->SetText(FText::FromString(TimeStr));
	}

	// Live score - recalculated every tick so the player can see the time
	// bonus ticking down as they play. Motivates speed!
	if (Txt_Score)
	{
		int32 CurrentScore = GM->CalculateScore();
		FString ScoreStr = FString::Printf(TEXT("Score: %d"), CurrentScore);
		Txt_Score->SetText(FText::FromString(ScoreStr));
	}
}

FString UHUDWidget::FormatTime(float TotalSeconds) const
{
	if (TotalSeconds < 0.f) TotalSeconds = 0.f;

	int32 Minutes = FMath::FloorToInt(TotalSeconds / 60.f);
	int32 Seconds = FMath::FloorToInt(TotalSeconds) % 60;
	return FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
}
