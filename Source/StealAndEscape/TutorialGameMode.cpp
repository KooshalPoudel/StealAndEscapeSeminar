/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: TutorialGameMode.cpp
Author: Kushal Poudel and Alok Poudel
Last Modified: April 24, 2026

Description: Implementation of the 10-step tutorial GameMode.

The tutorial uses strict gating to enforce step order:
  - Player input is disabled during interstitial messages (success/caught/
	transition delays) so they can't move past the next step before seeing it.
  - Guards start frozen. They activate only when the player reaches step 5
	(the guard warning). Before that the guard stands still even if the
	player wanders into their sight cone.
  - On caught-teleport, guards are reset to patrol and player input is
	briefly disabled during the caught message so the player sees it.
*/

#include "TutorialGameMode.h"
#include "TutorialStepTrigger.h"
#include "TutorialWidget.h"
#include "GuardAIController.h"
#include "GuardCharacter.h"
#include "StealAndEscapeCharacter.h"
#include "StealAndEscapePlayerController.h"
#include "StealableItem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

ATutorialGameMode::ATutorialGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ATutorialGameMode::BeginPlay()
{
	Super::BeginPlay();

	CurrentStepIndex = 0;
	bIsBeingChased = false;
	HideTimeAccumulated = 0.f;
	EscapeStepActiveTime = 0.f;
	bShowingInterstitialMessage = false;
	bHasShownSpottedMessage = false;
	bGuardChaseActive = false;
	bGuardsActivated = false;

	ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (PlayerChar)
	{
		LastCheckpointTransform = PlayerChar->GetActorTransform();
	}

	if (TutorialWidgetClass)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (PC)
		{
			TutorialWidgetInstance = CreateWidget<UTutorialWidget>(PC, TutorialWidgetClass);
			if (TutorialWidgetInstance)
			{
				TutorialWidgetInstance->AddToViewport(10);
			}
		}
	}

	GatherStepTriggers();
	UpdateStepTriggerActivation();

	// Freeze guards at start - they only activate at GuardActivationStep
	FreezeGuards();

	ShowCurrentStepMessage();
}

void ATutorialGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsGameOver) return;

	// Activate guards the first time we reach GuardActivationStep
	if (!bGuardsActivated && CurrentStepIndex >= GuardActivationStep)
	{
		ActivateGuards();
	}

	// Global chase detection - shows "Guard saw you!" warning during any step
	if (!bShowingInterstitialMessage && bGuardsActivated)
	{
		bool bGuardChasingNow = IsAnyGuardChasing();

		if (bGuardChasingNow && !bGuardChaseActive)
		{
			bGuardChaseActive = true;
			if (TutorialWidgetInstance)
			{
				TutorialWidgetInstance->SetMessage(GuardSpottedMessage);
				TutorialWidgetInstance->SetFlashing(false);
			}
		}
		else if (!bGuardChasingNow && bGuardChaseActive)
		{
			bGuardChaseActive = false;
			ShowCurrentStepMessage();
		}
	}

	if (bShowingInterstitialMessage) return;
	if (!StepTriggers.IsValidIndex(CurrentStepIndex)) return;

	ATutorialStepTrigger* Current = StepTriggers[CurrentStepIndex];
	if (!Current) return;

	switch (Current->StepType)
	{
	case ETutorialStepType::PressKey:
	{
		if (CheckKeyPressForCurrentStep())
		{
			CompleteCurrentStep();
		}
		break;
	}

	case ETutorialStepType::ApproachItem:
	{
		if (HasCollectedAllItems())
		{
			CompleteCurrentStep();
			break;
		}
		if (IsPlayerNearAnyItem())
		{
			CompleteCurrentStep();
		}
		break;
	}

	case ETutorialStepType::GrabItem:
	{
		if (HasCollectedAllItems())
		{
			CompleteCurrentStep();
		}
		break;
	}

	case ETutorialStepType::EscapeGuard:
	{
		EscapeStepActiveTime += DeltaTime;

		if (bGuardChaseActive)
		{
			bIsBeingChased = true;
			HideTimeAccumulated = 0.f;
		}
		else if (bIsBeingChased)
		{
			HideTimeAccumulated += DeltaTime;
			if (HideTimeAccumulated >= HideDurationRequired)
			{
				CompleteCurrentStep();
			}
		}
		else
		{
			if (EscapeStepActiveTime >= EscapeGuardGracePeriod)
			{
				CompleteCurrentStep();
			}
		}
		break;
	}

	default:
		break;
	}
}

