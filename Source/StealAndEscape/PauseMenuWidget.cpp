/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: PauseMenuWidget.cpp
Author: Kushal Poudel and Alok Poudel
Last Modified: April 19, 2026
*/

#include "PauseMenuWidget.h"
#include "StealAndEscapePlayerController.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

void UPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Resume)   Btn_Resume->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnResumeClicked);
	if (Btn_MainMenu) Btn_MainMenu->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnMainMenuClicked);
	if (Btn_Quit)     Btn_Quit->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnQuitClicked);
}

void UPauseMenuWidget::OnResumeClicked()
{
	PlayClickSound();
	APlayerController* PC = GetOwningPlayer();
	AStealAndEscapePlayerController* OurPC = Cast<AStealAndEscapePlayerController>(PC);
	if (OurPC)
	{
		OurPC->ClosePauseMenu();
	}
	else if (PC)
	{
		UGameplayStatics::SetGamePaused(GetWorld(), false);
		RemoveFromParent();
	}
}

void UPauseMenuWidget::OnMainMenuClicked()
{
	PlayClickSound();
	PrepareForLevelTransition();
	UGameplayStatics::OpenLevel(GetWorld(), MainMenuLevelName);
}

void UPauseMenuWidget::OnQuitClicked()
{
	PlayClickSound();
	UGameplayStatics::SetGamePaused(GetWorld(), false);
	APlayerController* PC = GetOwningPlayer();
	if (PC) PC->ConsoleCommand(TEXT("quit"));
}

void UPauseMenuWidget::PrepareForLevelTransition()
{
	UGameplayStatics::SetGamePaused(GetWorld(), false);
	RemoveFromParent();
}

void UPauseMenuWidget::PlayClickSound()
{
	if (ButtonClickSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), ButtonClickSound);
	}
}
