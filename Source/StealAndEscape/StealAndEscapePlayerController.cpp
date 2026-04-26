// Copyright Epic Games, Inc. All Rights Reserved.

#include "StealAndEscapePlayerController.h"
#include "PauseMenuWidget.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Runtime/Engine/Classes/Components/DecalComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "StealAndEscapeCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AStealAndEscapePlayerController::AStealAndEscapePlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
}

/* BeginPlay is called when the controller is ready to accept input.
   We force input mode back to Game & UI here because when the player enters
   a gameplay level from the main menu, the UI-only input mode set by
   MainMenuPlayerController persists across the level transition and blocks
   character input. Explicitly resetting it guarantees click-to-move and WASD
   input works regardless of whether the player launched straight into the
   level or came through the main menu.
*/
void AStealAndEscapePlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Reset input mode so the character can receive input from both mouse and keyboard
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);

	// Show cursor since the top-down game uses click-to-move
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
}

void AStealAndEscapePlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	// keep updating the destination every tick while desired
	if (bMoveToMouseCursor)
	{
		MoveToMouseCursor();
	}
}

void AStealAndEscapePlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	InputComponent->BindAction("SetDestination", IE_Pressed, this, &AStealAndEscapePlayerController::OnSetDestinationPressed);
	InputComponent->BindAction("SetDestination", IE_Released, this, &AStealAndEscapePlayerController::OnSetDestinationReleased);

	// support touch devices 
	InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AStealAndEscapePlayerController::MoveToTouchLocation);
	InputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AStealAndEscapePlayerController::MoveToTouchLocation);

	InputComponent->BindAction("ResetVR", IE_Pressed, this, &AStealAndEscapePlayerController::OnResetVR);

	/* Binding Escape directly by FKey avoids needing the player to configure
	   an action mapping in Project Settings. bExecuteWhenPaused is required
	   because the normal input path is blocked while the game is paused -
	   without it pressing Escape a second time would not resume the game. */
	InputComponent->BindKey(EKeys::Escape, IE_Pressed, this,
		&AStealAndEscapePlayerController::TogglePauseMenu).bExecuteWhenPaused = true;
}

void AStealAndEscapePlayerController::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

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

void AStealAndEscapePlayerController::OnSetDestinationPressed()
{
	// set flag to keep updating destination until released
	bMoveToMouseCursor = true;
}

void AStealAndEscapePlayerController::OnSetDestinationReleased()
{
	// clear flag to indicate we should stop updating the destination
	bMoveToMouseCursor = false;
}

/* =============================================================================
   PAUSE MENU
   ============================================================================= */

/* TogglePauseMenu is bound to the Escape key. If the pause menu is already
   up it closes it, otherwise it opens one. Tracking the instance pointer
   keeps state simple - null means closed, valid means open.
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

/* OpenPauseMenu creates the widget, adds it to the viewport, swaps to UI-only
   input mode so clicks work on the paused game, and pauses the world.
*/
void AStealAndEscapePlayerController::OpenPauseMenu()
{
	// Safety check - if the widget class was never set on the Blueprint
	// defaults there is nothing we can show
	if (!PauseMenuWidgetClass)
	{
		return;
	}

	PauseMenuInstance = CreateWidget<UPauseMenuWidget>(this, PauseMenuWidgetClass);
	if (!PauseMenuInstance) return;

	PauseMenuInstance->AddToViewport();

	// Switch to UI-only input so button clicks receive focus and keyboard
	// input does not accidentally reach the game while paused
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(PauseMenuInstance->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	bShowMouseCursor = true;

	// Freeze the world so guards do not keep chasing during the pause menu
	UGameplayStatics::SetGamePaused(GetWorld(), true);
}

/* ClosePauseMenu unpauses, removes the widget, and restores gameplay input.
   Called from Escape-to-close and from the Resume button on the widget.
*/
void AStealAndEscapePlayerController::ClosePauseMenu()
{
	if (PauseMenuInstance)
	{
		PauseMenuInstance->RemoveFromParent();
		PauseMenuInstance = nullptr;
	}

	// Unpause before restoring input so any ticks that depend on unpause
	// happen in the right order
	UGameplayStatics::SetGamePaused(GetWorld(), false);

	// Restore Game & UI input mode so the character can move again
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
	bShowMouseCursor = true;
}
