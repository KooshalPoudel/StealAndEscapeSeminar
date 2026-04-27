/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
Template Used: Unreal Engine 4 TopDown C++ Template (Epic Games)
Original Template Author: Epic Games

File Name: StealAndEscapePlayerController.cpp
Modified By: Kushal Poudel and Alok Poudel
Last Modified: April 26, 2026

Description: StealAndEscapePlayerController is the player controller class which is based
			 on Unreal Engine's Top-Down C++ template provided by Epic Games. The template
			 generated functions like click-to-move , touch input , cursor tracing and
			 SetupInputComponent are kept as it is from the Epic Games template.

			 Custom features that we added on top of the template are:
			 - BeginPlay override to force input mode back to Game & UI when entering
			   gameplay from the main menu
			 - Pause Menu system using Escape key , including TogglePauseMenu , OpenPauseMenu
			   and ClosePauseMenu functions
			 - Escape key binding inside SetupInputComponent with bExecuteWhenPaused = true
			   so player can resume from pause menu

			 Each section below is marked wheather it is from Epic Games template or it is
			 our custom added code.
*/

#include "StealAndEscapePlayerController.h"
#include "PauseMenuWidget.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Runtime/Engine/Classes/Components/DecalComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "StealAndEscapeCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// [Epic Games Template] Default constructor from the topdown template 
AStealAndEscapePlayerController::AStealAndEscapePlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
}

/* [Custom Added by Kushal] BeginPlay is called when the controller is ready to accept input.
   We force input mode back to Game & UI here because when the player enters a gameplay
   level from the main menu , the UI-only input mode set by MainMenuPlayerController
   persists across the level transition and blocks the character input. Explicitly reseting
   it here gaurantees that click-to-move and WASD input works regardless of wheather the
   player launched straight into the level or came through the main menu.
*/
void AStealAndEscapePlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Reseting input mode so the character can recieve input from both mouse and keyboard
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);

	// Showing cursor since the top-down game uses click-to-move
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
}

// [Epic Games Template] PlayerTick from the topdown template , kept as it is
void AStealAndEscapePlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	// keep updating the destination every tick while desired
	if (bMoveToMouseCursor)
	{
		MoveToMouseCursor();
	}
}

/* [Epic Games Template + Custom] SetupInputComponent is mostly from the topdown template
   The SetDestination , Touch and ResetVR bindings are from Epic Games template.
   The Escape key binding for Pause Menu at the bottom is custom added by us -- kushal
*/
void AStealAndEscapePlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	// [Epic Games Template] click to move bindings 
	InputComponent->BindAction("SetDestination", IE_Pressed, this, &AStealAndEscapePlayerController::OnSetDestinationPressed);
	InputComponent->BindAction("SetDestination", IE_Released, this, &AStealAndEscapePlayerController::OnSetDestinationReleased);

	// [Epic Games Template] support touch devices 
	InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AStealAndEscapePlayerController::MoveToTouchLocation);
	InputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AStealAndEscapePlayerController::MoveToTouchLocation);

	// [Epic Games Template] VR reset binding
	InputComponent->BindAction("ResetVR", IE_Pressed, this, &AStealAndEscapePlayerController::OnResetVR);

	/* [Custom Added by Kushal] Binding Escape directly by FKey avoids needing the player
	   to configure an action mapping in Project Settings. bExecuteWhenPaused is required
	   here because the normal input path is blocked while the game is paused, without it
	   pressing Escape a second time would not resume the game.
	*/
	InputComponent->BindKey(EKeys::Escape, IE_Pressed, this,
		&AStealAndEscapePlayerController::TogglePauseMenu).bExecuteWhenPaused = true;
}

// [Epic Games Template] VR reset function from the topdown template 
void AStealAndEscapePlayerController::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

// [Epic Games Template] MoveToMouseCursor is from topdown template 
void AStealAndEscapePlayerController::MoveToMouseCursor()
{
	if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
	{
		if (AStealAndEscapeCharacter* MyPawn = Cast<AStealAndEscapeCharacter>(GetPawn()))
		{
			if (MyPawn->GetCursorToWorld())
			{
				UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, MyPawn->GetCursorToWorld()->GetComponentLocation());
			}
		}
	}
	else
	{
		// Trace to see what is under the mouse cursor
		FHitResult Hit;
		GetHitResultUnderCursor(ECC_Visibility, false, Hit);

		if (Hit.bBlockingHit)
		{
			// We hit something, move there
			SetNewMoveDestination(Hit.ImpactPoint);
		}
	}
}

