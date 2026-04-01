/*
Project Name: Steal and Escape A 3D top-down stealth  escape game developed in Unreal Engine 
Course: CSCI 491 Seminar
File Name: StealAndEscapeGameMode.cpp
Author: Kushal Poudel and Alok Poudel
Last Modified: March 22, 2026

Description: This is the Implementation of the GameMode which is the brain of our project
it  manages win and lose conditions.
OnPlayerCaught() is called from  GuardAIController when a guard catches the player.
OnPlayerReachedExit() is called fromExitZone actor when the player reaches the door.
OnItemCollected() is called from StealableItem actors when the player picks up item
*/

#include "StealAndEscapeGameMode.h"
#include "StealAndEscapePlayerController.h"
#include "StealAndEscapeCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"

AStealAndEscapeGameMode::AStealAndEscapeGameMode()
{
	// here we are using  our custom PlayerController class
	PlayerControllerClass = AStealAndEscapePlayerController::StaticClass();

	// seting default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDownCPP/Blueprints/TopDownCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

/* This is our Lose Condition which is Called by GuardAIController when guard catches 
   the player. here Firstly we checks if the game is already over or not to avoid 
   triggering multiple times. Then as soon as that we Disable player input and 
   which then stops character movement immediately so the player freezes
   Then  game is paused so guards and all other actors also stop moving
*/
void AStealAndEscapeGameMode::OnPlayerCaught()
{
	// If game is already over do not trigger again
	if (bIsGameOver) return;

	bIsGameOver = true;

	UE_LOG(LogTemp, Warning, TEXT("GAME OVER - Player was caught by guard!"));

	/* Showing on-screen debug message so the player can see the result during gameplay
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

	// Here we are Pausing the game so guards and all other actors stop moving too
	// which makes sure nothing keeps running in the background after game over
	UGameplayStatics::SetGamePaused(GetWorld(), true);
}

/* This is the Win Condition whichis Called by ExitZone when player overlaps 
   the exit box. First checks if game is already over then checks if all items are collected
   If items are not all collected it logs a message and does not end the game
   If all items are collected it stops movement, disables input, pauses the game
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
}

/* Thisis Item Collection mechanism which is  Called by StealableItem when the player picks up an item
   Increment the collected count and logs the current progress
   This count is checked by OnPlayerReachedExit to decide if the player can escape
*/
void AStealAndEscapeGameMode::OnItemCollected()
{
	CollectedItems++;
	UE_LOG(LogTemp, Warning, TEXT("Item collected: %d / %d"), CollectedItems, RequiredItems);

	/* Showing on-screen debug message so the player can see their collection progress
	*/
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(2, 2.0f, FColor::Cyan,
			FString::Printf(TEXT("Item collected! (%d / %d)"), CollectedItems, RequiredItems));
	}
}

/* This is Helper function to check if all required items have been collected
   Returns true if the collected count is equal to or greater than required count
   Used by OnPlayerReachedExit and can also be used by the exit door blueprint
   to show visual feedback like changing door color when unlocked.
*/
bool AStealAndEscapeGameMode::HasCollectedAllItems() const
{
	return CollectedItems >= RequiredItems;
}