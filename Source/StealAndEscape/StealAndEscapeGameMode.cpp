/*
Project Name: Steal and Escape A 3D top-down stealth  escape game developed in Unreal Engine 
Course: CSCI 491 Seminar
File Name: StealAndEscapeGameMode.cpp
Author: Kushal Poudel and Alok Poudel
Last Modified: April 19, 2026

Description: Implementation of the GameMode. Added item pickup, win, and lose
             sound effects via PlaySound2D.
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
	PlayerControllerClass = AStealAndEscapePlayerController::StaticClass();

	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDownCPP/Blueprints/TopDownCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	PrimaryActorTick.bCanEverTick = true;
}

void AStealAndEscapeGameMode::BeginPlay()
{
	Super::BeginPlay();

	bIsGameOver = false;
	CollectedItems = 0;
	ElapsedTime = 0.f;

	SpawnHUD();
}

void AStealAndEscapeGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsGameOver)
	{
		ElapsedTime += DeltaTime;
	}
}

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

/* Lose Condition */
void AStealAndEscapeGameMode::OnPlayerCaught()
{
	if (bIsGameOver) return;

	bIsGameOver = true;

	// Play lose sound
	if (LoseSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), LoseSound);
	}

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

	if (HUDInstance)
	{
		HUDInstance->RemoveFromParent();
		HUDInstance = nullptr;
	}

	UEndScreenWidget* EndScreen = SpawnEndScreen();
	if (EndScreen)
	{
		EndScreen->ShowLoseScreen(CollectedItems, RequiredItems, ElapsedTime);
	}

	UGameplayStatics::SetGamePaused(GetWorld(), true);
}

/* Win Condition */
void AStealAndEscapeGameMode::OnPlayerReachedExit()
{
	if (bIsGameOver) return;

	if (!HasCollectedAllItems())
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1, 3.0f, FColor::Yellow,
				FString::Printf(TEXT("Collect all items before escaping! (%d / %d)"), CollectedItems, RequiredItems));
		}
		return;
	}

	bIsGameOver = true;

	int32 FinalScore = CalculateScore();

	// Play win sound
	if (WinSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), WinSound);
	}

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

	if (HUDInstance)
	{
		HUDInstance->RemoveFromParent();
		HUDInstance = nullptr;
	}

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

	// Play item pickup sound
	if (ItemPickupSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), ItemPickupSound);
	}

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

UEndScreenWidget* AStealAndEscapeGameMode::SpawnEndScreen()
{
	if (!EndScreenWidgetClass)
	{
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

void AStealAndEscapeGameMode::SpawnHUD()
{
	if (!HUDWidgetClass)
	{
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
