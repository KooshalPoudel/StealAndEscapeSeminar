/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar

File Name: GuardCharacter.h
Author: Kushal Poudel
Contributors: Alok Poudel (AI testing and gameplay integration)
Last Modified: March 1, 2026

Description : Header file for custom AI Controlled Guard Character.
This class defines the structure of the enemy guard which inherits from ACharacter.
Actual AI behavior is implemented inside GuardAIController.
*/

#pragma once

#include "GuardAIController.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GuardCharacter.generated.h"

/* Creating a  Guard Character Classwhich Inherits from Unreal Engine ACharacter
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

};
