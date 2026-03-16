/*
Project Name: Steal and Escape
Description: A 3D top-down stealth / escape game developed in Unreal Engine 4.27
Course: CSCI 491 Seminar
File Name: AnimNotify_Footstep.cpp
Author: Alok Poudel
Contributors: Kushal Poudel
Last Modified: March 16, 2026
Description: Implementation of the custom Animation Notify for footstep noise events.
When the running animation reaches the point where the foot hits the ground, this notify
fires and reports a noise event through the player character's MakeFootstepNoise() function.
The guard AI perception system (UAISense_Hearing) picks up this noise and the guard
investigates the sound location.
*/
#include "AnimNotify_Footstep.h"
#include "StealAndEscapeCharacter.h"

/* Notify is called automatically by the engine when the animation playback reaches
   the notify marker placed on the animation timeline. MeshComp is the skeletal mesh
   that is playing the animation, and we use its owner to get the character reference.
   We cast to AStealAndEscapeCharacter to access MakeFootstepNoise() which reports
   the noise event to the AI hearing system.
*/
void UAnimNotify_Footstep::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	// Safety check to make sure MeshComp is valid before accessing the owner
	if (!MeshComp) return;

	// Getting the actor that owns this skeletal mesh component
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	// Casting to our player character class to call MakeFootstepNoise
	AStealAndEscapeCharacter* Player = Cast<AStealAndEscapeCharacter>(Owner);
	if (Player)
	{
		// This reports a noise event at the player location for the guard AI to detect
		Player->MakeFootstepNoise();
	}
}