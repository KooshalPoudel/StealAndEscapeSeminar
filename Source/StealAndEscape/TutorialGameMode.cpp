/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar

File Name: TutorialGameMode.cpp
Author: Kushal Poudel and Alok Poudel
Last Modified: April 26, 2026

Description: Implementation of the 10-step tutorial GameMode . The tutorial uses strict gating
			 to enforce step order , so player has to do steps in correct order without skipping :
			 - Player input is disabled durring the interstitial messages (success / caught /
			   transition delays) so they cant move past the next step before seeing it.
			 - Guards start frozen . They activate only when the player reaches step 5 which is
			   the guard warning step . Before that the guard stands still even if the player
			   wanders into thier sight cone.
			 - On caught teleport , guards are reset to patrol and player input is briefly
			   disabled durring the caught message so the player can read it.
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
	// Enabling tick so we can poll step completion conditions every frame.

	PrimaryActorTick.bCanEverTick = true;
}

/* BeginPlay reseting all the tutorial state varibles to thier starting values . Then we
   save the player's spawn location as the first checkpoint , after that we spawn the tutorial widget on
   the screen , then gather the step triggers from the level ,also freeze guards and  then finaly show the
   first step message.
*/
void ATutorialGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Reseting all tutorial state varibles to thier starting values 
	CurrentStepIndex = 0;
	bIsBeingChased = false;
	HideTimeAccumulated = 0.f;
	EscapeStepActiveTime = 0.f;
	bShowingInterstitialMessage = false;
	bHasShownSpottedMessage = false;
	bGuardChaseActive = false;
	bGuardsActivated = false;

	// Saving player spawn location as the first checkpoint for caught teleport/ respawn
	ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (PlayerChar)
	{
		LastCheckpointTransform = PlayerChar->GetActorTransform();
	}

	// Creating and adding the tutorial widget to viewport so player can see step messages. 
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

	// Freezing guards at start , they only activate at GuardActivationStep. which is necessary for tuorial to work
	FreezeGuards();

	ShowCurrentStepMessage();
}

/* Tick handles all the per frame logic for the tutorial . Firstly  we activate guards once
   the player reaches the guard activation step i.e after wasd movements and enterint the building .
   Then we do global chase detection so we
   can show "Guard saw you" warning durring any step  as soon as guard starts chasing.
   Finaly we check the current step
   type and run the right completion check for that step.
*/
void ATutorialGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsGameOver) return;

	// Activating guards the first time we reach GuardActivationStep. 
	if (!bGuardsActivated && CurrentStepIndex >= GuardActivationStep)
	{
		ActivateGuards();
	}

	/* Global chase detection , shows "Guard saw you!" warning durring any step. We swap
	   between the warning and the normal step message based on wheather any guard is
	   currently chasing the player.
	*/
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

	// If we are showing a interstital message then we donot run step completion checks
	if (bShowingInterstitialMessage) return;
	if (!StepTriggers.IsValidIndex(CurrentStepIndex)) return;

	ATutorialStepTrigger* Current = StepTriggers[CurrentStepIndex];
	if (!Current) return;

	// Switch on the current step type and run the right completion check. 
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
		// If item was already collected somehow we just complete this step right away.
		//so that tutorial  flow looks a more natural.
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
		/* Escape step has 3 different ways to complete , 
		  1st if guard is chasing we track  hide time , 
		  if guard stops chasing we count it as escaped , and if no guard
		   ever chased after the grace period we just auto complete .
		*/
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
			// Grace period passed without any chase , just auto complete the step
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

/* Freezes all guards in the level by setting thier MaxWalkSpeed to 0 and stoping any
   current movement. We also tell the AI controller to stop moving so it doesnot try
   to keep moving the guard while frozen.
   so that it doesnot looks weird  when guard catches theplayer . and looks good while respawn.
*/
void ATutorialGameMode::FreezeGuards()
{
	for (TActorIterator<AGuardCharacter> It(GetWorld()); It; ++It)
	{
		AGuardCharacter* Guard = *It;
		if (!Guard) continue;

		// Stoping any current movement on the guard
		if (Guard->GetCharacterMovement())
		{
			Guard->GetCharacterMovement()->StopMovementImmediately();
			Guard->GetCharacterMovement()->MaxWalkSpeed = 0.f;
		}

		// Telling the AI controller to stop so it doesnot try to move the guard
		AGuardAIController* GuardAI = Cast<AGuardAIController>(Guard->GetController());
		if (GuardAI)
		{
			GuardAI->StopMovement();
		}
	}

}

