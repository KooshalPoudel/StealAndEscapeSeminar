/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: StealAndEscapeSaveGame.h
Author: Kushal Poudel and Alok Poudel
Last Modified: April 15, 2026

Description: Save game class that stores the player's progress.
             Currently stores which level the player was on and how many items
             they had collected. Used by the Load menu to resume gameplay.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "StealAndEscapeSaveGame.generated.h"

UCLASS()
class STEALANDESCAPE_API UStealAndEscapeSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// Name of the level that was active when the save was made
	UPROPERTY(VisibleAnywhere, Category = "SaveData")
		FName SavedLevelName;

	// How many items the player had collected at save time
	UPROPERTY(VisibleAnywhere, Category = "SaveData")
		int32 SavedCollectedItems = 0;

	// Human-readable label shown in the load menu (e.g. "Slot 1 - Level 2")
	UPROPERTY(VisibleAnywhere, Category = "SaveData")
		FString SaveDisplayName;

	// Date string of when the save was made so the player can tell saves apart
	UPROPERTY(VisibleAnywhere, Category = "SaveData")
		FString SaveDate;
};
