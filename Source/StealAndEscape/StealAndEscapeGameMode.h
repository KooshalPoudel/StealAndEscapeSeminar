/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar

File Name: StealAndEscapeGameMode.h
Author: Kushal Poudel and Alok Poudel
Last Modified: April 26, 2026

Description: This is header fiile for StealAndEscapeGameMode.cpp . Now we have added
			 item pickup , win and lose sound propertys and also HUD and EndScreen widget
			 classes. These all are set in BP_StealAndEscapeGameMode Class Defaults under
			 the Audio and UI catagory.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "StealAndEscapeGameMode.generated.h"

// Forward declarations so we donot need to include the full headers here 
class UEndScreenWidget;
class UHUDWidget;

UCLASS(minimalapi)
class AStealAndEscapeGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AStealAndEscapeGameMode();

	virtual void BeginPlay() override;

	// Tick is used to count up the elapsed time every frame for the timer
	virtual void Tick(float DeltaTime) override;

	/* This is Called from GuardAIController when guard gets close enough to catch the player
	*/
	UFUNCTION(BlueprintCallable, Category = "GameState")
		virtual void OnPlayerCaught();

	/* This is Called by ExitZone when the player overlaps the exit trigger
	*/
	UFUNCTION(BlueprintCallable, Category = "GameState")
		void OnPlayerReachedExit();

	/* This is Called by StealableItem when the player picks up a collectible
	*/
	UFUNCTION(BlueprintCallable, Category = "GameState")
		virtual void OnItemCollected();

	/*This  Checks whether the player has collected all required items
	  This is used by exit zone.
	*/
	UFUNCTION(BlueprintCallable, Category = "GameState")
		bool HasCollectedAllItems() const;

	// this calulates the final score from items collected and time taken
	UFUNCTION(BlueprintCallable, Category = "GameState")
		int32 CalculateScore() const;


	/*                           These are our Game State Variables                       */

	// Whether the game has ended either by win or lose to prevent repeated triggers
	UPROPERTY(BlueprintReadOnly, Category = "GameState")
		bool bIsGameOver = false;

	// How many items the player has collected.
	UPROPERTY(BlueprintReadOnly, Category = "GameState")
		int32 CollectedItems = 0;

	// How many items needs to be collected before the exit unlocks 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameState")
		int32 RequiredItems = 1;

	// this tracks how long player has been playing , used for score calulation and timer 
	UPROPERTY(BlueprintReadOnly, Category = "GameState")
		float ElapsedTime = 0.f;


	/* UI Widget Classes which is set in the blueprint for more control so that we can customize it  */

	UPROPERTY(EditDefaultsOnly, Category = "UI")
		TSubclassOf<UEndScreenWidget> EndScreenWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
		TSubclassOf<UHUDWidget> HUDWidgetClass;


	/* Audio - set in BP_StealAndEscapeGameMode Class Defaults */

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
		USoundBase* ItemPickupSound;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
		USoundBase* WinSound;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
		USoundBase* LoseSound;

private:
	//these are live instances of the widgets that are spawned , kept here so we can remove them later
	UPROPERTY()
		UEndScreenWidget* EndScreenInstance = nullptr;

	UPROPERTY()
		UHUDWidget* HUDInstance = nullptr;

	//helper funtions which spawns the HUD and end screen widgets and adds them to viewport 
	UEndScreenWidget* SpawnEndScreen();
	void SpawnHUD();
};