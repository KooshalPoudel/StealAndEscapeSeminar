/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: TutorialWidget.h
Author: Kushal Poudel and Alok Poudel
Last Modified: April 24, 2026

Description: UMG widget for the tutorial HUD message. The Blueprint child
WBP_TutorialWidget needs a TextBlock named exactly Txt_Message which this
class writes to via SetMessage. When flashing is enabled the text alpha
pulses so the final exit message stands out visually.
*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TutorialWidget.generated.h"

class UTextBlock;

UCLASS()
class STEALANDESCAPE_API UTutorialWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
		void SetMessage(const FString& NewMessage);

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
		void SetFlashing(bool bShouldFlash);

protected:
	UPROPERTY(meta = (BindWidget))
		UTextBlock* Txt_Message;

private:
	bool bFlashing = false;
	float FlashTimer = 0.f;
};