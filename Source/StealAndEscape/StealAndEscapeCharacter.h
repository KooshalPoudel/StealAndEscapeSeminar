/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
Template Used: Unreal Engine 4 TopDown C++ Template (Epic Games)
Original Template Author: Epic Games

File Name: StealAndEscapeCharacter.h
Modified By: Kushal Poudel and Alok Poudel 
Last Modified: March 1, 2026

This header file is based on the Unreal Engine TopDown template.
Camera components and cursor decal are template-generated.
WASD movement, Sprint system, and Grab animation are custom implemented.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Animation/AnimMontage.h"
#include "StealAndEscapeCharacter.generated.h"

UCLASS(Blueprintable)
class AStealAndEscapeCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AStealAndEscapeCharacter();

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Getters used by PlayerController Which are being used 
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UDecalComponent* GetCursorToWorld() const { return CursorToWorld; }

protected:
	// WASD movement -Alok
	void MoveForward(float Value);
	void MoveRight(float Value);

	// Shift to run- Alok
	void StartRun();
	void StopRun();

	//Grab (G key)  is pressed character plays Grab animation montage-- Kushal
	void GrabPressed();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
		UAnimMontage* GrabMontage = nullptr;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* TopDownCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UDecalComponent* CursorToWorld;

	float WalkSpeed = 350.f;
	float RunSpeed = 600.f;
};