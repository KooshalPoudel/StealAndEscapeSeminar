/*
Project Name: Steal and Escape
Description: A 3D top-down stealth / escape game developed in Unreal Engine 4.27
Course: CSCI 491 Seminar
File Name: GuardAIController.cpp
Author: Alok Poudel
Contributor: Kushal Poudel
Last Modified: March 16, 2026
Description: GuardAIController is responsible for controlling the enemy guard's behavior using
Unreal Engine's AI Perception system. This class controls how the guard recognizes the player,
interprets visual and audio cues, and reacts to visibility changes.

Updated: Implemented full patrol state machine with three states:
- Patrolling: Guard walks between waypoints set in the level editor
- Chasing: Guard sprints toward the player when detected by sight
- Investigating: Guard walks to a heard noise location, then returns to patrol
Guards now return to the nearest patrol point after losing the player.
*/
#include "GuardAIController.h"
#include "GuardCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISense_Hearing.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "TimerManager.h"

AGuardAIController::AGuardAIController()
{
	// Creating AI Perception component which allows guard to sense actors in the environment 
	PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));
	SetPerceptionComponent(*PerceptionComp);

	// ---- Sight Configuration ----
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 800.f;
	SightConfig->LoseSightRadius = 1000.f;
	SightConfig->PeripheralVisionAngleDegrees = 60.f;
	SightConfig->SetMaxAge(1.0f);
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	PerceptionComp->ConfigureSense(*SightConfig);
	PerceptionComp->SetDominantSense(UAISense_Sight::StaticClass());

	// ---- Hearing Configuration ----
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	HearingConfig->HearingRange = 350.f;
	HearingConfig->SetMaxAge(3.0f);
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	PerceptionComp->ConfigureSense(*HearingConfig);

	PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &AGuardAIController::OnTargetDetected);
}

/* OnPossess is called when this AI controller takes control of a pawn.
   We start the patrol behavior here because the guard character and its
   patrol points are now available to us.
*/
void AGuardAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Start patrolling once the controller possesses the guard
	AGuardCharacter* Guard = Cast<AGuardCharacter>(InPawn);
	if (Guard && Guard->PatrolPoints.Num() > 0)
	{
		CurrentState = EGuardState::Patrolling;
		Guard->GetCharacterMovement()->MaxWalkSpeed = Guard->PatrolSpeed;
		CurrentPatrolIndex = 0;
		MoveToNextPatrolPoint();
	}
}

/* Tick is called every frame. We use it to check if the guard has reached
   its current patrol point so we can trigger the wait timer and then move
   to the next point. We also check if the guard reached an investigation
   location so it can return to patrol.
*/
void AGuardAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return;

	AGuardCharacter* Guard = Cast<AGuardCharacter>(ControlledPawn);
	if (!Guard) return;

	if (CurrentState == EGuardState::Patrolling && !bIsWaiting)
	{
		// Check if guard has reached the current patrol point
		if (Guard->PatrolPoints.IsValidIndex(CurrentPatrolIndex))
		{
			// Using Dist2D to ignore Z height difference between guard and patrol point
			float DistToPoint = FVector::Dist2D(Guard->GetActorLocation(), Guard->PatrolPoints[CurrentPatrolIndex]);
			if (DistToPoint <= PatrolAcceptanceRadius)
			{
				// Guard reached the patrol point, start waiting
				bIsWaiting = true;
				StopMovement();

				// Set a timer to wait at this point before moving to the next one
				GetWorld()->GetTimerManager().SetTimer(
					PatrolWaitTimerHandle,
					this,
					&AGuardAIController::OnPatrolWaitFinished,
					Guard->PatrolWaitTime,
					false
				);
			}
		}
	}
	else if (CurrentState == EGuardState::Investigating)
	{
		// Check if guard reached the investigation location
		EPathFollowingStatus::Type MoveStatus = GetMoveStatus();
		if (MoveStatus != EPathFollowingStatus::Moving)
		{
			// Guard reached the noise location or could not reach it, return to patrol
			ReturnToPatrol();
		}
	}
}

