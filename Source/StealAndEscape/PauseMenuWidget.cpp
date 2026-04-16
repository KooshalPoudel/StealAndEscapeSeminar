/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: PauseMenuWidget.cpp
Author: Kushal Poudel and Alok Poudel
Last Modified: April 16, 2026

Description: Implementation of the in-game pause menu. Handles the Resume,
             Main Menu, and Quit button actions.

             Resume unpauses the game, removes this widget, and restores
             Game & UI input mode so the player can move again. Main Menu
             unpauses first before loading the menu level to avoid loading
             the next level in a paused state. Quit performs the same
             unpause-first pattern before issuing the quit console command.
*/

#include "PauseMenuWidget.h"
#include "StealAndEscapePlayerController.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

void UPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind each button to its handler. AddDynamic requires a UFUNCTION.
	if (Btn_Resume)   Btn_Resume->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnResumeClicked);
	if (Btn_MainMenu) Btn_MainMenu->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnMainMenuClicked);
	if (Btn_Quit)     Btn_Quit->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnQuitClicked);
}

void UPauseMenuWidget::OnResumeClicked()
{
	/* Ask the player controller to close the pause menu. Routing through the
	   controller instead of unpausing here directly keeps all pause state
	   logic in one place. That way if we ever add a save button or option
	   submenu to pause, the controller is the single source of truth. */
	APlayerController* PC = GetOwningPlayer();
	AStealAndEscapePlayerController* OurPC = Cast<AStealAndEscapePlayerController>(PC);
	if (OurPC)
	{
		OurPC->ClosePauseMenu();
	}
	else if (PC)
	{
		// Fallback: if for some reason the controller is not our custom type,
		// still unpause and remove the widget so the player is not stuck
		UGameplayStatics::SetGamePaused(GetWorld(), false);
		RemoveFromParent();
	}
}

void UPauseMenuWidget::OnMainMenuClicked()
{
	PrepareForLevelTransition();
	UGameplayStatics::OpenLevel(GetWorld(), MainMenuLevelName);
}

void UPauseMenuWidget::OnQuitClicked()
{
	// Unpause first so the quit command processes cleanly on all platforms
	UGameplayStatics::SetGamePaused(GetWorld(), false);

	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		PC->ConsoleCommand(TEXT("quit"));
	}
}

void UPauseMenuWidget::PrepareForLevelTransition()
{
	// Unpause before loading a new level so the destination level does not
	// load in a paused state which would freeze the new scene immediately
	UGameplayStatics::SetGamePaused(GetWorld(), false);

	// Remove this widget so it does not linger over the next level
	RemoveFromParent();
}