/* Freezes all guards in the level. Sets their MaxWalkSpeed to 0 and stops
   any current movement. Also unregisters their perception so they don't
   notice the player during tutorial movement steps.
*/
void ATutorialGameMode::FreezeGuards()
{
	for (TActorIterator<AGuardCharacter> It(GetWorld()); It; ++It)
	{
		AGuardCharacter* Guard = *It;
		if (!Guard) continue;

		// Stop any current movement
		if (Guard->GetCharacterMovement())
		{
			Guard->GetCharacterMovement()->StopMovementImmediately();
			Guard->GetCharacterMovement()->MaxWalkSpeed = 0.f;
		}

		// Stop the AI controller so it doesn't try to move the guard
		AGuardAIController* GuardAI = Cast<AGuardAIController>(Guard->GetController());
		if (GuardAI)
		{
			GuardAI->StopMovement();
		}
	}

}

/* Wakes guards up - restores their patrol speed and sends them back to
   their patrol points. Called once when the player reaches step 5.
*/
void ATutorialGameMode::ActivateGuards()
{
	if (bGuardsActivated) return;
	bGuardsActivated = true;

	for (TActorIterator<AGuardCharacter> It(GetWorld()); It; ++It)
	{
		AGuardCharacter* Guard = *It;
		if (!Guard) continue;

		if (Guard->GetCharacterMovement())
		{
			Guard->GetCharacterMovement()->MaxWalkSpeed = Guard->PatrolSpeed;
		}

		AGuardAIController* GuardAI = Cast<AGuardAIController>(Guard->GetController());
		if (GuardAI)
		{
			GuardAI->ResetToPatrol();
		}
	}

}

/* Disables the player's input so they can't move/act during messages. */
void ATutorialGameMode::FreezePlayer()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	PC->DisableInput(PC);

	ACharacter* PlayerChar = Cast<ACharacter>(PC->GetPawn());
	if (PlayerChar && PlayerChar->GetCharacterMovement())
	{
		PlayerChar->GetCharacterMovement()->StopMovementImmediately();
	}
}

/* Re-enables the player's input. */
void ATutorialGameMode::UnfreezePlayer()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	PC->EnableInput(PC);
}

bool ATutorialGameMode::IsAnyGuardChasing() const
{
	for (TActorIterator<AGuardCharacter> It(GetWorld()); It; ++It)
	{
		AGuardCharacter* Guard = *It;
		if (!Guard) continue;
		UCharacterMovementComponent* GuardMove = Guard->GetCharacterMovement();
		if (GuardMove && FMath::IsNearlyEqual(GuardMove->MaxWalkSpeed, Guard->ChaseSpeed, 1.f))
		{
			return true;
		}
	}
	return false;
}

bool ATutorialGameMode::CheckKeyPressForCurrentStep()
{
	if (!StepTriggers.IsValidIndex(CurrentStepIndex)) return false;
	ATutorialStepTrigger* Current = StepTriggers[CurrentStepIndex];
	if (!Current) return false;

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return false;

	FKey TargetKey(Current->KeyToPress);
	return PC->IsInputKeyDown(TargetKey);
}

bool ATutorialGameMode::IsPlayerNearAnyItem() const
{
	ACharacter* PlayerCharBase = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	AStealAndEscapeCharacter* Player = Cast<AStealAndEscapeCharacter>(PlayerCharBase);
	if (!Player) return false;
	return Player->HasNearbyItems();
}

void ATutorialGameMode::CompleteCurrentStep()
{
	if (bIsGameOver) return;
	if (bShowingInterstitialMessage) return;
	if (!StepTriggers.IsValidIndex(CurrentStepIndex)) return;

	ATutorialStepTrigger* Current = StepTriggers[CurrentStepIndex];
	if (!Current) return;

	ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (PlayerChar)
	{
		LastCheckpointTransform = PlayerChar->GetActorTransform();
	}


	bIsBeingChased = false;
	HideTimeAccumulated = 0.f;

	if (Current->StepType == ETutorialStepType::GrabItem)
	{
		ResetGuardsToPatrol();
		bGuardChaseActive = false;
	}

	FString SuccessMsg = Current->SuccessMessage.IsEmpty()
		? DefaultSuccessMessage
		: Current->SuccessMessage;

	if (TutorialWidgetInstance)
	{
		TutorialWidgetInstance->SetMessage(SuccessMsg);
		TutorialWidgetInstance->SetFlashing(false);
	}

	USoundBase* SoundToPlay = Current->CustomSuccessSound ? Current->CustomSuccessSound : StepCompleteSound;
	if (SoundToPlay)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), SoundToPlay);
	}

	bShowingInterstitialMessage = true;

	// Freeze the player so they can't move during the success message
	FreezePlayer();

	GetWorld()->GetTimerManager().SetTimer(
		StepAdvanceTimerHandle,
		this,
		&ATutorialGameMode::AdvanceToNextStep,
		SuccessMessageDuration,
		false
	);
}

