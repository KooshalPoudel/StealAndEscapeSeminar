/*
Project Name: Steal and Escape
Description: A 3D top-down stealth escape game developed in Unreal Engine 
Course: CSCI 491 Seminar
File Name: AnimNotifyGrabItem.cpp
Author: Kushal Poudel 
Last Modified: April 26, 2026

Description: This is a class where we implemented another animation notify for grabbing an item.
             We added animation notify in grab item montage in such a way that when the player 
			 characters hand reaches the lowest point  i.e where plaeyer is able to grab item
			 this notify is fired . Which then calls a function CollectNearbyItem() on the player char.
			 which somehow makes it look like item pickup feeels phyical instead instant. 

This follows the same pattern as AnimNotify_Footstep:
1.) Fristly getting the skeletal mesh component that is playing the animation
2.) Then getting the owning actor from that mesh component
3.) after that Casting that  to AStealAndEscapeCharacter
4.) Call the appropriate function (for this case is CollectNearbyItem )
*/
#include "AnimNotifyGrabItem.h"
#include "StealAndEscapeCharacter.h"
// Including Engine.h  mainly for GEngine->AddOnScreenDebugMessage for debugging 
#include "Engine/Engine.h"

/* The Notify function is automatically called by the engine when the animation
   playback reaches a notify marker placed on the grab montage timeline. MeshComp
   refers to the skeletal mesh that is playing the animation, and we can get the
   character refrence from its owner. We cast to AStealAndEscapeCharacter to access
   CollectNearbyItem(), which handles finding the closest item and telling it to
   collect itself, even if somtimes there is no item nearby or it might fail.
*/
void UAnimNotifyGrabItem::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	//  makeing  sure MeshComp is correct  before accessing the owner
	if (!MeshComp) return;

	// Getting the actor that owns this skeletal mesh component
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	// Casting to our player character class to call CollectNearbyItem
	AStealAndEscapeCharacter* Player = Cast<AStealAndEscapeCharacter>(Owner);
	if (Player)
	{
		// This finds the closest candidate item and tells it to collect itself
		Player->CollectNearbyItem();
	}
}