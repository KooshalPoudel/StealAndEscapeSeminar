/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: GuardAIController.h
Author: Alok Poudel
Contributors: Kushal Poudel
Last Modified: March 16, 2026
Description: GuardAIController handles the enemy guard's AI behavior using
Unreal Engine's AI Perception system. This class controls how the guard recognizes the player,
interprets visual and audio cues, and reacts to visibility changes.

Updated: Added patrol state machine with three states (Patrolling, Chasing, Investigating).
Guards now walk between patrol waypoints, chase on sight, investigate on hearing,
and return to their nearest patrol point after losing the player.
*/
#pragma once
#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "GuardAIController.generated.h"

/* Enum for guard behavior states
   Patrolling = walking between patrol waypoints
   Chasing = actively pursuing the player (sight detection)
   Investigating = moving to a heard noise location then returning to patrol
*/
UENUM(BlueprintType)
enum class EGuardState : uint8
{
	Patrolling,
	Chasing,
	Investigating
};

UCLASS()
class STEALANDESCAPE_API AGuardAIController : public AAIController
{
	GENERATED_BODY()

public:
	AGuardAIController();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaSeconds) override;

private:
	/* AI Perception Components */
	UPROPERTY()
		class UAIPerceptionComponent* PerceptionComp;
	UPROPERTY()
		class UAISenseConfig_Sight* SightConfig;
	UPROPERTY()
		class UAISenseConfig_Hearing* HearingConfig;

	UFUNCTION()
		void OnTargetDetected(AActor* Actor, FAIStimulus Stimulus);

	/* Patrol System Variables */

	// Current behavior state of the guard
	EGuardState CurrentState = EGuardState::Patrolling;

	// Index of the current patrol point the guard is walking toward
	int32 CurrentPatrolIndex = 0;

	// Whether the guard is currently waiting at a patrol point
	bool bIsWaiting = false;

	// Timer handle for the wait delay at patrol points
	FTimerHandle PatrolWaitTimerHandle;

	/* Patrol System Methods */

	// Moves the guard to the next patrol waypoint in the array
	void MoveToNextPatrolPoint();

	// Called when the wait timer finishes at a patrol point
	void OnPatrolWaitFinished();

	// Finds the closest patrol point index and starts moving there
	void ReturnToPatrol();

	// How close the guard needs to be to a patrol point to consider it reached (in unreal units)
	float PatrolAcceptanceRadius = 150.f;
};