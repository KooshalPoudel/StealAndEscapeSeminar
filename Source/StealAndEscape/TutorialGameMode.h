/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: TutorialGameMode.h
Author: Kushal Poudel and Alok Poudel
Last Modified: April 24, 2026

Description: Tutorial GameMode inheriting from StealAndEscapeGameMode.

Key design: the tutorial ENFORCES step order using three mechanisms:
  1. Player input is disabled during success/caught/transition messages
	 so the player cannot move during the 1.5 second delays.
  2. Guards are disabled (frozen at spawn, no chase, no patrol) during the
	 movement-teaching steps (0-4). They only "wake up" when the player
	 reaches step 5 (the guard warning). This prevents the guard from
	 chasing during the W/A/S/D tutorial.
  3. The player's input is disabled BEFORE each step starts and RE-enabled
	 only after the step message has been shown. This guarantees the
	 player sees every step's instructions.
*/

#pragma once

#include "CoreMinimal.h"
#include "StealAndEscapeGameMode.h"
#include "TutorialGameMode.generated.h"

class ATutorialStepTrigger;
class UTutorialWidget;
class USoundBase;
class AGuardCharacter;

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

	virtual void OnPlayerCaught() override;
	virtual void OnItemCollected() override;

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
		int32 GetCurrentStepIndex() const { return CurrentStepIndex; }

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
		bool IsCurrentStep(int32 StepIndex) const { return StepIndex == CurrentStepIndex; }

	UPROPERTY(EditDefaultsOnly, Category = "Tutorial")
		TSubclassOf<UTutorialWidget> TutorialWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Tutorial")
		float HideDurationRequired = 2.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Tutorial")
		float SuccessMessageDuration = 1.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Tutorial")
		float CaughtMessageDuration = 2.0f;

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

	/* Step index at which guards are activated. Before this step the guards
	   are frozen in place. Set this to the first step that involves the
	   guard (usually step 5 - the guard warning step).
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Tutorial")
		int32 GuardActivationStep = 5;

private:
	int32 CurrentStepIndex = 0;
	FTransform LastCheckpointTransform;
	bool bIsBeingChased = false;
	float HideTimeAccumulated = 0.f;
	bool bShowingInterstitialMessage = false;
	float EscapeStepActiveTime = 0.f;
	bool bHasShownSpottedMessage = false;
	bool bGuardChaseActive = false;

	// True when guards have been activated. Used so we only activate them
	// once and don't keep re-enabling every frame.
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

	// Disables all player movement input. Called during messages/transitions.
	void FreezePlayer();

	// Re-enables player movement input.
	void UnfreezePlayer();

	// Freezes all guards (sets MaxWalkSpeed to 0 and stops their movement).
	// Called at BeginPlay and during interstitial messages.
	void FreezeGuards();

	// Unfreezes guards and sets them up for patrol. Called when the player
	// reaches GuardActivationStep for the first time.
	void ActivateGuards();
};