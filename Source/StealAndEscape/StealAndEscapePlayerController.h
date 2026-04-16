// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "StealAndEscapePlayerController.generated.h"

class UPauseMenuWidget;

UCLASS()
class AStealAndEscapePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AStealAndEscapePlayerController();

	/* Closes the pause menu. Called by the PauseMenuWidget's Resume button.
	   Removes the widget, unpauses the game, and restores Game & UI input.
	   Public so the widget can call it. */
	UFUNCTION(BlueprintCallable, Category = "Pause")
		void ClosePauseMenu();

	/* Widget Blueprint class for the pause menu. Set this on the
	   BP_StealAndEscapePlayerController defaults to WBP_PauseMenu. If left
	   null Escape will do nothing. */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
		TSubclassOf<UPauseMenuWidget> PauseMenuWidgetClass;

protected:
	/** True if the controlled character should navigate to the mouse cursor. */
	uint32 bMoveToMouseCursor : 1;

	// Begin PlayerController interface
	virtual void BeginPlay() override;
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;
	// End PlayerController interface

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Navigate player to the current mouse cursor location. */
	void MoveToMouseCursor();

	/** Navigate player to the current touch location. */
	void MoveToTouchLocation(const ETouchIndex::Type FingerIndex, const FVector Location);
	
	/** Navigate player to the given world location. */
	void SetNewMoveDestination(const FVector DestLocation);

	/** Input handlers for SetDestination action. */
	void OnSetDestinationPressed();
	void OnSetDestinationReleased();

	/* Toggle pause menu when Escape is pressed. Opens it if closed, closes
	   it if open. */
	void TogglePauseMenu();

private:
	/* Keep a pointer to the spawned pause menu so it can be removed on resume
	   and is not garbage collected while the game is paused. Null when the
	   menu is not shown. */
	UPROPERTY()
		UPauseMenuWidget* PauseMenuInstance = nullptr;

	/* Helper that creates the widget, adds it to the viewport, switches to
	   UI-only input mode, and pauses the game. */
	void OpenPauseMenu();
};
