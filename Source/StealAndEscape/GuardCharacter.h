// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GuardAIController.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GuardCharacter.generated.h"

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
