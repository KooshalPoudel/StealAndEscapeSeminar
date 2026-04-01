/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
Template Used: Unreal Engine 4 TopDown C++ Template (Epic Games)
Original Template Author: Epic Games
File Name: StealAndEscapeCharacter.h
Modified By: Kushal Poudel and Alok Poudel
Last Modified: March 18, 2026
Description:StealAndEscapeCharacter is the player character header file based on Unreal Engine's 
            Top-Down C++ template. 
*/
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Animation/AnimMontage.h"
#include "StealAndEscapeCharacter.generated.h"

// Forward declaration so we do not need to include the full header here
class AStealableItem;

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

	// This reports the noise vent to the player ocation vis uai presception system
	UFUNCTION(BlueprintCallable, Category = "Noise")
		void MakeFootstepNoise();

	//This finds the closest valid item from the candidate list and collects it
	UFUNCTION(BlueprintCallable, Category = "Item")
		void CollectNearbyItem();

	// this is Called by StealableItem OnOverlapBegin
	void AddNearbyItem(AStealableItem* Item);
    void RemoveNearbyItem(AStealableItem* Item);
	bool HasNearbyItems() const;

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
		float WalkSpeed = 350.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
		float RunSpeed = 600.f;

	//this tracks all StealableItems currently overlapping the player
	TArray<AStealableItem*> NearbyItems;

	AStealableItem* GetClosestNearbyItem() const;
};
