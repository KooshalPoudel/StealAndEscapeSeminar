/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
Template Used: Unreal Engine 4 TopDown C++ Template (Epic Games)
Original Template Author: Epic Games
File Name: StealAndEscapeCharacter.h
Modified By: Kushal Poudel and Alok Poudel
Last Modified: March 18, 2026
This header file is based on the Unreal Engine TopDown template.
Camera components and cursor decal are template-generated.
WASD movement, Sprint system, and Grab animation are custom implemented.

Updated: Added candidate item list for the manual grab system.
The player character keeps a TArray of all StealableItems whose sphere collisions
the player is currently overlapping. This handles the case where multiple items
are in range at the same time. When the player presses G and at least one candidate
exists the grab animation plays. When the grab animation notify fires at the
hand-contact frame CollectNearbyItem() finds the closest valid candidate item
and collects it. This ensures the player always grabs the nearest item.

Grab system flow:
  overlap  = mark item as candidate (add to NearbyItems list)
  G key    = request grab (play animation if candidates exist)
  notify   = commit pickup (collect closest candidate)
  item     = award score / notify GameMode / destroy self

AddNearbyItem() and RemoveNearbyItem() are called by StealableItem overlap events
to add and remove items from the candidate list as the player walks near them.
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

	/* Function to generate footstep noise for the AI hearing system
	   This is called from the animation notify on the running animation
	   when the foot touches the ground. It reports a noise event that the
	   guard AI perception system can detect using UAISense_Hearing
	*/
	UFUNCTION(BlueprintCallable, Category = "Noise")
		void MakeFootstepNoise();

	/* Called by the grab animation notify (AnimNotify_GrabItem) at the frame
	   where the hand touches the item. Finds the closest valid item from the
	   candidate list and tells it to collect itself which notifies the GameMode
	   and destroys the item actor
	*/
	UFUNCTION(BlueprintCallable, Category = "Item")
		void CollectNearbyItem();

	/* Called by StealableItem OnOverlapBegin to add an item to the candidate list
	   when the player enters that item's sphere collision
	*/
	void AddNearbyItem(AStealableItem* Item);

	/* Called by StealableItem OnOverlapEnd to remove an item from the candidate list
	   when the player leaves that item's sphere collision
	*/
	void RemoveNearbyItem(AStealableItem* Item);

	/* Returns true if there is at least one candidate item in range
	   Used by GrabPressed to decide whether to play the grab animation
	*/
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

	/* List of all stealable items whose sphere collisions the player is currently inside
	   Multiple items can be in range at the same time. When the grab notify fires
	   we find the closest item in this list and collect that one.
	   Items add themselves on overlap begin and remove themselves on overlap end.
	   Not a UPROPERTY because items manage their own lifetime through Destroy()
	   and we remove them from this list before they are destroyed
	*/
	TArray<AStealableItem*> NearbyItems;

	/* Helper function that finds the closest item in the NearbyItems list
	   based on distance from the player's current location
	   Returns nullptr if the list is empty
	*/
	AStealableItem* GetClosestNearbyItem() const;
};