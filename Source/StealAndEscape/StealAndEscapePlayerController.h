/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
Template Used: Unreal Engine 4 TopDown C++ Template (Epic Games)
Original Template Author: Epic Games

File Name: StealAndEscapePlayerController.h
Modified By: Kushal Poudel and Alok Poudel
Last Modified: April 26, 2026

Description: Header file for StealAndEscapePlayerController.cpp. Most of this is from
			 Epic Games topdown template , and we added the pause menu system on top of it.
			 Each section below is marked wheather it is from Epic Games template or it
			 is custom added by us.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "StealAndEscapePlayerController.generated.h"

// Forward declaration so we donot need to include the full header here
class UPauseMenuWidget;

UCLASS()
class AStealAndEscapePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AStealAndEscapePlayerController();

	/* [Custom Added by Kushal] Closes the pause menu. This is called by the PauseMenuWidget's
	   Resume button. It removes the widget , unpauses the game and restores Game & UI input.
	   Public so the widget can call it from blueprint.
	*/
	UFUNCTION(BlueprintCallable, Category = "Pause")
		void ClosePauseMenu();

	/* [Custom Added by Kushal] Gate for movement input (click-to-move) without disabling the
	   whole input component. The tutorial uses this to freeze the player durring step
	   messages while keeping the Escape key binding alive for the pause menu . If we used
	   DisableInput on the controller instead , it would strip the Escape binding off the
	   input stack and ESC would silently do nothing in the tutorial.
	*/
	UFUNCTION(BlueprintCallable, Category = "Input")
		void SetMovementInputEnabled(bool bEnabled);

	/* [Custom Added by Kushal] Widget Blueprint class for the pause menu . Set this on the
	   BP_StealAndEscapePlayerController defaults to WBP_PauseMenu . If left null Escape
	   key will do nothing.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "UI")
		TSubclassOf<UPauseMenuWidget> PauseMenuWidgetClass;

protected:
	/** [Epic Games Template] True if the controlled character should navigate to the mouse cursor. */
	uint32 bMoveToMouseCursor : 1;

	// Begin PlayerController interface

	/* [Custom Added by Kushal] BeginPlay override to reset input mode back to Game & UI when
	   entering gameplay from the main menu
	*/
	virtual void BeginPlay() override;

	// [Epic Games Template] PlayerTick from the topdown template
	virtual void PlayerTick(float DeltaTime) override;

	/* [Epic Games Template + Custom] SetupInputComponent is mostly from epic games template
	   we added our Escape key binding for pause menu inside it -- kushal
	*/
	virtual void SetupInputComponent() override;

	// End PlayerController interface

	/** [Epic Games Template] Resets HMD orientation in VR. */
	void OnResetVR();

	/** [Epic Games Template] Navigate player to the current mouse cursor location. */
	void MoveToMouseCursor();

	/** [Epic Games Template] Navigate player to the current touch location. */
	void MoveToTouchLocation(const ETouchIndex::Type FingerIndex, const FVector Location);

	/** [Epic Games Template] Navigate player to the given world location. */
	void SetNewMoveDestination(const FVector DestLocation);

	/** [Epic Games Template] Input handlers for SetDestination action. */
	void OnSetDestinationPressed();
	void OnSetDestinationReleased();

	/* [Custom Added by Kushal] Toggle the pause menu when Escape is pressed. Opens it if
	   closed and closes it if its already open.
	*/
	void TogglePauseMenu();

private:
	/* [Custom Added by Kushal] When false , click-to-move is gated off in the input
	   handlers and PlayerTick . Defaults to true . Toggled by SetMovementInputEnabled.
	   This is separate from DisableInput because we want movement off but ESC on.
	*/
	bool bMovementInputEnabled = true;

	/* [Custom Added by Kushal] Keeping a pointer to the spawned pause menu so that it can be
	   removed on resume and also so it is not garbage collected while the game is paused.
	   This is null when the menu is not shown.
	*/
	UPROPERTY()
		UPauseMenuWidget* PauseMenuInstance = nullptr;

	/* [Custom Added by Kushal] Helper funtion that creates the pause widget , adds it to the
	   viewport, switches to UI-only input mode and pauses the game.
	*/
	void OpenPauseMenu();
};