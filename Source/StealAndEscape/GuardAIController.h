/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: GuardAIController.h
Author: Alok Poudel & Kushal Poudel
Last Modified: March 22, 2026
 
Description: GuardAIController is responsible for controlling the enemy guard's behavior using Unreal Engine's
             AI Perception system. This class controls how the guard recognizes the player, interprets visual 
			 and audio cues, and reacts accordingly using a three-state behavior machine. In the Patrolling state 
			 the guard walks between waypoints set in the level editor and waits briefly at each one before continuing. 
			 In the Chasing state the guard sprints directly toward the player when detected by sight, and if it gets 
			 within CatchDistance it stops and calls GameMode->OnPlayerCaught() to trigger the lose condition. 
			 In the Investigating state the guard walks to the location of a heard noise before returning to the 
			 nearest patrol point and resuming normal patrol. Guards always return to their nearest patrol point after 
			 losing the player or finishing an investigation.
*/
#pragma once
#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "GuardAIController.generated.h"

/* guard behavior states
   Patrolling is walking between patrol waypoints
   Chasing is actively pursuing the player sight detection
   Investigating is moving to a heard noise location then returning to patrol
*/
UENUM(BlueprintType)
enum class EGuardState : uint8
{
	Patrolling,
	Chasing,
	Investigating
};
/* AGuardAIController inherits from AAIController which is Unreal's built-in base class
   for AI controllers.*/
UCLASS()
class STEALANDESCAPE_API AGuardAIController : public AAIController
{
	GENERATED_BODY()

public:
	AGuardAIController();

	UFUNCTION(BlueprintCallable, Category = "Guard")
		void ResetToPatrol();

protected:
    //this is called by the engine when this controller takes control of a pawn
	virtual void OnPossess(APawn* InPawn) override;
    // this tick is called every frame by the engine we use it to poll conditions that cannot be driven purely by events
    
	virtual void Tick(float DeltaSeconds) override;

private:
	/* This is for AI Perception Components */
	UPROPERTY()
		class UAIPerceptionComponent* PerceptionComp;
	UPROPERTY()
		class UAISenseConfig_Sight* SightConfig;
	UPROPERTY()
		class UAISenseConfig_Hearing* HearingConfig;

	UFUNCTION()
		void OnTargetDetected(AActor* Actor, FAIStimulus Stimulus);

	/* This Patrol System Variables */

	// this is foe current behavior state of the guard
	EGuardState CurrentState = EGuardState::Patrolling;

	// this is index of the current patrol point the guard is walking toward
	int32 CurrentPatrolIndex = 0;

	// this is to know if the guard is currently waiting at a patrol point
	bool bIsWaiting = false;

	// this handles timer cancel to switch between patrol and chasing
	FTimerHandle PatrolWaitTimerHandle;

	/* Belows this is for the guard to move between patrol points*/

	// this moves the guard to the next patrol waypoint in the array
	void MoveToNextPatrolPoint();

	// this is called when the wait timer finishes at a patrol point
	void OnPatrolWaitFinished();

	// this finds the closest patrol point index and starts moving there
	void ReturnToPatrol();

	// this is to know how close the guard has to be near the patrol points to know it reached there
	float PatrolAcceptanceRadius = 150.f;

	/* Below this is catch System Variables */

	// this shows how close the guard has to be to know the player has been caught
	float CatchDistance = 100.f;

	// this prevents from calling the OnPlayerCaught() multiple times while is in distance of catching  player
	bool bHasCaughtPlayer = false;
};