void ATutorialGameMode::AdvanceToNextStep()
{

	bShowingInterstitialMessage = false;
	CurrentStepIndex++;
	EscapeStepActiveTime = 0.f;
	bHasShownSpottedMessage = false;

	if (CurrentStepIndex >= StepTriggers.Num())
	{
		if (TutorialWidgetInstance)
		{
			TutorialWidgetInstance->SetMessage(TEXT("Tutorial complete!"));
			TutorialWidgetInstance->SetFlashing(false);
		}
		UnfreezePlayer();
		return;
	}

	// Re-enable player input now that we're advancing to the next step
	UnfreezePlayer();

	UpdateStepTriggerActivation();
	OnEnterStep();
}

void ATutorialGameMode::OnEnterStep()
{
	if (StepTriggers.IsValidIndex(CurrentStepIndex))
	{
		ATutorialStepTrigger* Current = StepTriggers[CurrentStepIndex];
		if (Current)
		{
			if ((Current->StepType == ETutorialStepType::ApproachItem ||
				Current->StepType == ETutorialStepType::GrabItem) &&
				HasCollectedAllItems())
			{

				if (TutorialWidgetInstance)
				{
					TutorialWidgetInstance->SetMessage(ItemAlreadyGrabbedMessage);
					TutorialWidgetInstance->SetFlashing(false);
				}

				if (StepCompleteSound)
				{
					UGameplayStatics::PlaySound2D(GetWorld(), StepCompleteSound);
				}

				bShowingInterstitialMessage = true;
				FreezePlayer();
				GetWorld()->GetTimerManager().SetTimer(
					StepAdvanceTimerHandle,
					this,
					&ATutorialGameMode::AdvanceToNextStep,
					SuccessMessageDuration,
					false
				);
				return;
			}
		}
	}

	ShowCurrentStepMessage();
}

void ATutorialGameMode::OnPlayerCaught()
{

	if (GetWorld()->GetTimerManager().IsTimerActive(CaughtTeleportTimerHandle))
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(StepAdvanceTimerHandle);

	if (TutorialWidgetInstance)
	{
		TutorialWidgetInstance->SetMessage(CaughtMessage);
		TutorialWidgetInstance->SetFlashing(false);
	}

	if (CaughtSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), CaughtSound);
	}

	// Freeze the player AND guards during the caught message
	FreezePlayer();
	FreezeGuards();

	bShowingInterstitialMessage = true;
	bIsBeingChased = false;
	HideTimeAccumulated = 0.f;
	bHasShownSpottedMessage = false;
	bGuardChaseActive = false;

	FTimerManager& TimerMgr = GetWorld()->GetTimerManager();
	TimerMgr.ClearTimer(CaughtTeleportTimerHandle);
	TimerMgr.SetTimer(
		CaughtTeleportTimerHandle,
		this,
		&ATutorialGameMode::PerformCaughtTeleport,
		CaughtMessageDuration,
		false
	);
}

