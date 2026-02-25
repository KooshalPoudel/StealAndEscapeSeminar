// Copyright Epic Games, Inc. All Rights Reserved.

#include "StealAndEscapeGameMode.h"
#include "StealAndEscapePlayerController.h"
#include "StealAndEscapeCharacter.h"
#include "UObject/ConstructorHelpers.h"

AStealAndEscapeGameMode::AStealAndEscapeGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = AStealAndEscapePlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDownCPP/Blueprints/TopDownCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}