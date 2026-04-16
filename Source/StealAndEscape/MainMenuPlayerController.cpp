/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: MainMenuPlayerController.cpp
Author: Kushal Poudel and Alok Poudel
Last Modified: April 15, 2026

Description: Implementation of the main-menu player controller. Spawns the main
             menu widget and sets up UI input mode so clicking buttons works.
*/

#include "MainMenuPlayerController.h"
#include "MainMenuWidget.h"
#include "Blueprint/UserWidget.h"

AMainMenuPlayerController::AMainMenuPlayerController()
{
	// Cursor is needed for the player to click menu buttons
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
}

void AMainMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Safety check so we do not crash if the widget class was never assigned
	if (!MainMenuWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("MainMenuPlayerController - MainMenuWidgetClass not set! Assign it on the PlayerController Blueprint defaults."));
		return;
	}

	// Create the widget and add it to the viewport so it shows on screen
	MainMenuWidgetInstance = CreateWidget<UMainMenuWidget>(this, MainMenuWidgetClass);
	if (MainMenuWidgetInstance)
	{
		MainMenuWidgetInstance->AddToViewport();

		// Set input mode to UI only so button clicks work and no game input leaks in
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(MainMenuWidgetInstance->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
	}
}
