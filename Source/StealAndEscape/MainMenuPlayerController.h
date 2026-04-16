/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: MainMenuPlayerController.h
Author: Kushal Poudel and Alok Poudel
Last Modified: April 15, 2026

Description: Dedicated player controller for the main menu level. On BeginPlay
             it creates the MainMenuWidget, adds it to the viewport, shows the
             mouse cursor, and puts the input mode in UI-only so button clicks
             work without game-world input interfering.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainMenuPlayerController.generated.h"

class UMainMenuWidget;

UCLASS()
class STEALANDESCAPE_API AMainMenuPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMainMenuPlayerController();

protected:
	virtual void BeginPlay() override;

	// Widget class to spawn. Will be set to the UMG asset that inherits from
	// UMainMenuWidget in the editor through the Blueprint defaults panel
	// on the MainMenuPlayerController Blueprint or by setting it directly
	// in code below. We expose it as EditDefaultsOnly so a designer can swap
	// the widget class without recompiling.
	UPROPERTY(EditDefaultsOnly, Category = "UI")
		TSubclassOf<UMainMenuWidget> MainMenuWidgetClass;

private:
	// Keep a pointer to the spawned widget so it is not garbage collected
	UPROPERTY()
		UMainMenuWidget* MainMenuWidgetInstance = nullptr;
};
