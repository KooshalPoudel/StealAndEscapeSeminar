/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar

File Name: GuardCharacter.cpp
Author: Kushal Poudel
Contributors: Alok Poudel (AI testing and gameplay integration)
Last Modified: March 1, 2026

Description : This is for creating a custom  Guard Character which is AI Controlled.
The guard is controlled by GuardAIController and is automatically possessed when placed in the world.
Mesh setup, animation blueprint assignment, movement configuration, and AI controller assignment
are custom implemented for enemy guard behavior.
*/

#include "GuardCharacter.h"
#include "GuardAIController.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Animation/AnimInstance.h"

AGuardCharacter::AGuardCharacter()
{
	// Engine setting to allow Tick to run every frame
	PrimaryActorTick.bCanEverTick = true;

	/* Custom Setup for AI Controller
	   GuardAIController is being assigned to control this character
	   AutoPossessAI makessure when placed or spawnedguard is automatically 
	   controlled when placed or spawned 
	*/
	AIControllerClass = AGuardAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// This is the Engine default capsule collision size as well as their rotation setting
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	/* Creating custom movement for enemy guard
	   Guard rotates toward movement direction
       Walk speed is reduced for patrol and controlled enemy behavior 
	*/
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->MaxWalkSpeed = 250.f;

	/* Custom Mesh Setup for Guard Character
	   Right now default mannequin is assigned for tesing phase later a 
	   Custom enemy AI character will be rigged and assigned here
	   Mesh position and rotation are adjusted to align properly with capsule 
	*/

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(
		TEXT("/Game/Mannequin/Character/Mesh/SK_Mannequin.SK_Mannequin")
	);

	if (MeshAsset.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshAsset.Object);
		GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
		GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	}

	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBP(
		TEXT("/Game/Mannequin/Animations/ThirdPerson_AnimBP.ThirdPerson_AnimBP_C")
	);

	if (AnimBP.Succeeded())
	{
		GetMesh()->SetAnimInstanceClass(AnimBP.Class);
	}
}

//This is Engine lifecycle method which is called automatically when game starts
void AGuardCharacter::BeginPlay()
{
	Super::BeginPlay();
}

// This is unreal Engine tick method whic is being called everyframe
void AGuardCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

/* This is Engine input setup method.
   Here Guard character does not use players input beacuse of being controlled automatically 
   GuardAIController class handles all the input beheviour
*/
void AGuardCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}