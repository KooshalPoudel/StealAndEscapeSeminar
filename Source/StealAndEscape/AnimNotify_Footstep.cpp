/*
Project Name: Steal and Escape
Description: A 3D top-down stealth escape game developed in Unreal Engine 
File Name: AnimNotify_Footstep.cpp
Author: Alok Poudel, Kushal Poudel 
Last Modified: April 26, 2026
Description: In this class we are implementing  a custom animation notifiy which triggers
             Footstep noise event. In the running animation when the foot reasches ground. 
			 The footstep sound plays and after that this event is triggered which reports a
			 noise event with the help of MakeFootstepNoise() function . After this notifes
			 is fired the guard ai has a preception system 'UAISense_Hearing' which picks up 
			 this noise then guard chases the player based on that.
*/

#include "AnimNotify_Footstep.h"
#include "StealAndEscapeCharacter.h"

/*
* The Notify function is automatically called by the engine when the animation
  playback reaches a notify marker on the animation timeline. MeshComp refers to
  a skeletal mesh that is playing an animation. We can obtain a character reference
  from its owner. We cast to AStealAndEscapeCharacter to access MakeFootstepNoise(),
  which reports noise to the AI hearing system.--Kushal Poudel
*/
void UAnimNotify_Footstep::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!MeshComp) return;

	// Getting the actor which owns this skeletal mesh component - kushal
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	// Casting to our player character class to call MakeFootstepNoise - kushal 
	AStealAndEscapeCharacter* Player = Cast<AStealAndEscapeCharacter>(Owner);
	if (Player)
	{
		// This makes a noise event at the player location when the foot reaches ground.-kushal
		Player->MakeFootstepNoise();
	}
}