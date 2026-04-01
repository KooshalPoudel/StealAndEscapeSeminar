/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: AnimNotify_Footstep.h
Author: Alok Poudel, Kushal Poudel
Last Modified: March 22, 2026
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
	/* Override the existing Notify function from UAnimNotify
	   This function run automatically when the animation reaches the notify point.- kushal
	*/
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};