/* Wakes guards up , restores thier patrol speed and sends them back to thier patrol
   points using ResetToPatrol. Called once when the player reaches step 5.
*/
void ATutorialGameMode::ActivateGuards()
{
	if (bGuardsActivated) return;
	bGuardsActivated = true;

	for (TActorIterator<AGuardCharacter> It(GetWorld()); It; ++It)
	{
		AGuardCharacter* Guard = *It;
		if (!Guard) continue;

		// Restoring patrol speed
		if (Guard->GetCharacterMovement())
		{
			Guard->GetCharacterMovement()->MaxWalkSpeed = Guard->PatrolSpeed;
		}

		// Reseting AI back to clean patrol state
		AGuardAIController* GuardAI = Cast<AGuardAIController>(Guard->GetController());
		if (GuardAI)
		{
			GuardAI->ResetToPatrol();
		}
	}

}

// Disables the player's input so they cant move or do anything during messages. 
void ATutorialGameMode::FreezePlayer()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	PC->DisableInput(PC);

	// Also stoping movement immediatly so the character doesnot slide
	ACharacter* PlayerChar = Cast<ACharacter>(PC->GetPawn());
	if (PlayerChar && PlayerChar->GetCharacterMovement())
	{
		PlayerChar->GetCharacterMovement()->StopMovementImmediately();
	}
}

// Re-enables the player's input so they can move again.
void ATutorialGameMode::UnfreezePlayer()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	PC->EnableInput(PC);
}

/* Checks if any guard in the level is currently chasing the player . We do this by
   checking each guard's MaxWalkSpeed and seeing if it matches thier ChaseSpeed value
   which is set by the AI controller when it spots the player.
*/
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

// Checks if the key required for current step is being pressed by the player or not 
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

// Checks if player is near any stealable item using the candidate list on player char
//similar logic applied from stelable item logic 
bool ATutorialGameMode::IsPlayerNearAnyItem() const
{
	ACharacter* PlayerCharBase = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	AStealAndEscapeCharacter* Player = Cast<AStealAndEscapeCharacter>(PlayerCharBase);
	if (!Player) return false;
	return Player->HasNearbyItems();
}

/* CompleteCurrentStep is called when the current step's completion condition is met .
   We save the current player position as the new checkpoint , reset the chase state ,
   show the success message , play a sound and start a timer to advance to the next
   step after a brief delay so the player has time to read the success message.
*/
void ATutorialGameMode::CompleteCurrentStep()
{
	if (bIsGameOver) return;
	if (bShowingInterstitialMessage) return;
	if (!StepTriggers.IsValidIndex(CurrentStepIndex)) return;

	ATutorialStepTrigger* Current = StepTriggers[CurrentStepIndex];
	if (!Current) return;

	// Saving current player position as the new checkpoint location for new respawn
	ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (PlayerChar)
	{
		LastCheckpointTransform = PlayerChar->GetActorTransform();
	}

	// Reseting the chase tracking variables
	bIsBeingChased = false;
	HideTimeAccumulated = 0.f;

	// If we just completed a grab item step we reset guards to patrol so they calm down
	if (Current->StepType == ETutorialStepType::GrabItem)
	{
		ResetGuardsToPatrol();
		bGuardChaseActive = false;
	}

	// Use the step's custom success message if it has one , otherwise fall back to default 
	FString SuccessMsg = Current->SuccessMessage.IsEmpty()
		? DefaultSuccessMessage
		: Current->SuccessMessage;

	if (TutorialWidgetInstance)
	{
		TutorialWidgetInstance->SetMessage(SuccessMsg);
		TutorialWidgetInstance->SetFlashing(false);
	}

	// Play custom step sound if it has one , otherwise play the default complete sound
	USoundBase* SoundToPlay = Current->CustomSuccessSound ? Current->CustomSuccessSound : StepCompleteSound;
	if (SoundToPlay)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), SoundToPlay);
	}

	bShowingInterstitialMessage = true;

	// Freezing the player so they cant move durring the success message
	FreezePlayer();

	// Setting timer to advance to next step after the message duration is over.
	GetWorld()->GetTimerManager().SetTimer(
		StepAdvanceTimerHandle,
		this,
		&ATutorialGameMode::AdvanceToNextStep,
		SuccessMessageDuration,
		false
	);
}

/* AdvanceToNextStep is called by the timer after the success message duration . We
   move on to the next step , reset some per-step state , and if all steps are done
   we show the tutorial complete message instead.
*/
void ATutorialGameMode::AdvanceToNextStep()
{
	bShowingInterstitialMessage = false;
	CurrentStepIndex++;
	EscapeStepActiveTime = 0.f;
	bHasShownSpottedMessage = false;

	// If we ran out of steps the tutorial is done , show complete message. 
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

	// Re-enabling player input now that we are advancing to the next step. 
	UnfreezePlayer();

	UpdateStepTriggerActivation();
	OnEnterStep();
}

