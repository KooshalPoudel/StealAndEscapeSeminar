/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: AnimNotify_Footstep.h
Author: Alok Poudel
Contributors: Kushal Poudel
Last Modified: March 16, 2026
Description: This is a custom Animation Notify class that triggers footstep noise events
for the AI hearing detection system. When placed on the running animation timeline at the
point where the foot contacts the ground, it calls MakeFootstepNoise() on the player character
which reports a noise event that the guard AI can detect and investigate.
*/
#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_Footstep.generated.h"

UCLASS()
class STEALANDESCAPE_API UAnimNotify_Footstep : public UAnimNotify
{
	GENERATED_BODY()
public:
	/* Override of the Notify function from UAnimNotify
	   This is called by the engine when the animation reaches the notify point
	   It casts the owning actor to our player character and calls MakeFootstepNoise()
	*/
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};