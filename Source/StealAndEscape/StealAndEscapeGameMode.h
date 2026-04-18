/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: StealAndEscapeGameMode.h
Author: Kushal Poudel and Alok Poudel
Last Modified: April 18, 2026

Description: This is header file for StealAndEscapeGameMode.cpp
             Added in this revision:
             - ElapsedTime tracking (ticks during gameplay, frozen on win/lose)
             - CalculateScore() function using the scoring formula
             - HUDWidgetClass property and HUD spawn on BeginPlay
             EndScreenWidget calls remain unchanged (2-arg signatures).
*/

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "StealAndEscapeGameMode.generated.h"

class UEndScreenWidget;
class UHUDWidget;

UCLASS(minimalapi)
class AStealAndEscapeGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AStealAndEscapeGameMode();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/* Called from GuardAIController when guard catches the player */
	UFUNCTION(BlueprintCallable, Category = "GameState")
		void OnPlayerCaught();

	/* Called by ExitZone when the player overlaps the exit trigger */
	UFUNCTION(BlueprintCallable, Category = "GameState")
		void OnPlayerReachedExit();

	/* Called by StealableItem when the player picks up a collectible */
	UFUNCTION(BlueprintCallable, Category = "GameState")
		void OnItemCollected();

	/* Checks whether the player has collected all required items */
	UFUNCTION(BlueprintCallable, Category = "GameState")
		bool HasCollectedAllItems() const;

	/* Calculate the final score from current items + elapsed time.
	   Formula: (items * 1000) + time bonus
	   Time bonus: 1000 if time <= 60s, otherwise max(0, 1000 - (time-60)*2)
	   Public so the HUD widget can call it each tick for live display. */
	UFUNCTION(BlueprintCallable, Category = "GameState")
		int32 CalculateScore() const;

	/* Game State Variables */

	UPROPERTY(BlueprintReadOnly, Category = "GameState")
		bool bIsGameOver = false;

	UPROPERTY(BlueprintReadOnly, Category = "GameState")
		int32 CollectedItems = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameState")
		int32 RequiredItems = 1;

	/* Elapsed gameplay time in seconds. Incremented during Tick while the
	   game is not over. Used by HUD display and final score calculation. */
	UPROPERTY(BlueprintReadOnly, Category = "GameState")
		float ElapsedTime = 0.f;

	/* Widget Blueprint class spawned when the game ends */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
		TSubclassOf<UEndScreenWidget> EndScreenWidgetClass;

	/* Widget Blueprint class spawned on BeginPlay as the gameplay HUD */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
		TSubclassOf<UHUDWidget> HUDWidgetClass;

private:
	UPROPERTY()
		UEndScreenWidget* EndScreenInstance = nullptr;

	UPROPERTY()
		UHUDWidget* HUDInstance = nullptr;

	UEndScreenWidget* SpawnEndScreen();
	void SpawnHUD();
};
