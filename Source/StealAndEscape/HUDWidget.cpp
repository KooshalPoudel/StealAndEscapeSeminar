/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar

File Name: HUDWidget.cpp
Author: Kushal Poudel and Alok Poudel
Last Modified: April 26, 2026

Description: Implementation of the gameplay HUD widget . This reads the item count , elapsed
			 time and live score from the GameMode every tick and updates its three text blocks
			 which are positioned in the top-right corner of the screen . The HUD is spawned
			 by the GameMode at BeginPlay and is removed when the game ends so the end screen
			 can take over.
*/

#include "HUDWidget.h"
#include "StealAndEscapeGameMode.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

/* NativeConstruct is called when the widget is created and ready. We use it to set
   the initial text on each of the three text blocks so they are not blank on the first
   frame before NativeTick has a chance to update them with real values from GameMode.
*/
void UHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	// Setting initial display so it is not blank for the first frame!
	if (Txt_Items) Txt_Items->SetText(FText::FromString(TEXT("Items: 0 / 0")));
	if (Txt_Time)  Txt_Time->SetText(FText::FromString(TEXT("Time: 00:00")));
	if (Txt_Score) Txt_Score->SetText(FText::FromString(TEXT("Score: 0")));
}

/* NativeTick is called every frame by the engine .We use it to pull fresh values
   from the GameMode and update the HUD text blocks . 
   
   The cast to GameMode was failling  on main menu level where a diffrent GameMode is running , 
   FOr that case we just skip
   the update and leave the last values shown so that it doesnt crash.
*/
void UHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	/* Pulling fresh values from the GameMode each frame . Cast may fail on main menu
	   level where a diffrent GameMode runs , in that case we just skip the update
	*/
	AStealAndEscapeGameMode* GM = Cast<AStealAndEscapeGameMode>(
		UGameplayStatics::GetGameMode(GetWorld()));
	if (!GM) return;
	// THis is Items counter , showing how many collected out of total required
	if (Txt_Items)
	{
		FString ItemsStr = FString::Printf(TEXT("Items: %d / %d"),
			GM->CollectedItems, GM->RequiredItems);
		Txt_Items->SetText(FText::FromString(ItemsStr));
	}
	// Timer , showing elapsed time in MM:SS format using FormatTime helper
	if (Txt_Time)
	{
		FString TimeStr = FString::Printf(TEXT("Time: %s"), *FormatTime(GM->ElapsedTime));
		Txt_Time->SetText(FText::FromString(TimeStr));
	}
	/* Live score is recalulated every tick so the player can see the time bonus
	   ticking down as they play . This motivates the player to be fast !
	*/
	if (Txt_Score)
	{
		int32 CurrentScore = GM->CalculateScore();
		FString ScoreStr = FString::Printf(TEXT("Score: %d"), CurrentScore);
		Txt_Score->SetText(FText::FromString(ScoreStr));
	}
}

/* FormatTime is a helper funtion which converts seconds into MM:SS string format .
   We use FloorToInt to get whole minutes and seconds , and modulo 60 makes sure the
   seconds part stays between 0 and 59 . The %02d format makes sure each part is
   always 2 digits like 01:05 instead of 1:5.
*/
FString UHUDWidget::FormatTime(float TotalSeconds) const
{
	//if seconds is negative we just set it to zero
	if (TotalSeconds < 0.f) TotalSeconds = 0.f;
	// Getting whole minutes by dividing by 60 and flooring 
	int32 Minutes = FMath::FloorToInt(TotalSeconds / 60.f);
	// Getting remainder seconds using % 60
	int32 Seconds = FMath::FloorToInt(TotalSeconds) % 60;
	// Formating as MM:SS with leading zeros
	return FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
}