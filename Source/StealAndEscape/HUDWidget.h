/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar

File Name: HUDWidget.h
Author: Kushal Poudel and Alok Poudel
Last Modified: April 26, 2026

Description: Header file for HUDWidget.cpp . This is the gameplay HUD shown during levels
			 which displays three pieces of info in the top-right corner of the screen:
			   1. Item collection progress ----->(Items:X/X)
			   2. Elapsed time-->(Time: XX:XX)
			   3. Live score ------->(Score: X)
			 All these values are pulled from the StealAndEscapeGameMode every tick using
			 NativeTick funtion so the display stays in sync automaticaly. The score is
			 recalulated every frame so the player can watch the time bonus tick down.
*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUDWidget.generated.h"

// This one Forward declaration so we donot need to include the full header here.
class UTextBlock;

UCLASS()
class STEALANDESCAPE_API UHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	//This is Called when widget is created , we use it to set inital text on each text block
	virtual void NativeConstruct() override;
	// This Called every frame , we use it to pull fresh values from GameMode and update text 
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	// Displays "Items: x:x"
	UPROPERTY(meta = (BindWidget))
		UTextBlock* Txt_Items;
	// Displays "Time: xx:xx"
	UPROPERTY(meta = (BindWidget))
		UTextBlock* Txt_Time;
	// Displays "Score: x"
	UPROPERTY(meta = (BindWidget))
		UTextBlock* Txt_Score;

private:
	// Helper that converts seconds into MM:SS string format for clean display. 
	FString FormatTime(float TotalSeconds) const;
};