void ATutorialGameMode::PerformCaughtTeleport()
{

	bShowingInterstitialMessage = false;

	TeleportPlayerToCheckpoint();

	// Guards are currently frozen - reactivate them since we were past
	// the GuardActivationStep when caught. Reset to patrol so they go
	// back to their patrol points after catching.
	if (bGuardsActivated)
	{
		for (TActorIterator<AGuardCharacter> It(GetWorld()); It; ++It)
		{
			AGuardCharacter* Guard = *It;
			if (!Guard) continue;
			if (Guard->GetCharacterMovement())
			{
				Guard->GetCharacterMovement()->MaxWalkSpeed = Guard->PatrolSpeed;
			}
			AGuardAIController* GuardAI = Cast<AGuardAIController>(Guard->GetController());
			if (GuardAI)
			{
				GuardAI->ResetToPatrol();
			}
		}
	}

	bool bSkipped = SkipObsoleteSteps();

	EscapeStepActiveTime = 0.f;
	bHasShownSpottedMessage = false;
	bGuardChaseActive = false;

	UpdateStepTriggerActivation();

	if (bSkipped)
	{
		if (TutorialWidgetInstance)
		{
			TutorialWidgetInstance->SetMessage(ItemAlreadyGrabbedMessage);
			TutorialWidgetInstance->SetFlashing(false);
		}

		if (StepCompleteSound)
		{
			UGameplayStatics::PlaySound2D(GetWorld(), StepCompleteSound);
		}

		bShowingInterstitialMessage = true;
		FreezePlayer();
		GetWorld()->GetTimerManager().SetTimer(
			StepAdvanceTimerHandle,
			this,
			&ATutorialGameMode::AdvanceToNextStep,
			SuccessMessageDuration,
			false
		);

		CurrentStepIndex--;
	}
	else
	{
		// Re-enable player input so they can move again
		UnfreezePlayer();
		ShowCurrentStepMessage();
	}
}

bool ATutorialGameMode::SkipObsoleteSteps()
{
	int32 OriginalIndex = CurrentStepIndex;

	while (StepTriggers.IsValidIndex(CurrentStepIndex))
	{
		ATutorialStepTrigger* Current = StepTriggers[CurrentStepIndex];
		if (!Current) break;

		bool bObsolete = false;

		if ((Current->StepType == ETutorialStepType::ApproachItem ||
			Current->StepType == ETutorialStepType::GrabItem) &&
			HasCollectedAllItems())
		{
			bObsolete = true;
		}

		if (!bObsolete) break;
		CurrentStepIndex++;
	}

	return CurrentStepIndex != OriginalIndex;
}

void ATutorialGameMode::OnItemCollected()
{
	Super::OnItemCollected();

	if (bShowingInterstitialMessage) return;
	if (!StepTriggers.IsValidIndex(CurrentStepIndex)) return;

	ATutorialStepTrigger* Current = StepTriggers[CurrentStepIndex];
	if (Current && Current->StepType == ETutorialStepType::GrabItem)
	{
		CompleteCurrentStep();
	}
}

void ATutorialGameMode::GatherStepTriggers()
{
	StepTriggers.Empty();

	for (TActorIterator<ATutorialStepTrigger> It(GetWorld()); It; ++It)
	{
		ATutorialStepTrigger* Trigger = *It;
		if (Trigger)
		{
			StepTriggers.Add(Trigger);
		}
	}

	StepTriggers.Sort([](const ATutorialStepTrigger& A, const ATutorialStepTrigger& B)
		{
			return A.StepIndex < B.StepIndex;
		});

}

void ATutorialGameMode::UpdateStepTriggerActivation()
{
	for (int32 i = 0; i < StepTriggers.Num(); ++i)
	{
		ATutorialStepTrigger* Trigger = StepTriggers[i];
		if (!Trigger) continue;
		Trigger->SetTriggerActive(i == CurrentStepIndex);
	}
}

void ATutorialGameMode::ShowCurrentStepMessage()
{
	if (!StepTriggers.IsValidIndex(CurrentStepIndex))
	{
		return;
	}

	ATutorialStepTrigger* Current = StepTriggers[CurrentStepIndex];
	if (!Current) return;

	const FString& Msg = Current->StepMessage;

	if (TutorialWidgetInstance)
	{
		TutorialWidgetInstance->SetMessage(Msg);
		TutorialWidgetInstance->SetFlashing(Current->StepType == ETutorialStepType::ReachExit);
	}
}

void ATutorialGameMode::TeleportPlayerToCheckpoint()
{
	ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!PlayerChar) return;

	if (PlayerChar->GetCharacterMovement())
	{
		PlayerChar->GetCharacterMovement()->StopMovementImmediately();
	}

	PlayerChar->SetActorLocationAndRotation(
		LastCheckpointTransform.GetLocation(),
		LastCheckpointTransform.GetRotation(),
		false,
		nullptr,
		ETeleportType::TeleportPhysics
	);
}

void ATutorialGameMode::ResetGuardsToPatrol()
{
	for (TActorIterator<AGuardCharacter> It(GetWorld()); It; ++It)
	{
		AGuardCharacter* Guard = *It;
		if (!Guard) continue;

		AGuardAIController* GuardAI = Cast<AGuardAIController>(Guard->GetController());
		if (!GuardAI) continue;

		GuardAI->ResetToPatrol();
	}
}