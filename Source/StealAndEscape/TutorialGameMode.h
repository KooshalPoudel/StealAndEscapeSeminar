/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar

File Name: TutorialGameMode.h
Author: Kushal Poudel and Alok Poudel
Last Modified: April 26, 2026

Description: Tutorial GameMode inheriting from StealAndEscapeGameMode . The tutorial enforces
			 step order using three mechanisms:
			   a. Player input is disabled durring success/caught/transition messages so the
				  player cant move during the 1.5 second delays.
			   b. Guards are disabled (frozen at spawn , no chase , no patrol) during the
				  movement teaching steps (0 to 4) . They only wake up when the player reaches
				  step 5 which is the guard warning step . This prevents the guard from chasing
				  during the W/A/S/D tutorial.
			   c. Player input is disabled BEFORE each step starts and re-enabled only after the
				  step message has been shown . This gaurantees the player sees every step's
				  instructions.
*/

#pragma once

#include "CoreMinimal.h"
#include "StealAndEscapeGameMode.h"
#include "TutorialGameMode.generated.h"

class ATutorialStepTrigger;
class UTutorialWidget;
class USoundBase;
class AGuardCharacter;

// Different types of tutorial steps , each one has its own completion check in Tick
UENUM(BlueprintType)
enum class ETutorialStepType : uint8
{
	PressKey       UMETA(DisplayName = "Press Key"),
	ReachArea      UMETA(DisplayName = "Reach Area"),
	ApproachItem   UMETA(DisplayName = "Approach Item"),
	GrabItem       UMETA(DisplayName = "Grab Item"),
	EscapeGuard    UMETA(DisplayName = "Escape Guard"),
	ReachExit      UMETA(DisplayName = "Reach Exit")
};

UCLASS()
class STEALANDESCAPE_API ATutorialGameMode : public AStealAndEscapeGameMode
{
	GENERATED_BODY()

public:
	ATutorialGameMode();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
		void CompleteCurrentStep();

	// Overriden so caught player teleports back to checkpoint instead of ending the game
	virtual void OnPlayerCaught() override;
	// Overriden so we can complete the grab item step when player picks up item 
	virtual void OnItemCollected() override;

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
		int32 GetCurrentStepIndex() const { return CurrentStepIndex; }

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
		bool IsCurrentStep(int32 StepIndex) const { return StepIndex == CurrentStepIndex; }

	UPROPERTY(EditDefaultsOnly, Category = "Tutorial")
		TSubclassOf<UTutorialWidget> TutorialWidgetClass;

	// How long player must hide from guard before escape step is considered complete
	UPROPERTY(EditDefaultsOnly, Category = "Tutorial")
		float HideDurationRequired = 2.0f;

	// How long the success message stays on screen before advancing to next step
	UPROPERTY(EditDefaultsOnly, Category = "Tutorial")
		float SuccessMessageDuration = 1.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Tutorial")
		float CaughtMessageDuration = 2.0f;

	// Time before escape step auto completes if no guard ever started chasing 
	UPROPERTY(EditDefaultsOnly, Category = "Tutorial")
		float EscapeGuardGracePeriod = 5.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Tutorial")
		FString DefaultSuccessMessage = TEXT("Nice job!");

	UPROPERTY(EditDefaultsOnly, Category = "Tutorial")
		FString CaughtMessage = TEXT("Busted! The guard caught you. Try again.");

	UPROPERTY(EditDefaultsOnly, Category = "Tutorial")
		FString GuardSpottedMessage = TEXT("Guard saw you! Hold Shift + W to sprint and hide!");

	UPROPERTY(EditDefaultsOnly, Category = "Tutorial")
		FString ItemAlreadyGrabbedMessage = TEXT("You already have the coin! Now escape.");

	UPROPERTY(EditDefaultsOnly, Category = "Tutorial")
		USoundBase* StepCompleteSound = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Tutorial")
		USoundBase* CaughtSound = nullptr;

	/* Step index at which guards are activated . Before this step the guards are frozen in place .
	   Set this to the first step that involves the guard (usually step 5 - the guard warning step).
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Tutorial")
		int32 GuardActivationStep = 5;

private:
	// Tutorial state tracking variables
	int32 CurrentStepIndex = 0;
	FTransform LastCheckpointTransform;
	bool bIsBeingChased = false;
	float HideTimeAccumulated = 0.f;
	bool bShowingInterstitialMessage = false;
	float EscapeStepActiveTime = 0.f;
	bool bHasShownSpottedMessage = false;
	bool bGuardChaseActive = false;

	// True when guards have been activated . Used so we only activate them once and donot keep
	// re-enabling them every frame in Tick
	bool bGuardsActivated = false;

	FTimerHandle StepAdvanceTimerHandle;
	FTimerHandle CaughtTeleportTimerHandle;

	UPROPERTY()
		UTutorialWidget* TutorialWidgetInstance = nullptr;

	UPROPERTY()
		TArray<ATutorialStepTrigger*> StepTriggers;

	void GatherStepTriggers();
	void UpdateStepTriggerActivation();
	void ShowCurrentStepMessage();
	void TeleportPlayerToCheckpoint();
	void ResetGuardsToPatrol();
	bool CheckKeyPressForCurrentStep();
	void AdvanceToNextStep();
	void PerformCaughtTeleport();
	bool IsPlayerNearAnyItem() const;
	bool SkipObsoleteSteps();
	bool IsAnyGuardChasing() const;
	void OnEnterStep();

	// Disables all player movement input , called during messages and transitions
	void FreezePlayer();

	// Re-enables player movement input
	void UnfreezePlayer();

	/* Freezes all guards by setting MaxWalkSpeed to 0 and stoping thier movement.
	   Called at BeginPlay and during interstitial messages so guards donot move
	   while player is reading the messages.
	*/
	void FreezeGuards();

	/* Unfreezes guards and sets them up for patrol. Called when the player reaches
	   GuardActivationStep for the first time so guards start patroling at step 5.
	*/
	void ActivateGuards();
};