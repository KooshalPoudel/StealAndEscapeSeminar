/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar

File Name: GuardAIController.h
Author: Alok Poudel
Contributors: Kushal Poudel
Last Modified: March 1, 2026

Description: This is a GuardAIController class and this inherits from AIcontroller
GuardAIController handles the enemy guard’s AI behavior by 
the help of visual perception data which trigger response for movement if seen or lost
*/

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "GuardAIController.generated.h"

UCLASS()
class STEALANDESCAPE_API AGuardAIController : public AAIController
{
	GENERATED_BODY()

// Constructor setting  up perception components and basic vision configuration
public:
	AGuardAIController();

protected:
	virtual void OnPossess(APawn* InPawn) override;

private:
	/* Main AI Perception components which Handles sensing logic and processes perception stimuli
	   for this AI controller
	*/
	UPROPERTY()
		class UAIPerceptionComponent* PerceptionComp;

	UPROPERTY()
		class UAISenseConfig_Sight* SightConfig;

	UFUNCTION()
		void OnTargetDetected(AActor* Actor, FAIStimulus Stimulus);
};