/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar

File Name: StealableItem.h
Author: Kushal Poudel and Alok Poudel
Last Modified: March 18, 2026

Description: Header file for the StealableItem actor which represents collectible objects
in the game world such as keys, treasure, or objective items. This actor has a sphere
collision that detects when the player walks close enough to be in pickup range.

Updated: Changed from auto-collect on overlap to manual grab system.
Previously the item was collected automatically when the player entered the sphere.
Now the sphere collision only tracks whether the player is nearby. The player must
press G to play the grab animation and the item is collected when the animation
notify fires at the moment the hand touches the item. This makes the stealing
feel more intentional and satisfying.

The item adds itself to the player character's candidate list when the player enters
the sphere and removes itself when the player leaves. The player character picks
the closest candidate when the grab animation notify fires. This correctly handles
multiple items being in range at the same time.

Place this actor in the level editor wherever you want a collectible item to appear.
The CollisionRadius can be adjusted in the editor to control how close the player
needs to be to grab. A static mesh can be assigned to give the item a visual appearance.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StealableItem.generated.h"

// Forward declarations for component types to avoid IntelliSense parse order issues
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

	/* Called by the player character when the grab animation notify fires
	   and this item is the closest candidate. This notifies the GameMode
	   to increment the item count and destroys the item.
	   This is public so the player character can call it from CollectNearbyItem()
	*/
	void CollectItem();

protected:

	virtual void BeginPlay() override;

private:

	/* Sphere collision component that detects when the player is close enough to grab
	   Radius can be adjusted in the editor to control pickup range
	   The player must be inside this sphere AND press G to collect the item
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
		USphereComponent* CollisionSphere;

	/* Static mesh component to give the item a visual appearance in the game world
	   Assign a mesh in the editor or blueprint like a gold coin key or treasure model
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* ItemMesh;

	/* Overlap event that fires when the player enters the sphere collision
	   Adds this item to the player's candidate list so it can be grabbed
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