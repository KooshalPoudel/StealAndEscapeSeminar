/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: StealAndEscapeGameMode.h
Author: Kushal Poudel and Alok Poudel
Last Modified: March 17, 2026

Description: This is the GameMode class which manages the overall game state.
It tracks win and lose conditions, item collection count, and controls
what happens when the player is caught by a guard or reaches the exit.
The GameMode is accessible from anywhere using GetWorld()->GetAuthGameMode()
which makes it the best place to store game-wide state like collected items
and whether the game is over.

Updated: Added win/lose condition logic, item collection tracking,
and game state management functions. Guards call OnPlayerCaught() when
they catch the player. The exit zone calls OnPlayerReachedExit() when
the player reaches the door. Items call OnItemCollected() when picked up.
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

	/* Called by GuardAIController when guard gets close enough to catch the player
	   This ends the game with a lose condition and disables player input
	*/
	UFUNCTION(BlueprintCallable, Category = "GameState")
		void OnPlayerCaught();

	/* Called by ExitZone when the player overlaps the exit trigger
	   This checks if all items are collected before allowing the win
	   If items are not collected it logs a message and does not end the game
	*/
	UFUNCTION(BlueprintCallable, Category = "GameState")
		void OnPlayerReachedExit();

	/* Called by StealableItem when the player picks up a collectible
	   This increments the collected count and logs the progress
	*/
	UFUNCTION(BlueprintCallable, Category = "GameState")
		void OnItemCollected();

	/* Checks whether the player has collected all required items
	   Used by the exit zone to decide if the door should let the player through
	*/
	UFUNCTION(BlueprintCallable, Category = "GameState")
		bool HasCollectedAllItems() const;

	/* Game State Variables */

	// Whether the game has ended either by win or lose to prevent repeated triggers
	UPROPERTY(BlueprintReadOnly, Category = "GameState")
		bool bIsGameOver = false;

	// How many items the player has collected so far during this level
	UPROPERTY(BlueprintReadOnly, Category = "GameState")
		int32 CollectedItems = 0;

	// How many items need to be collected before the exit unlocks - Set in level editor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameState")
		int32 RequiredItems = 1;
};