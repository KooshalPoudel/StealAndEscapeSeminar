/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: StealAndEscapeGameMode.h
Author: Kushal Poudel and Alok Poudel
Last Modified: April 19, 2026

Description: Header for gameplay GameMode. Added item pickup, win, and lose
             sound properties. Set these in BP_StealAndEscapeGameMode Class
             Defaults under the Audio category.
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

	UFUNCTION(BlueprintCallable, Category = "GameState")
		virtual void OnPlayerCaught();

	UFUNCTION(BlueprintCallable, Category = "GameState")
		void OnPlayerReachedExit();

	UFUNCTION(BlueprintCallable, Category = "GameState")
		virtual void OnItemCollected();

	UFUNCTION(BlueprintCallable, Category = "GameState")
		bool HasCollectedAllItems() const;

	UFUNCTION(BlueprintCallable, Category = "GameState")
		int32 CalculateScore() const;

	/* Game State Variables */

	UPROPERTY(BlueprintReadOnly, Category = "GameState")
		bool bIsGameOver = false;

	UPROPERTY(BlueprintReadOnly, Category = "GameState")
		int32 CollectedItems = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameState")
		int32 RequiredItems = 1;

	UPROPERTY(BlueprintReadOnly, Category = "GameState")
		float ElapsedTime = 0.f;

	/* UI Widget Classes */

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
	UPROPERTY()
		UEndScreenWidget* EndScreenInstance = nullptr;

	UPROPERTY()
		UHUDWidget* HUDInstance = nullptr;

	UEndScreenWidget* SpawnEndScreen();
	void SpawnHUD();
};