// [Epic Games Template] MoveToTouchLocation is from topdown template , kept as it is
void AStealAndEscapePlayerController::MoveToTouchLocation(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	FVector2D ScreenSpaceLocation(Location);

	// Trace to see what is under the touch location
	FHitResult HitResult;
	GetHitResultAtScreenPosition(ScreenSpaceLocation, CurrentClickTraceChannel, true, HitResult);
	if (HitResult.bBlockingHit)
	{
		// We hit something, move there
		SetNewMoveDestination(HitResult.ImpactPoint);
	}
}

// [Epic Games Template] SetNewMoveDestination is from topdown template , kept as it is
void AStealAndEscapePlayerController::SetNewMoveDestination(const FVector DestLocation)
{
	APawn* const MyPawn = GetPawn();
	if (MyPawn)
	{
		float const Distance = FVector::Dist(DestLocation, MyPawn->GetActorLocation());

		// We need to issue move command only if far enough in order for walk animation to play correctly
		if ((Distance > 120.0f))
		{
			UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, DestLocation);
		}
	}
}

// [Epic Games Template] OnSetDestinationPressed from topdown template , kept as it is
void AStealAndEscapePlayerController::OnSetDestinationPressed()
{
	// set flag to keep updating destination until released
	bMoveToMouseCursor = true;
}

// [Epic Games Template] OnSetDestinationReleased from topdown template , kept as it is
void AStealAndEscapePlayerController::OnSetDestinationReleased()
{
	// clear flag to indicate we should stop updating the destination
	bMoveToMouseCursor = false;
}

/* 
   PAUSE MENU [Custom Added by Kushal]
   The whole pause menu system below is custom added by us , not from the Epic
   Games template. It handles opening and closing the pause menu , swaping the
   input modes and pausing/unpausing the game.
   */

   /* [Custom Added by Kushal] TogglePauseMenu is binded to the Escape key. If the pause
	  menu is already open it closes it , otherwise it opens one . Tracking the instance
	  pointer keeps the state simple , null means closed and valid means open.
   */
void AStealAndEscapePlayerController::TogglePauseMenu()
{
	if (PauseMenuInstance)
	{
		ClosePauseMenu();
	}
	else
	{
		OpenPauseMenu();
	}
}

/* [Custom Added by Kushal] OpenPauseMenu creates the pause widget , adds it to the viewport,
   then swaps to UI-only input mode so the clicks work on the paused game , and finaly
   pauses the world so guards stop moving while paused.
*/
void AStealAndEscapePlayerController::OpenPauseMenu()
{
	/* Safety check , if the widget class was never set on the Blueprint defaults
	   there is nothing we can show so we just return
	*/
	if (!PauseMenuWidgetClass)
	{
		return;
	}

	// Creating the pause menu widget instance from the class set in blueprint
	PauseMenuInstance = CreateWidget<UPauseMenuWidget>(this, PauseMenuWidgetClass);
	if (!PauseMenuInstance) return;

	// Adding the widget to viewport so it actually shows on screen 
	PauseMenuInstance->AddToViewport();

	/* Switching to UI-only input so button clicks recieve focus and keyboard
	   input doesnot accidently reach the game while it is paused
	*/
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(PauseMenuInstance->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	bShowMouseCursor = true;

	// Freezing the world so guards donot keep chasing during the pause menu
	UGameplayStatics::SetGamePaused(GetWorld(), true);
}

/* [Custom Added by Kushal] ClosePauseMenu unpauses the game , removes the widget from
   viewport and restores the gameplay input back to Game & UI mode . This is called from
   Escape-to-close and also from the Resume button on the pause menu widget.
*/
void AStealAndEscapePlayerController::ClosePauseMenu()
{
	// Removing the pause menu widget from viewport and clearing the instance pointer. 
	if (PauseMenuInstance)
	{
		PauseMenuInstance->RemoveFromParent();
		PauseMenuInstance = nullptr;
	}

	/* Unpausing before restoring input so any ticks that depend on unpause
	   happen in the right order
	*/
	UGameplayStatics::SetGamePaused(GetWorld(), false);

	// Restoring Game & UI input mode so the character can move again 
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
	bShowMouseCursor = true;
}