/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: MainMenuGameMode.cpp
Author: Kushal Poudel and Alok Poudel
Last Modified: April 15, 2026

Description: Implementation of the main-menu GameMode. Only job is to hand
             control to MainMenuPlayerController which spawns the menu widget.
*/

#include "MainMenuGameMode.h"
#include "MainMenuPlayerController.h"

AMainMenuGameMode::AMainMenuGameMode()
{
	// Using our custom player controller that spawns the main menu widget and
	// switches input mode to UI only. No default pawn is needed on the menu.
	PlayerControllerClass = AMainMenuPlayerController::StaticClass();
	DefaultPawnClass = nullptr;
}
