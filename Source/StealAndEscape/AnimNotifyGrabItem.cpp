/*
Project Name: Steal and Escape
Description: A 3D top-down stealth / escape game developed in Unreal Engine 4.27
Course: CSCI 491 Seminar
File Name: AnimNotifyGrabItem.cpp
Author: Kushal Poudel and Alok Poudel
Last Modified: March 18, 2026

Description: Implementation of the custom Animation Notify for item grabbing.
When the grab animation reaches the frame where the player's hand touches the item
this notify fires and calls CollectNearbyItem() on the player character. This makes
the item pickup feel physical and timed to the animation instead of instant.

This follows the same pattern as AnimNotify_Footstep:
1.) Get the skeletal mesh component that is playing the animation
2.) Get the owning actor from that mesh component
3.) Cast to AStealAndEscapeCharacter
4.) Call the appropriate function (CollectNearbyItem instead of MakeFootstepNoise)
*/
#include "AnimNotifyGrabItem.h"
#include "StealAndEscapeCharacter.h"
// Including Engine.h for GEngine->AddOnScreenDebugMessage for debug feedback
#include "Engine/Engine.h"

/* Notify is called automatically by the engine when the animation playback reaches
   the notify marker placed on the grab montage timeline. MeshComp is the skeletal mesh
   that is playing the animation and we use its owner to get the character reference.
   We cast to AStealAndEscapeCharacter to access CollectNearbyItem() which handles
   finding the closest candidate item and telling it to collect itself.
*/
void UAnimNotifyGrabItem::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	// Safety check to make sure MeshComp is valid before accessing the owner
	if (!MeshComp) return;

	// Getting the actor that owns this skeletal mesh component
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	// Casting to our player character class to call CollectNearbyItem
	AStealAndEscapeCharacter* Player = Cast<AStealAndEscapeCharacter>(Owner);
	if (Player)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimNotifyGrabItem - Notify fired! Calling CollectNearbyItem"));
		// This finds the closest candidate item and tells it to collect itself
		Player->CollectNearbyItem();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimNotifyGrabItem - Cast to player failed"));
	}
}