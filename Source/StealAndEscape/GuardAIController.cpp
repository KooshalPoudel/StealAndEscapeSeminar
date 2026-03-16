/*
Project Name: Steal and Escape
Description: A 3D top-down stealth / escape game developed in Unreal Engine 4.27
Course: CSCI 491 Seminar
File Name: GuardAIController.cpp
Author: Alok Poudel
Contributor: Kushal Poudel
Last Modified: March 1, 2026
Description:GuardAIController is responsible for controlling the enemy guard's behavior using
Unreal Engine's AI Perception system. This class controls how the guard recognises the player,
interprets visual cues, and reacts to visibility changes.
*/
#include "GuardAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"
// Including hearing sense headers for audio-based detection
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISense_Hearing.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "GameFramework/Character.h"
/* Constructor for  this class which initializes AI Preception  and being assigned to
   this controller. Basic sight based detection is configured as primary detection

*/
AGuardAIController::AGuardAIController()
{
	// Creating AI Perception component which allows guard to sense actor in the environment 
	PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));
	// Registering the component just created as primary sensing system
	SetPerceptionComponent(*PerceptionComp);
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	/*These are the vision parameter for the guard which helps for detection
	  SightRadius = how far the guard can see the player
	  LoseSightRadius = how far the player can travel  before guard looses it
	  PeripheralVisionAngleDegrees = Guard field of view
	  MaxAge = how long can the guard remember the detected player
	*/
	SightConfig->SightRadius = 800.f;
	SightConfig->LoseSightRadius = 1000.f;
	SightConfig->PeripheralVisionAngleDegrees = 60.f;
	SightConfig->SetMaxAge(1.0f);
	/* Setting detection true for enemies, friendlies, and neutral actors.
	   Doing this will alow guard to sense all actor affiliations
	*/
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	/* Here the sight configuration is applied as set sight as dominant perceprion
	   OnTargetDetected is where bindpreception gets updated so guard will react
	   dynamically if a target is seen or lost
	*/
	PerceptionComp->ConfigureSense(*SightConfig);
	PerceptionComp->SetDominantSense(UAISense_Sight::StaticClass());

	/* Hearing configuration for the guard AI
	   HearingRange = how far the guard can hear noise events (in unreal units)
	   MaxAge = how long the guard remembers a heard sound before forgetting it
	   DetectionByAffiliation = which actor affiliations the guard can hear
	*/
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	HearingConfig->HearingRange = 350.f;
	HearingConfig->SetMaxAge(3.0f);
	/* Setting detection true for enemies, friendlies, and neutral actors.
	   This ensures the guard can hear noise from all actor types including the player
	*/
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	// Registering the hearing sense with the perception component
	PerceptionComp->ConfigureSense(*HearingConfig);

	PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &AGuardAIController::OnTargetDetected);
}
// Engine method called when this AI controller takes possession of a pawn.
void AGuardAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}
//This is the Target Detection Logic  which is triggered whenever the perception system updates.
void AGuardAIController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
	ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	// Checking if player exists if not return which avoid null reference
	if (!PlayerChar) return;
	/* If the detected actor is the player, then chase behavior is initialized  when successfully sensed
	   Else stopping the movement when player is no longer visible
	*/
	if (Actor == PlayerChar)
	{
		/* Checking if the stimulus came from sight or hearing sense
		   Sight: guard directly chases the player actor
		   Hearing: guard investigates by moving to the location where the sound was heard
		*/
		if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
		{
			// Sight detection: chase or stop based on whether the player is still visible
			if (Stimulus.WasSuccessfullySensed())
			{
				UAIBlueprintHelperLibrary::SimpleMoveToActor(this, PlayerChar);
			}
			else
			{
				StopMovement();
			}
		}
		else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
		{
			/* Hearing detection: the guard heard a noise from the player
			   Instead of chasing the player directly, the guard moves to the sound location
			   This creates an investigation behavior where the guard checks where the noise came from
			*/
			if (Stimulus.WasSuccessfullySensed())
			{
				FVector NoiseLocation = Stimulus.StimulusLocation;
				UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, NoiseLocation);
			}
		}
	}
}