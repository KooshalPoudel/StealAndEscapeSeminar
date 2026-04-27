/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: ExitZone.h
Author: Kushal Poudel 
Last Modified: April 26, 2026
Description: Header file for the ExitZone actor which is used as the win condition trigger.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExitZone.generated.h"

class UBoxComponent;

/* Creating an ExitZone Actor Class which is placed at the location of exit door
   When the player character overlap the box collision it triggers the win-condition
   through the GameMode if all required item is  collected
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

	/* Box collision component which  detect when the player enters the exit box
	   Size can be adjusted in the level editor to match the door or exit location
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Exit", meta = (AllowPrivateAccess = "true"))
		UBoxComponent* TriggerBox;

	/* Overlap event function that fires when any actor enters the box collision
	   We cast OtherActor to the player character to make sure only the player triggers the exit
	   Then we call function form  gamemode to check items and trigger win
	*/
	UFUNCTION()
		void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
			bool bFromSweep, const FHitResult& SweepResult);
};