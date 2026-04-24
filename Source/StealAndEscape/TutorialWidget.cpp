/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: TutorialWidget.cpp
Author: Kushal Poudel and Alok Poudel
Last Modified: April 24, 2026

Description: Implementation of the tutorial HUD message widget.
*/

#include "TutorialWidget.h"
#include "Components/TextBlock.h"

void UTutorialWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Txt_Message)
	{
		Txt_Message->SetText(FText::FromString(TEXT("")));
	}
}

void UTutorialWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bFlashing || !Txt_Message) return;

	FlashTimer += InDeltaTime;
	float Alpha = 0.7f + 0.3f * FMath::Sin(FlashTimer * 6.0f);

	FLinearColor CurrentColor = Txt_Message->ColorAndOpacity.GetSpecifiedColor();
	CurrentColor.A = Alpha;
	Txt_Message->SetColorAndOpacity(FSlateColor(CurrentColor));
}

void UTutorialWidget::SetMessage(const FString& NewMessage)
{
	if (Txt_Message)
	{
		Txt_Message->SetText(FText::FromString(NewMessage));
	}
}

void UTutorialWidget::SetFlashing(bool bShouldFlash)
{
	bFlashing = bShouldFlash;
	FlashTimer = 0.f;

	if (!bShouldFlash && Txt_Message)
	{
		FLinearColor CurrentColor = Txt_Message->ColorAndOpacity.GetSpecifiedColor();
		CurrentColor.A = 1.0f;
		Txt_Message->SetColorAndOpacity(FSlateColor(CurrentColor));
	}
}