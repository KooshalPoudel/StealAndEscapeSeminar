/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar

File Name: StealableItem.h
Author: Kushal Poudel
Last Modified: April 26, 2026

Description: Header file for the StealableItem actor which represents collectible objects
in the game world such as keys, treasure, or objective items. This actor has a sphere
collision that detects when the player walks close enough to be in pickup range.

*/

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StealableItem.generated.h"

class USphereComponent;
class UStaticMeshComponent;

/* Creating a StealableItem Actor Class for collectible objects in the game
   The sphere collision tracks whether the player is close enough to grab
   When the player presses G and the grab animation notify fires the item is collected
   The GameMode is notified to update the collected count and the item is destroyed
*/

UCLASS()
class STEALANDESCAPE_API AStealableItem : public AActor
{
	GENERATED_BODY()

public:

	AStealableItem();

	//  this is Called by the player character when the grab animation notify fires
	void CollectItem();

protected:

	virtual void BeginPlay() override;

private:

	/*
	Sphere collision component that detects when the player is near to grab
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
		USphereComponent* CollisionSphere;

	/*
	Static mesh component to give the item a visual appearance in the game world
	This is changed in editor later
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* ItemMesh;

	/* This is Overlap event that fire when the player enters the sphere
	   Adds this item to the player's candidate list so player can garb it
	*/
	UFUNCTION()
		void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
			bool bFromSweep, const FHitResult& SweepResult);

	/* Overlap event that fires when the player leaves the sphere collision
	   Removes this item from the player's candidate list so it can no longer be grabbed
	*/
	UFUNCTION()
		void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};