/* OnTargetDetected is triggered whenever the perception system updates.
   Sight detection: Chase the player directly at chase speed
   Hearing detection: Investigate the noise location at patrol speed
   Lost sight: Return to nearest patrol point
*/
void AGuardAIController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
	ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!PlayerChar) return;

	if (Actor != PlayerChar) return;

	AGuardCharacter* Guard = Cast<AGuardCharacter>(GetPawn());
	if (!Guard) return;

	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			// Player spotted! Switch to chase state
			CurrentState = EGuardState::Chasing;

			// Clear any patrol wait timer that might be active
			GetWorld()->GetTimerManager().ClearTimer(PatrolWaitTimerHandle);
			bIsWaiting = false;

			// Increase speed to chase speed and pursue the player
			Guard->GetCharacterMovement()->MaxWalkSpeed = Guard->ChaseSpeed;
			UAIBlueprintHelperLibrary::SimpleMoveToActor(this, PlayerChar);
		}
		else
		{
			// Lost sight of player, return to patrol
			ReturnToPatrol();
		}
	}
	else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			// Only investigate sound if not already chasing the player visually
			if (CurrentState != EGuardState::Chasing)
			{
				CurrentState = EGuardState::Investigating;

				// Clear any patrol wait timer
				GetWorld()->GetTimerManager().ClearTimer(PatrolWaitTimerHandle);
				bIsWaiting = false;

				// Move to the noise location at a cautious speed (between patrol and chase)
				Guard->GetCharacterMovement()->MaxWalkSpeed = Guard->PatrolSpeed * 1.5f;
				FVector NoiseLocation = Stimulus.StimulusLocation;
				UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, NoiseLocation);
			}
		}
	}
}

/* MoveToNextPatrolPoint makes the guard walk to the current patrol point
   in the PatrolPoints array. After the guard reaches it (checked in Tick),
   it waits for PatrolWaitTime seconds, then advances to the next index.
   The patrol loops back to index 0 after reaching the last point.
*/
void AGuardAIController::MoveToNextPatrolPoint()
{
	AGuardCharacter* Guard = Cast<AGuardCharacter>(GetPawn());
	if (!Guard) return;

	// If no patrol points are set, do nothing
	if (Guard->PatrolPoints.Num() == 0) return;

	// Make sure the index is valid (wrap around if needed)
	CurrentPatrolIndex = CurrentPatrolIndex % Guard->PatrolPoints.Num();

	// Move to the current patrol point
	FVector Destination = Guard->PatrolPoints[CurrentPatrolIndex];
	UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, Destination);
}

/* OnPatrolWaitFinished is called when the patrol wait timer expires.
   It advances the patrol index to the next point and starts moving.
*/
void AGuardAIController::OnPatrolWaitFinished()
{
	bIsWaiting = false;

	AGuardCharacter* Guard = Cast<AGuardCharacter>(GetPawn());
	if (!Guard) return;

	// Advance to the next patrol point (loops back to 0 at the end)
	CurrentPatrolIndex = (CurrentPatrolIndex + 1) % Guard->PatrolPoints.Num();
	MoveToNextPatrolPoint();
}

/* ReturnToPatrol finds the closest patrol point to the guard's current location
   and starts moving there. This is called when the guard loses sight of the player
   or finishes investigating a noise. The guard's speed is reset to patrol speed.
*/
void AGuardAIController::ReturnToPatrol()
{
	AGuardCharacter* Guard = Cast<AGuardCharacter>(GetPawn());
	if (!Guard) return;

	// If no patrol points exist, just stop
	if (Guard->PatrolPoints.Num() == 0)
	{
		CurrentState = EGuardState::Patrolling;
		StopMovement();
		return;
	}

	// Find the closest patrol point to the guard's current position
	float ClosestDist = FLT_MAX;
	int32 ClosestIndex = 0;

	for (int32 i = 0; i < Guard->PatrolPoints.Num(); i++)
	{
		// Using Dist2D to ignore Z height difference between guard and patrol point
		float Dist = FVector::Dist2D(Guard->GetActorLocation(), Guard->PatrolPoints[i]);
		if (Dist < ClosestDist)
		{
			ClosestDist = Dist;
			ClosestIndex = i;
		}
	}

	// Set state back to patrolling and reset speed
	CurrentState = EGuardState::Patrolling;
	CurrentPatrolIndex = ClosestIndex;
	Guard->GetCharacterMovement()->MaxWalkSpeed = Guard->PatrolSpeed;

	bIsWaiting = false;
	MoveToNextPatrolPoint();
}