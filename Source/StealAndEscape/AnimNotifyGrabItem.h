/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: AnimNotifyGrabItem.h
Author: Kushal Poudel and Alok Poudel
Last Modified: March 18, 2026

Description: This is a custom Animation Notify class that triggers item collection
at the exact frame in the grab animation where the player's hand touches the item.
This works the same way as AnimNotify_Footstep but instead of reporting noise
it calls CollectNearbyItem() on the player character to pick up the item.

To use this notify:
1.) Open your GrabMontage in the Unreal Animation Editor
2.) Right click on the Notifies track at the frame where the hand makes contact
3.) Select Add Notify -> AnimNotifyGrabItem
4.) The item will be collected at that exact frame during gameplay
*/
#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotifyGrabItem.generated.h"

UCLASS()
class STEALANDESCAPE_API UAnimNotifyGrabItem : public UAnimNotify
{
	GENERATED_BODY()
public:
	/* Override of the Notify function from UAnimNotify
	   This is called by the engine when the animation reaches the notify point
	   It casts the owning actor to our player character and calls CollectNearbyItem()
	   which tells the nearest stealable item to collect itself
	*/
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};