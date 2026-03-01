#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "GuardAIController.generated.h"

UCLASS()
class STEALANDESCAPE_API AGuardAIController : public AAIController
{
	GENERATED_BODY()

public:
	AGuardAIController();

protected:
	virtual void OnPossess(APawn* InPawn) override;

private:
	UPROPERTY()
		class UAIPerceptionComponent* PerceptionComp;

	UPROPERTY()
		class UAISenseConfig_Sight* SightConfig;

	UFUNCTION()
		void OnTargetDetected(AActor* Actor, FAIStimulus Stimulus);
};