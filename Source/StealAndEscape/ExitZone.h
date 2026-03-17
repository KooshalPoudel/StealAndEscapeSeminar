/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar

File Name: ExitZone.h
Author: Kushal Poudel and Alok Poudel
Last Modified: March 17, 2026

Description: Header file for the ExitZone actor which is used as the win condition trigger.
This actor has a box collision that detects when the player walks into the exit area.
When the player overlaps the box it checks with the GameMode if all items are collected.
If all items are collected the game ends with a win. If not it logs a message telling
the player to collect all items first.

Place this actor in the level editor at the door or exit location.
The box size can be adjusted in the editor using the BoxExtent property.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExitZone.generated.h"

// Forward declaration for BoxComponent to avoid IntelliSense parse order issues
class UBoxComponent;

/* Creating an ExitZone Actor Class which is placed at the exit door location
   When the player character overlaps the box collision it triggers the win condition
   through the GameMode if all required items have been collected
*/

UCLASS()
class STEALANDESCAPE_API AExitZone : public AActor
{
	GENERATED_BODY()

public:

	AExitZone();

protected:

	virtual void BeginPlay() override;

private:

	/* Box collision component that detects when the player enters the exit area
	   Size can be adjusted in the level editor to match the door or exit location
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Exit", meta = (AllowPrivateAccess = "true"))
		UBoxComponent* TriggerBox;

	/* Overlap event function that fires when any actor enters the box collision
	   We cast OtherActor to the player character to make sure only the player triggers the exit
	   Then we call GameMode->OnPlayerReachedExit() to check items and trigger win
	*/
	UFUNCTION()
		void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
			bool bFromSweep, const FHitResult& SweepResult);
};