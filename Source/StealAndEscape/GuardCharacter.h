/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar

File Name: GuardCharacter.h
Author:Alok Poudel
Last Modified: March 22, 2026
Description : Header file for custom AI Controlled Guard Character.
This class defines the structure of the enemy guard which inherits from ACharacter.
Actual AI behavior is implemented inside GuardAIController. patrol point array for waypoint-based patrol system.
Guards can now have unique patrol routes set in the level editor.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GuardCharacter.generated.h"

// Forward declaration to avoid duplicate symbol linker errors
class AGuardAIController;

/* Creating a Guard Character Class which Inherits from Unreal Engine ACharacter
   which is being Used as base structure for enemy AI guard
*/

UCLASS()
class STEALANDESCAPE_API AGuardCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	AGuardCharacter();

protected:

	virtual void BeginPlay() override;

public:

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/* Patrol Points Array  Set in the Level Editor
	   Each element is a world-space location that the guard will walk to in orders
	   After reaching the last point the guard loops back to the first point
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol", meta = (MakeEditWidget = "true"))
		TArray<FVector> PatrolPoints;

	/* Patrol speed how fast the guard walks while patrolling */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
		float PatrolSpeed = 150.f;

	/* Chase speed how fast the guard runs when chasing the player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
		float ChaseSpeed = 400.f;

	/* Wait time at each patrol point before moving to next one */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
		float PatrolWaitTime = 2.0f;
};
