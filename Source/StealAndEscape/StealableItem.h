/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar

File Name: StealableItem.h
Author: Kushal Poudel and Alok Poudel
Last Modified: March 17, 2026

Description: Header file for the StealableItem actor which represents collectible objects
in the game world such as keys, treasure, or objective items. This actor has a sphere
collision that detects when the player walks close enough to pick it up.
When the player overlaps the sphere collision the item is collected and destroyed.
The GameMode is notified so it can update the collected item count.

Place this actor in the level editor wherever you want a collectible item to appear.
The CollisionRadius can be adjusted in the editor to control how close the player
needs to be to pick up the item. A static mesh can be assigned to give the item
a visual appearance like a gold coin or key.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StealableItem.generated.h"

// Forward declarations for component types to avoid IntelliSense parse order issues
class USphereComponent;
class UStaticMeshComponent;

/* Creating a StealableItem Actor Class for collectible objects in the game
   When the player overlaps the sphere collision the item is picked up
   and the GameMode is notified to update the collected count
*/

UCLASS()
class STEALANDESCAPE_API AStealableItem : public AActor
{
	GENERATED_BODY()

public:

	AStealableItem();

protected:

	virtual void BeginPlay() override;

private:

	/* Sphere collision component that detects when the player gets close enough to pick up
	   Radius can be adjusted in the editor to control pickup range
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
		USphereComponent* CollisionSphere;

	/* Static mesh component to give the item a visual appearance in the game world
	   Assign a mesh in the editor or blueprint like a gold coin key or treasure model
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* ItemMesh;

	/* Overlap event function that fires when the player enters the sphere collision
	   We cast to the player character to make sure only the player can pick up items
	   Then we notify the GameMode and destroy this actor
	*/
	UFUNCTION()
		void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
			bool bFromSweep, const FHitResult& SweepResult);
};