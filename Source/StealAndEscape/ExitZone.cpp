/*
Project Name: Steal and Escape
Description: A 3D top-down stealth / escape game developed in Unreal Engine 4.27
Course: CSCI 491 Seminar

File Name: ExitZone.cpp
Author: Kushal Poudel and Alok Poudel
Last Modified: March 17, 2026

Description: Implementation of the ExitZone actor which handles the win condition.
When the player character enters the box collision this actor checks with the GameMode
whether all required items have been collected. If yes it triggers the win condition.
If no it logs a message so the player knows they need to collect more items.

The TriggerBox is set up with overlap events and only responds to pawns.
Query only collision means it does not physically block the player from walking through.
This way the player can walk into the exit area and the overlap event handles the logic.
*/

#include "ExitZone.h"
#include "StealAndEscapeGameMode.h"
#include "StealAndEscapeCharacter.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
// Including Engine/World.h so Visual Studio IntelliSense can resolve GetWorld()
#include "Engine/World.h"

AExitZone::AExitZone()
{
	PrimaryActorTick.bCanEverTick = false;

	/* Creating the box collision component for detecting player overlap
	   BoxExtent sets the half-size of the box so 100 units means 200 units total per side
	   SetCollisionProfileName to OverlapAllDynamic so it generates overlap events
	   SetGenerateOverlapEvents must be true for OnComponentBeginOverlap to fire
	*/
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetBoxExtent(FVector(100.f, 100.f, 100.f));
	TriggerBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	TriggerBox->SetGenerateOverlapEvents(true);
	RootComponent = TriggerBox;
}

// BeginPlay is called when the game starts or when the actor is spawned
void AExitZone::BeginPlay()
{
	Super::BeginPlay();

	// Binding the overlap event to our custom function
	// When any actor enters the box collision OnOverlapBegin will be called
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AExitZone::OnOverlapBegin);
}

/* OnOverlapBegin is called when an actor enters the box collision area
   We first cast OtherActor to our player character class to make sure
   only the player triggers the exit and not other actors like guards
   Then we get the GameMode and call OnPlayerReachedExit which handles
   checking if all items are collected and triggering the win condition
*/
void AExitZone::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Player reached exit")); //Debug Code
	// Safety check to make sure OtherActor is valid
	if (!OtherActor) return;

	// Casting to our player character class to make sure only the player triggers the exit
	AStealAndEscapeCharacter* Player = Cast<AStealAndEscapeCharacter>(OtherActor);
	if (!Player) return;

	// Getting the GameMode to call the win condition check
	AStealAndEscapeGameMode* GM = Cast<AStealAndEscapeGameMode>(
		UGameplayStatics::GetGameMode(GetWorld()));
	if (GM)
	{
		GM->OnPlayerReachedExit();
	}
}