/*
Project Name: Steal and Escape A 3D top-down stealth  escape game developed in Unreal Engine 
Course: CSCI 491 Seminar
File Name: StealAndEscapeGameMode.cpp
Author: Kushal Poudel and Alok Poudel
Last Modified: April 18, 2026

Description: This is the Implementation of the GameMode which is the brain of our project
it  manages win and lose conditions.
OnPlayerCaught() is called from  GuardAIController when a guard catches the player.
OnPlayerReachedExit() is called fromExitZone actor when the player reaches the door.
OnItemCollected() is called from StealableItem actors when the player picks up item

Update (April 16 2026): On win / lose we now also spawn the EndScreenWidget.
Update (April 18 2026): Added gameplay timer, score calculation, HUD widget spawn.
Update (April 18 2026): EndScreen now receives time and score for display.
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

	// Enable tick so we can run the gameplay timer
	PrimaryActorTick.bCanEverTick = true;
}

void AStealAndEscapeGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Reset game state on level begin - required for Retry to work correctly
	bIsGameOver = false;
	CollectedItems = 0;
	ElapsedTime = 0.f;

	// Spawn the HUD so the player sees item count, timer, and score from frame 1
	SpawnHUD();
}

void AStealAndEscapeGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Only advance the timer while the game is actively being played
	if (!bIsGameOver)
	{
		ElapsedTime += DeltaTime;
	}
}

/* Score formula:
     item bonus = CollectedItems * 1000
     time bonus = 1000 if ElapsedTime <= 30s
                  otherwise max(0, 1000 - (ElapsedTime - 30) * 2)
     total = item bonus + time bonus
   Score is always non-negative. */
int32 AStealAndEscapeGameMode::CalculateScore() const
{
	int32 ItemBonus = CollectedItems * 1000;

	int32 TimeBonus = 0;
	if (ElapsedTime <= 30.f)
	{
		TimeBonus = 1000;
	}
	else
	{
		float ExtraSeconds = ElapsedTime - 30.f;
		int32 Deduction = FMath::FloorToInt(ExtraSeconds * 2.f);
		TimeBonus = FMath::Max(0, 1000 - Deduction);
	}

	return ItemBonus + TimeBonus;
}

/* This is our Lose Condition */
void AStealAndEscapeGameMode::OnPlayerCaught()
{
	if (bIsGameOver) return;

	bIsGameOver = true;

	UE_LOG(LogTemp, Warning, TEXT("GAME OVER - Player was caught by guard!"));

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		PC->DisableInput(PC);

		ACharacter* PlayerChar = Cast<ACharacter>(PC->GetPawn());
		if (PlayerChar && PlayerChar->GetCharacterMovement())
		{
			PlayerChar->GetCharacterMovement()->StopMovementImmediately();
		}
	}

	// Remove the HUD so it does not clutter the end screen
	if (HUDInstance)
	{
		HUDInstance->RemoveFromParent();
		HUDInstance = nullptr;
	}

	// Spawn end screen - LOSE gets time but no score
	UEndScreenWidget* EndScreen = SpawnEndScreen();
	if (EndScreen)
	{
		EndScreen->ShowLoseScreen(CollectedItems, RequiredItems, ElapsedTime);
	}

	UGameplayStatics::SetGamePaused(GetWorld(), true);
}

/* This is the Win Condition */
void AStealAndEscapeGameMode::OnPlayerReachedExit()
{
	if (bIsGameOver) return;

	if (!HasCollectedAllItems())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot exit - Collect all items first! (%d / %d)"), CollectedItems, RequiredItems);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1, 3.0f, FColor::Yellow,
				FString::Printf(TEXT("Collect all items before escaping! (%d / %d)"), CollectedItems, RequiredItems));
		}
		return;
	}

	bIsGameOver = true;

	int32 FinalScore = CalculateScore();
	UE_LOG(LogTemp, Warning, TEXT("YOU WIN - Escaped with all items! Score: %d, Time: %.1fs"),
		FinalScore, ElapsedTime);

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		PC->DisableInput(PC);

		ACharacter* PlayerChar = Cast<ACharacter>(PC->GetPawn());
		if (PlayerChar && PlayerChar->GetCharacterMovement())
		{
			PlayerChar->GetCharacterMovement()->StopMovementImmediately();
		}
	}

	// Remove HUD on win
	if (HUDInstance)
	{
		HUDInstance->RemoveFromParent();
		HUDInstance = nullptr;
	}

	// Spawn end screen - WIN gets time and score
	UEndScreenWidget* EndScreen = SpawnEndScreen();
	if (EndScreen)
	{
		EndScreen->ShowWinScreen(CollectedItems, RequiredItems, ElapsedTime, FinalScore);
	}

	UGameplayStatics::SetGamePaused(GetWorld(), true);
}

/* Item Collection */
void AStealAndEscapeGameMode::OnItemCollected()
{
	CollectedItems++;
	UE_LOG(LogTemp, Warning, TEXT("Item collected: %d / %d"), CollectedItems, RequiredItems);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(2, 2.0f, FColor::Cyan,
			FString::Printf(TEXT("Item collected! (%d / %d)"), CollectedItems, RequiredItems));
	}
}

bool AStealAndEscapeGameMode::HasCollectedAllItems() const
{
	return CollectedItems >= RequiredItems;
}

/* Spawns end screen widget */
UEndScreenWidget* AStealAndEscapeGameMode::SpawnEndScreen()
{
	if (!EndScreenWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("StealAndEscapeGameMode - EndScreenWidgetClass is not set!"));
		return nullptr;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return nullptr;

	EndScreenInstance = CreateWidget<UEndScreenWidget>(PC, EndScreenWidgetClass);
	if (!EndScreenInstance) return nullptr;

	EndScreenInstance->AddToViewport();

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(EndScreenInstance->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(InputMode);
	PC->bShowMouseCursor = true;

	return EndScreenInstance;
}

/* Spawns HUD widget */
void AStealAndEscapeGameMode::SpawnHUD()
{
	if (!HUDWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("StealAndEscapeGameMode - HUDWidgetClass is not set. Skipping HUD spawn."));
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	HUDInstance = CreateWidget<UHUDWidget>(PC, HUDWidgetClass);
	if (HUDInstance)
	{
		HUDInstance->AddToViewport();
	}
}
