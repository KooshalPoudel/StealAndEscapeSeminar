/*
Project Name: Steal and Escape A 3D top-down stealth  escape game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: StealAndEscapeGameMode.cpp
Author: Kushal Poudel and Alok Poudel
Last Modified: April 26, 2026

Description: This is the Implementation of the GameMode which is the brain of our project
it manages win and lose conditions, item collection counting, the in game timer, the score
calculation,  HUD spawning and the end screen spawning.

OnPlayerCaught() is called from GuardAIController when a guard catches the player.
OnPlayerReachedExit() is called from ExitZone actor when the player reaches the door.
OnItemCollected() is called from StealableItem actors when the player picks up item.

Now we also added sound effects for item pickup , win and lose using PlaySound2D so the
player gets audio feedback for each event. The HUD is spawned at BeginPlay so the player
can see item count and timer durring the game , and when the game ends we remove the HUD
and spawn the end screen instead which shows the final result and final score.
*/

#include "StealAndEscapeGameMode.h"
#include "StealAndEscapePlayerController.h"
#include "StealAndEscapeCharacter.h"
#include "EndScreenWidget.h"
#include "HUDWidget.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Blueprint/UserWidget.h"
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

	// Enabling tick on gamemode so we can run the in game timer every frame.
	PrimaryActorTick.bCanEverTick = true;
}

/* BeginPlay is called when the game starts. Here we reset all the game state varibles
   to their starting values so the timer , item count and game over flag all start fresh
   even if level is reloaded. After that we spawn the HUD so player can see the timer
   and item count from the begining of the game.
*/
void AStealAndEscapeGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Reseting the game state varibles to their starting values 
	bIsGameOver = false;
	CollectedItems = 0;
	ElapsedTime = 0.f;

	// Spawning the HUD so player can see item count and timer
	SpawnHUD();
}

/* Tick runs every frame , here we use it to count up the elapsed time of the player.
   We only add to the time when the game isnot over so that timer frezes on win or lose.
   This time is later used to caluclate the score and as well as to display on the HUD as well.
*/
void AStealAndEscapeGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Only counting time if game is still going on otherwise noneed
	if (!bIsGameOver)
	{
		ElapsedTime += DeltaTime;
	}
}

/* CalculateScore is the function which calulates the players final score at the end.
   The score is made of two parts , item bonus and time bonus.
   Firstly each item collected give 1000 points so more items  thenn bigger item bonus.
   Then for time bonus, if the player finishes within 30 seconds they get full 1000 points
   bonus .If they take longer than that we deduct 2 points for each extra second after 30
   seconds. We use FMath::Max so that time bonus never goes below 0 even if player takes
   really long. Final score is sum of both bonuses.
*/
int32 AStealAndEscapeGameMode::CalculateScore() const
{
	// Item bonus is just 1000 points per item collected
	int32 ItemBonus = CollectedItems * 1000;

	int32 TimeBonus = 0;

	// If player finished within 30 seconds give full time bonus!!!
	if (ElapsedTime <= 30.f)
	{
		TimeBonus = 1000;
	}
	else
	{
		/* If player took more than 30 seconds then  deduct 2 points for every extra second
		   after the 30 second mark. FMath::Max makes sure bonus doesnot go below 0 \.
		*/
		float ExtraSeconds = ElapsedTime - 30.f;
		int32 Deduction = FMath::FloorToInt(ExtraSeconds * 2.f);
		TimeBonus = FMath::Max(0, 1000 - Deduction);
	}

	// Final score is item bonus + plus time bonus 
	return ItemBonus + TimeBonus;
}

/* This is our Lose Condition which is Called by GuardAIController when guard catches
   the player. Firstly we check if the game is already over or not to avoid triggering
   multiple times . Then we play the lose sound for audio feedback. After that we disable
   player input and stop character movement immediatly so the player freezes . Then we
   remove the HUD from screen because we donot want to show timer and item count over the
   end screen. After that we spawn the end screen which shows the lose result. And finaly
   the game is paused so guards and all other actors stop moving too.
*/
void AStealAndEscapeGameMode::OnPlayerCaught()
{
	// If game is already over do not trigger again!
	if (bIsGameOver) return;

	bIsGameOver = true;

	// Playing the lose sound effect using PlaySound2D so the player hears it 
	if (LoseSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), LoseSound);
	}

	// Getting player controller to disable input so player cannot move after being caught
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		// Disabling input on the controller itself so no input reaches the character at all.
		PC->DisableInput(PC);

		// Also stopping character movement immediately so the character does not slide or drift
		ACharacter* PlayerChar = Cast<ACharacter>(PC->GetPawn());
		if (PlayerChar && PlayerChar->GetCharacterMovement())
		{
			PlayerChar->GetCharacterMovement()->StopMovementImmediately();
		}
	}

	/* Removing the HUD from the screen because we donot want it overlaping with the
	   end screen and showing item count and timer is not needed after game over
	*/
	if (HUDInstance)
	{
		HUDInstance->RemoveFromParent();
		HUDInstance = nullptr;
	}

	// Spawning the end screen widget and telling it to show the lose layout
	UEndScreenWidget* EndScreen = SpawnEndScreen();
	if (EndScreen)
	{
		EndScreen->ShowLoseScreen(CollectedItems, RequiredItems, ElapsedTime);
	}

	// Pausing the game so guards and all other actors stop moving too
	// which makes sure nothing keeps running in the background after gameover
	UGameplayStatics::SetGamePaused(GetWorld(), true);
}

