/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: TutorialStepTrigger.h
Author: Kushal Poudel and Alok Poudel
Last Modified: April 24, 2026

Description: Actor placed in the tutorial level representing one tutorial
step. Carries step index, type, message, optional per-step success message
and sound, and (for PressKey steps) the key name to watch for.

Removed the EditorMesh visualization component because it was causing
Map Check errors about NULL StaticMesh and DrawScale3D issues. The box
collider already draws an orange wireframe when the actor is selected
so you can see it fine in the editor without needing a separate mesh.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TutorialGameMode.h"
#include "TutorialStepTrigger.generated.h"

class UBoxComponent;
class USoundBase;

UCLASS()
class STEALANDESCAPE_API ATutorialStepTrigger : public AActor
{
	GENERATED_BODY()

public:
	ATutorialStepTrigger();

	/* Order of this step. 0 = first, 1 = second, etc. */
	UPROPERTY(EditAnywhere, Category = "Tutorial")
		int32 StepIndex = 0;

	/* How this step completes */
	UPROPERTY(EditAnywhere, Category = "Tutorial")
		ETutorialStepType StepType = ETutorialStepType::PressKey;

	/* Instruction message shown when this step becomes active */
	UPROPERTY(EditAnywhere, Category = "Tutorial", meta = (MultiLine = true))
		FString StepMessage = TEXT("");

	/* Success message shown briefly after this step completes, before
	   advancing. Leave empty to use the GameMode's DefaultSuccessMessage.
	*/
	UPROPERTY(EditAnywhere, Category = "Tutorial", meta = (MultiLine = true))
		FString SuccessMessage = TEXT("");

	/* Optional per-step success sound. Leave empty to use the GameMode default. */
	UPROPERTY(EditAnywhere, Category = "Tutorial")
		USoundBase* CustomSuccessSound = nullptr;

	/* Only used when StepType == PressKey. Valid values: "W", "A", "S", "D",
	   "G", "SpaceBar", "LeftShift" etc. Use capital letters for letter keys.
	*/
	UPROPERTY(EditAnywhere, Category = "Tutorial", meta = (EditCondition = "StepType == ETutorialStepType::PressKey"))
		FName KeyToPress = TEXT("W");

	void SetTriggerActive(bool bActive);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
		UBoxComponent* TriggerBox;

	UFUNCTION()
		void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
			bool bFromSweep, const FHitResult& SweepResult);
};