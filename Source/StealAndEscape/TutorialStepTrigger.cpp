/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: TutorialStepTrigger.cpp
Author: Kushal Poudel and Alok Poudel
Last Modified: April 24, 2026

Description: Box trigger actor for a tutorial step. Fires overlap for
ReachArea and ReachExit step types. Automatically sanitizes any broken
actor scale at BeginPlay so existing placed triggers with corrupt scales
won't crash the physics system.
*/

#include "TutorialStepTrigger.h"
#include "Components/BoxComponent.h"
#include "StealAndEscapeCharacter.h"
#include "Kismet/GameplayStatics.h"

ATutorialStepTrigger::ATutorialStepTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBox;
	TriggerBox->SetBoxExtent(FVector(150.f, 150.f, 100.f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->SetGenerateOverlapEvents(true);
}

void ATutorialStepTrigger::BeginPlay()
{
	Super::BeginPlay();

	// Auto-repair any triggers that got accidentally scaled in the editor.
	// This prevents the "component set to zero" physics warnings and the
	// "Invalid DrawScale" map check errors. We force scale back to 1,1,1
	// at runtime so collisions work correctly even if the saved level has
	// corrupt scale values from earlier editing mistakes.
	FVector CurrentScale = GetActorScale3D();
	if (!CurrentScale.Equals(FVector(1.f, 1.f, 1.f), 0.001f))
	{
		SetActorScale3D(FVector(1.f, 1.f, 1.f));
	}

	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ATutorialStepTrigger::OnOverlapBegin);
	}
}

void ATutorialStepTrigger::SetTriggerActive(bool bActive)
{
	if (!TriggerBox) return;

	TriggerBox->SetCollisionEnabled(bActive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	TriggerBox->SetGenerateOverlapEvents(bActive);
}

void ATutorialStepTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	AStealAndEscapeCharacter* Player = Cast<AStealAndEscapeCharacter>(OtherActor);
	if (!Player) return;

	if (StepType != ETutorialStepType::ReachArea &&
		StepType != ETutorialStepType::ReachExit)
	{
		return;
	}

	ATutorialGameMode* GM = Cast<ATutorialGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!GM) return;

	if (!GM->IsCurrentStep(StepIndex)) return;

	GM->CompleteCurrentStep();
}