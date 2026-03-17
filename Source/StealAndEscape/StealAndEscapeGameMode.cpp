/*
Project Name: Steal and Escape
Description: A 3D top-down stealth / escape game developed in Unreal Engine 4.27
Course: CSCI 491 Seminar
File Name: StealAndEscapeGameMode.cpp
Author: Kushal Poudel and Alok Poudel
Last Modified: March 17, 2026

Description: Implementation of the GameMode which manages win and lose conditions.
OnPlayerCaught() is called by GuardAIController when a guard catches the player.
OnPlayerReachedExit() is called by the ExitZone actor when the player reaches the door.
OnItemCollected() is called by StealableItem actors when the player picks them up.
The game ends by disabling player input, stopping character movement immediately,
and pausing the game so all actors freeze. Later UMG widgets will be shown for
win and lose screens.

Updated: Added full implementation for game state management including
lose condition (guard catches player), win condition (player escapes with all items),
and item collection tracking with progress logging.

Updated: Improved game-over handling. Now disables input on the controller directly
using PC->DisableInput(PC), calls StopMovementImmediately() on the character movement
component to prevent sliding, and calls SetGamePaused() to freeze all actors.

Updated: Added on-screen debug messages using GEngine->AddOnScreenDebugMessage
so win, lose, item pickup, and exit blocked feedback are visible during gameplay.
These are temporary debug messages that will be replaced with UMG widgets later.
*/

#include "StealAndEscapeGameMode.h"
#include "StealAndEscapePlayerController.h"
#include "StealAndEscapeCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
// Including CharacterMovementComponent to call StopMovementImmediately when game ends
#include "GameFramework/CharacterMovementComponent.h"
// Including Engine.h for GEngine->AddOnScreenDebugMessage to show on-screen text during gameplay
#include "Engine/Engine.h"

AStealAndEscapeGameMode::AStealAndEscapeGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = AStealAndEscapePlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDownCPP/Blueprints/TopDownCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

/* Lose Condition - Called by GuardAIController when guard catches the player
   First checks if the game is already over to avoid triggering multiple times
   Disables player input and stops character movement immediately so the player freezes
   Then pauses the game so guards and all other actors also stop moving
   Later this will show a lose screen UMG widget
*/
void AStealAndEscapeGameMode::OnPlayerCaught()
{
	// If game is already over do not trigger again
	if (bIsGameOver) return;

	bIsGameOver = true;

	UE_LOG(LogTemp, Warning, TEXT("GAME OVER - Player was caught by guard!"));

	/* Showing on-screen debug message so the player can see the result during gameplay
	   Key -1 means it does not replace a previous message with the same key
	   5.0f is how long the message stays on screen in seconds
	   FColor::Red makes it stand out as a lose message
	*/
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("GAME OVER - You were caught!"));
	}

	// Getting player controller to disable input so player cannot move after being caught
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		// Disabling input on the controller itself so no input reaches the character at all
		PC->DisableInput(PC);

		// Also stopping character movement immediately so the character does not slide or drift
		ACharacter* PlayerChar = Cast<ACharacter>(PC->GetPawn());
		if (PlayerChar && PlayerChar->GetCharacterMovement())
		{
			PlayerChar->GetCharacterMovement()->StopMovementImmediately();
		}
	}

	// Pausing the game so guards and all other actors stop moving too
	// This makes sure nothing keeps running in the background after game over
	UGameplayStatics::SetGamePaused(GetWorld(), true);

	// TODO: Show lose screen UMG widget here later
}

/* Win Condition - Called by ExitZone when player overlaps the exit trigger
   First checks if game is already over then checks if all items are collected
   If items are not all collected it logs a message and does not end the game
   If all items are collected it stops movement, disables input, pauses the game
   Later this will show a win screen UMG widget
*/
void AStealAndEscapeGameMode::OnPlayerReachedExit()
{
	// If game is already over do not trigger again
	if (bIsGameOver) return;

	// Check if player has collected all required items before allowing escape
	if (!HasCollectedAllItems())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot exit - Collect all items first! (%d / %d)"), CollectedItems, RequiredItems);

		/* Showing on-screen debug message so the player knows they need more items
		   Key 1 means if the player touches the exit again it replaces the old message
		   instead of stacking multiple messages on screen
		   3.0f seconds is enough time to read the message before it fades
		   FColor::Yellow makes it a warning color not a fail color
		*/
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1, 3.0f, FColor::Yellow,
				FString::Printf(TEXT("Collect all items before escaping! (%d / %d)"), CollectedItems, RequiredItems));
		}
		return;
	}

	bIsGameOver = true;

	UE_LOG(LogTemp, Warning, TEXT("YOU WIN - Escaped with all items!"));

	/* Showing on-screen debug message so the player can see the win result
	   Key -1 means unique message that does not replace other messages
	   5.0f seconds stays on screen long enough for the player to read
	   FColor::Green makes it obvious that this is a win message
	*/
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("YOU WIN - Escaped with all items!"));
	}

	// Getting player controller to disable input after winning
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		// Disabling input on the controller itself so no input reaches the character
		PC->DisableInput(PC);

		// Also stopping character movement immediately so the character does not slide
		ACharacter* PlayerChar = Cast<ACharacter>(PC->GetPawn());
		if (PlayerChar && PlayerChar->GetCharacterMovement())
		{
			PlayerChar->GetCharacterMovement()->StopMovementImmediately();
		}
	}

	// Pausing the game so everything freezes on the win screen
	UGameplayStatics::SetGamePaused(GetWorld(), true);

	// TODO: Show win screen UMG widget here later
}

/* Item Collection - Called by StealableItem when the player picks up an item
   Increments the collected count and logs the current progress
   This count is checked by OnPlayerReachedExit to decide if the player can escape
*/
void AStealAndEscapeGameMode::OnItemCollected()
{
	CollectedItems++;
	UE_LOG(LogTemp, Warning, TEXT("Item collected: %d / %d"), CollectedItems, RequiredItems);

	/* Showing on-screen debug message so the player can see their collection progress
	   Key 2 means repeated pickups replace the previous pickup message on screen
	   instead of stacking multiple lines which would clutter the display
	   2.0f seconds is enough to confirm the pickup without staying too long
	   FColor::Cyan makes it a distinct info color separate from win lose and warning
	*/
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(2, 2.0f, FColor::Cyan,
			FString::Printf(TEXT("Item collected! (%d / %d)"), CollectedItems, RequiredItems));
	}
}

/* Helper function to check if all required items have been collected
   Returns true if the collected count is equal to or greater than required count
   Used by OnPlayerReachedExit and can also be used by the exit door blueprint
   to show visual feedback like changing door color when unlocked
*/
bool AStealAndEscapeGameMode::HasCollectedAllItems() const
{
	return CollectedItems >= RequiredItems;
}