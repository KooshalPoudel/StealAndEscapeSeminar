/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: HUDWidget.h
Author: Kushal Poudel and Alok Poudel
Last Modified: April 18, 2026

Description: Gameplay HUD shown during levels. Displays three pieces of info
             in the top-right corner of the screen:
               1. Item collection progress  (Items: 1 / 3)
               2. Elapsed time              (Time: 01:23)
               3. Live score                (Score: 3940)

             All values are pulled from the StealAndEscapeGameMode every tick
             via NativeTick so the display stays in sync automatically. The
             score recalculates every frame so the player can watch the time
             bonus tick down as they play which motivates speed.
*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUDWidget.generated.h"

class UTextBlock;

UCLASS()
class STEALANDESCAPE_API UHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	// Displays "Items: 1 / 3"
	UPROPERTY(meta = (BindWidget))
		UTextBlock* Txt_Items;

	// Displays "Time: 01:23"
	UPROPERTY(meta = (BindWidget))
		UTextBlock* Txt_Time;

	// Displays "Score: 4000"
	UPROPERTY(meta = (BindWidget))
		UTextBlock* Txt_Score;

private:
	// Format seconds as MM:SS for clean display
	FString FormatTime(float TotalSeconds) const;
};