/* OnEnterStep is called right after we advance to a new step . If the new step is an
   item step but the item is already collected (which can happen after a caught teleport)
   we just skip past it with a small message so the player doesnot get stuck.
*/
void ATutorialGameMode::OnEnterStep()
{
	if (StepTriggers.IsValidIndex(CurrentStepIndex))
	{
		ATutorialStepTrigger* Current = StepTriggers[CurrentStepIndex];
		if (Current)
		{
			// If item is already grabbed we skip this step automaticaly
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

/* OnPlayerCaught is overriden from the base GameMode . Instead of ending the game like
   the normal gamemode does , in tutorial we want to teleport the player back to the last
   checkpoint and let them try again . We show a "you got caught" message , freeze
   everything for a sec , then teleport them.
*/
void ATutorialGameMode::OnPlayerCaught()
{
	// If teleport timer is already running we donot trigger again to avoid double teleport
	if (GetWorld()->GetTimerManager().IsTimerActive(CaughtTeleportTimerHandle))
	{
		return;
	}

	// Cancelling any pending step advance timer because we are interupting it 
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

	// Freezing the player AND guards durring the caught message so nothing keeps moving!
	FreezePlayer();
	FreezeGuards();

	bShowingInterstitialMessage = true;
	bIsBeingChased = false;
	HideTimeAccumulated = 0.f;
	bHasShownSpottedMessage = false;
	bGuardChaseActive = false;

	// Setting timer to teleport back to checkpoint after the caught message is shown!
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

/* PerformCaughtTeleport actually does the teleport after the caught message duration .
   We teleport the player , reactivate guards if they were active before , skip any
   other  steps for ex(like grab item if item was already grabbed) and finaly show the
   current step message again.
*/
void ATutorialGameMode::PerformCaughtTeleport()
{
	bShowingInterstitialMessage = false;

	TeleportPlayerToCheckpoint();

	/* Guards are currently frozen , reactivate them since we were past the
	   GuardActivationStep when caught . Reseting to patrol so they go back
	   to thier patrol points after catching player.
	*/
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
		// We skipped some obsolete steps , show the already grabbed message and advance
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

		// Decrement so that when AdvanceToNextStep increments we land on the right step. 
		CurrentStepIndex--;
	}
	else
	{
		// Re-enabling player input so they can move again
		UnfreezePlayer();
		ShowCurrentStepMessage();
	}
}

/* SkipObsoleteSteps loops through the steps starting from the current one and skips
   any step that is no longer relevent (like grab item step when item is already
   grabbed) .Returns true if any steps were skipped so caller can show a message.
*/
bool ATutorialGameMode::SkipObsoleteSteps()
{
	int32 OriginalIndex = CurrentStepIndex;

	while (StepTriggers.IsValidIndex(CurrentStepIndex))
	{
		ATutorialStepTrigger* Current = StepTriggers[CurrentStepIndex];
		if (!Current) break;

		bool bObsolete = false;

		// Item steps become obsolete once the item is collected
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

/* OnItemCollected is overriden so we can complete the grab item step when player picks
   up an item . We still call Super first so the base GameMode does its normal increment
   of the item count and other stuff.
*/
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

/* GatherStepTriggers finds all the TutorialStepTrigger actors placed in the level and
   stores them in our list, then sorts them by thier StepIndex value so we know which
   order to run them in.
*/
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

	// Sorting triggers by StepIndex so they run in correct order 
	StepTriggers.Sort([](const ATutorialStepTrigger& A, const ATutorialStepTrigger& B)
		{
			return A.StepIndex < B.StepIndex;
		});

}

/* UpdateStepTriggerActivation enables only the trigger for current step and disables all
   others . This way only the right trigger responds to player overlap at any time.
*/
void ATutorialGameMode::UpdateStepTriggerActivation()
{
	for (int32 i = 0; i < StepTriggers.Num(); ++i)
	{
		ATutorialStepTrigger* Trigger = StepTriggers[i];
		if (!Trigger) continue;
		Trigger->SetTriggerActive(i == CurrentStepIndex);
	}
}

// Shows the message for the current step on the tutorial widget , flashing for exit step 
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
		// Flash the message only on the reach exit step to draw attention
		TutorialWidgetInstance->SetFlashing(Current->StepType == ETutorialStepType::ReachExit);
	}
}

/* TeleportPlayerToCheckpoint moves the player back to the last saved checkpoint location.
   We stop thier current movement first so they donot keep sliding after the teleport.
*/
void ATutorialGameMode::TeleportPlayerToCheckpoint()
{
	ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!PlayerChar) return;

	// Stoping any current movement so player doesnot slide after teleport
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

// Calls ResetToPatrol on every guard in the level so they all go back to patroling 
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