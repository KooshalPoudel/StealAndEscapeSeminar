/*
Project Name: Steal and Escape
Description: A 3D top-down stealth / escape game developed in Unreal Engine 4.27
Course: CSCI 491 Seminar
File Name: GuardAIController.cpp
Author: Alok Poudel & Kushal Poudel
Last Modified: March 22, 2026
Description: GuardAIController is responsible for controlling the enemy guard's behavior using Unreal Engine's AI Perception system, 
             managing how the guard recognizes the player through sight and hearing and reacts using a three-state behavior machine. 
			 In the Patrolling state the guard walks between waypoints set in the level editor and returns to the nearest patrol point 
			 after losing the player. In the Chasing state the guard sprints toward the player when detected by sight, and when it gets 
			 within CatchDistance it stops and calls GameMode->OnPlayerCaught() to trigger the lose condition, with the bHasCaughtPlayer 
			 flag preventing this from firing multiple times. In the Investigating state the guard walks to the location of a heard noise 
			 and then returns to patrol.
*/
#include "GuardAIController.h"
#include "GuardCharacter.h"
#include "StealAndEscapeGameMode.h"
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

	// Sight Configuration we can change this anytime as we like
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 500.f;
	SightConfig->LoseSightRadius = 700.f;
	SightConfig->PeripheralVisionAngleDegrees = 60.f;
	SightConfig->SetMaxAge(1.0f);
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	PerceptionComp->ConfigureSense(*SightConfig);
	PerceptionComp->SetDominantSense(UAISense_Sight::StaticClass());

	// Hearing Configuration we cange change is as we like anytime
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	HearingConfig->HearingRange = 500.f; //original 350.f
	HearingConfig->SetMaxAge(3.0f);
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	PerceptionComp->ConfigureSense(*HearingConfig);

	PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &AGuardAIController::OnTargetDetected);
}

/* OnPossess is called when this AI controller takes control of a pawn
   We start the patrol behavior here because the guard character and its
   patrol points are now available to us
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

/* Tick, Chasing state check When chasing the player we check
   the distance between guard and player every frame If the distance is
   less than CatchDistance the guard catches the player and the game ends.\
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
	else if (CurrentState == EGuardState::Chasing)
	{
		/* Catch Logic Check if the guard is close enough to catch the player
		   Using Dist2D to ignore Z height difference between guard and player
		   If within CatchDistance the guard stops and tells GameMode the player was caught
		   bHasCaughtPlayer prevents this from being called multiple times per chase
		*/
		if (!bHasCaughtPlayer)
		{
			ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
			if (PlayerChar)
			{
				float DistToPlayer = FVector::Dist2D(Guard->GetActorLocation(), PlayerChar->GetActorLocation());
				if (DistToPlayer <= CatchDistance)
				{
					// Player caught! Stop guard movement and trigger lose condition
					bHasCaughtPlayer = true;
					StopMovement();

					// Getting GameMode to call OnPlayerCaught which handles the lose condition
					AStealAndEscapeGameMode* GM = Cast<AStealAndEscapeGameMode>(
						UGameplayStatics::GetGameMode(GetWorld()));
					if (GM)
					{
						GM->OnPlayerCaught();
					}
				}
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

/* OnTargetDetected is triggered whenever the perception system updates
   Sight detection Chase the player directly at chase speed
   Hearing detection Investigate the noise location at patrol speed
   Lost sight Return to nearest patrol point
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
   in the PatrolPoints array. After the guard reaches it checked in Tick,
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
