/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: MainMenuGameMode.h
Author: Kushal Poudel and Alok Poudel
Last Modified: April 15, 2026

Description: Dedicated GameMode for the main menu level. Kept separate from
             StealAndEscapeGameMode because the gameplay GameMode handles catch
             and item-collection logic which does not apply on the title screen.
             Assigns MainMenuPlayerController as the default controller.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MainMenuGameMode.generated.h"

UCLASS(minimalapi)
class AMainMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMainMenuGameMode();
};
