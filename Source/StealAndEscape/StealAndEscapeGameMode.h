/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: StealAndEscapeGameMode.h
Author: Kushal Poudel and Alok Poudel
Last Modified: March 22, 2026

Description: This is header fiile for StealAndEscapeGameMode.cpp
*/

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "StealAndEscapeGameMode.generated.h"

UCLASS(minimalapi)
class AStealAndEscapeGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AStealAndEscapeGameMode();

	/* This is Called from GuardAIController when guard gets close enough to catch the player
	*/
	UFUNCTION(BlueprintCallable, Category = "GameState")
		void OnPlayerCaught();

	/* This is Called by ExitZone when the player overlaps the exit trigger
	*/
	UFUNCTION(BlueprintCallable, Category = "GameState")
		void OnPlayerReachedExit();

	/* This is Called by StealableItem when the player picks up a collectible
	*/
	UFUNCTION(BlueprintCallable, Category = "GameState")
		void OnItemCollected();

	/*This  Checks whether the player has collected all required items
	  This is used by exit zone.
	*/
	UFUNCTION(BlueprintCallable, Category = "GameState")
		bool HasCollectedAllItems() const;

	/* These are ourGame State Variables */

	// Whether the game has ended either by win or lose to prevent repeated triggers
	UPROPERTY(BlueprintReadOnly, Category = "GameState")
		bool bIsGameOver = false;

	// How many items the player has collected
	UPROPERTY(BlueprintReadOnly, Category = "GameState")
		int32 CollectedItems = 0;

	// How many items need to be collected before the exit unlocks 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameState")
		int32 RequiredItems = 1;
};