/* This is the Win- Condition which is Called by ExitZone when player overlaps the exit
   box. Firstly we check if the game is already over , then check if all items are
   collected . If items are not all collected it logs a message and does not end the
   game . If all items are collected we calulate the final score , play win sound ,
   stop movement, disable input , remove HUD and spawn the end screen with win layout
   and finaly pause the game.
*/
void AStealAndEscapeGameMode::OnPlayerReachedExit()
{
	// If game is already over do not triger again
	if (bIsGameOver) return;

	// Check if player has collected all required items before allowing escape
	if (!HasCollectedAllItems())
	{
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

	// Calulating final score now that the player has won
	int32 FinalScore = CalculateScore();

	// Playing the win sound effect using PlaySound2D for audio feedback
	if (WinSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), WinSound);
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

	// Removing HUD from the screen so it doesnot overlap with the end screen 
	if (HUDInstance)
	{
		HUDInstance->RemoveFromParent();
		HUDInstance = nullptr;
	}

	// Spawning the end screen widget and telling it to show the win layout with final score 
	UEndScreenWidget* EndScreen = SpawnEndScreen();
	if (EndScreen)
	{
		EndScreen->ShowWinScreen(CollectedItems, RequiredItems, ElapsedTime, FinalScore);
	}

	// Pausing the game so everything freezes on the win screen
	UGameplayStatics::SetGamePaused(GetWorld(), true);
}

/* This is Item Collection mechanismn which is Called by StealableItem when the player
   picks up an item. We increment the collected count and play the pickup sound for
   audio feedback. Then we log the current progress on screen so player can see how
   many items they have collected so far . This count is checked by OnPlayerReachedExit
   to decide if the player can escape or not.
*/
void AStealAndEscapeGameMode::OnItemCollected()
{
	CollectedItems++;

	// Playing item pickup sound effect using PlaySound2D so player hears the pickup
	if (ItemPickupSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), ItemPickupSound);
	}

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

/* SpawnEndScreen is a helper funtion that creates the end screen widget and adds it to
   the viewport. We first check if EndScreenWidgetClass is set in the blueprint , if not
   we just return nullpointer because there is nothing to spawn. Then we get the player
   controller and use CreateWidget to make a new instance ofthe end screen widget.
   After adding it to viewport we change the input mode to UI only so player can click
   buttons on the end screen like restart or quit. We also show the mouse cursor so
   player can see what they are clicking. Finaly we return the widget instance so the
   caller can call ShowWinScreen or ShowLoseScreen on it.
*/
UEndScreenWidget* AStealAndEscapeGameMode::SpawnEndScreen()
{
	// If no end screen widget class is set we cannot spawn anything
	if (!EndScreenWidgetClass)
	{
		return nullptr;
	}

	// Getting player controller because CreateWidget needs it as the owner
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return nullptr;

	// Creating new instance of the end screen widget from the class set in blueprint
	EndScreenInstance = CreateWidget<UEndScreenWidget>(PC, EndScreenWidgetClass);
	if (!EndScreenInstance) return nullptr;

	// Adding widget to viewport so it actually shows on screen 
	EndScreenInstance->AddToViewport();

	/* Changing input modee to UI only so player can click buttons on the end screen
	   like restart and quit . SetWidgetToFocus makes sure the widget is focused
	   for keyboard input as well.
	*/
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(EndScreenInstance->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(InputMode);

	// Showing mouse cursor so player can click the buttons on the end screen 
	PC->bShowMouseCursor = true;

	return EndScreenInstance;
}

/* SpawnHUD is a helper funtion that creates the HUD widget and adds it to the viewport.
   This is called from BeginPlay so the HUD shows up as soon as the game starts. The HUD
   shows things like the timer and the item count to the player during gameplay. We first
   check if HUDWidgetClass is set in the blueprint , if not we just return because there
   is nothing to spawn . Then we get the player controller and create the widget instance
   then add it to viewport so it actually shows on the screen.
*/
void AStealAndEscapeGameMode::SpawnHUD()
{
	// If no HUD widget class is set in blueprint then nothing is spawend 
	if (!HUDWidgetClass)
	{
		return;
	}

	// Getting player controller because CreateWidget needs it as the owner
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	// Creating the HUD widget instance from the class set in blueprint 
	HUDInstance = CreateWidget<UHUDWidget>(PC, HUDWidgetClass);
	if (HUDInstance)
	{
		// Adding to viewport so HUD actually shows on screen
		HUDInstance->AddToViewport();
	}
}