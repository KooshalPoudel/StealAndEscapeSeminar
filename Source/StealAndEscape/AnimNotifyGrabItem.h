/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: AnimNotifyGrabItem.h
Author: Kushal Poudel 
Last Modified: April 26, 2026

Description:  This is a header class Custom Animation Notify which triggers
              item collection with the help of grab animation montage.
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
	/* Override of the existing Notify function from UAnimNotify
	   Unreal calls this  function at the exact frame where we added notify in the 
	   grab montage
	*/
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};