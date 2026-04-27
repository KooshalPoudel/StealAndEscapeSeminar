/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar

File Name: MainMenuGameMode.cpp
Author: Kushal Poudel and Alok Poudel
Last Modified: April 26, 2026

Description: Implementation of the main-menu GameMode. Only job is to hand control to
			 MainMenuPlayerController which spawns the menu widget.
*/

#include "MainMenuGameMode.h"
#include "MainMenuPlayerController.h"

AMainMenuGameMode::AMainMenuGameMode()
{
	// Using our custom player controller that spawns the menu widget and switches input to UI only
	PlayerControllerClass = AMainMenuPlayerController::StaticClass();
	// No defult pawn needed on the menu screen
	DefaultPawnClass = nullptr;
}