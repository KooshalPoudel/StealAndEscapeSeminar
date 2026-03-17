/*
Project Name: Steal and Escape
Description: A 3D top-down stealth / escape game developed in Unreal Engine 4.27
Course: CSCI 491 Seminar

File Name: StealableItem.cpp
Author: Kushal Poudel and Alok Poudel
Last Modified: March 17, 2026

Description: Implementation of the StealableItem actor which handles collectible objects.
When the player character enters the sphere collision the item notifies the GameMode
to increment the collected item count and then destroys itself from the world.
The static mesh component provides a visual representation so the player can see
what they are picking up. The mesh can be assigned in the editor or blueprint.

The collision sphere uses OverlapAllDynamic profile so it generates overlap events
without physically blocking the player. Query only collision means the player
walks through the item and the overlap event handles the pickup logic.
*/

#include "StealableItem.h"
#include "StealAndEscapeGameMode.h"
#include "StealAndEscapeCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
// Including Engine/World.h so Visual Studio IntelliSense can resolve GetWorld()
#include "Engine/World.h"
// Including Engine.h for GEngine->AddOnScreenDebugMessage to show debug text during gameplay
#include "Engine/Engine.h"

AStealableItem::AStealableItem()
{
	PrimaryActorTick.bCanEverTick = false;

	/* Creating the sphere collision component for detecting player overlap
	   InitSphereRadius sets the radius of the detection area around the item
	   200 units is a large pickup range for testing so we can confirm overlap works
	   Once confirmed working this can be reduced to 80 or whatever feels right

	   Collision is set up manually instead of using a profile name to make sure
	   it works correctly. QueryOnly means it does not physically block anything.
	   We set all channels to Overlap so it detects the player capsule entering.
	   GenerateOverlapEvents must be true on both this sphere and the player capsule
	   for OnComponentBeginOverlap to fire.
	*/
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->InitSphereRadius(200.f);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	CollisionSphere->SetGenerateOverlapEvents(true);
	RootComponent = CollisionSphere;

	/* Creating the static mesh component for the visual appearance of the item
	   This is attached to the collision sphere so it moves with it
	   The actual mesh like a gold coin or key is assigned in the editor or blueprint
	*/
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	ItemMesh->SetupAttachment(RootComponent);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// BeginPlay is called when the game starts or when the actor is spawned
void AStealableItem::BeginPlay()
{
	Super::BeginPlay();

	/* DEBUG: Confirming that the StealableItem spawned and checking collision state
	   This message appears on screen when the game starts so we know the item exists
	   If you do NOT see this message then the item was not placed in the level
	*/
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange,
			FString::Printf(TEXT("StealableItem spawned: %s"), *GetName()));
	}
	UE_LOG(LogTemp, Warning, TEXT("StealableItem BeginPlay: %s"), *GetName());

	// Checking if CollisionSphere is valid before binding
	if (CollisionSphere)
	{
		// Binding the overlap event to our custom function
		// When the player enters the sphere collision OnOverlapBegin will be called
		CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AStealableItem::OnOverlapBegin);

		/* DEBUG: Logging the collision state so we can verify it is set up correctly
		   GenerateOverlapEvents must be true for overlaps to fire
		   CollisionEnabled must be QueryOnly or QueryAndPhysics for overlaps to work
		*/
		UE_LOG(LogTemp, Warning, TEXT("StealableItem %s - Overlap events enabled: %s, Collision enabled: %d, Sphere radius: %.1f"),
			*GetName(),
			CollisionSphere->GetGenerateOverlapEvents() ? TEXT("YES") : TEXT("NO"),
			(int32)CollisionSphere->GetCollisionEnabled(),
			CollisionSphere->GetScaledSphereRadius());
	}
	else
	{
		// If CollisionSphere is null something went wrong in the constructor
		UE_LOG(LogTemp, Error, TEXT("StealableItem %s - CollisionSphere is NULL!"), *GetName());
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red,
				FString::Printf(TEXT("ERROR: StealableItem %s has no CollisionSphere!"), *GetName()));
		}
	}
}

/* OnOverlapBegin is called when an actor enters the sphere collision area
   DEBUG VERSION: Every step has a debug message so we can see exactly what happens
   and where the function stops if the item is not being collected
*/
void AStealableItem::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	/* DEBUG STEP 1: Something entered the sphere collision
	   If you see this message then the overlap event IS firing
	   If you never see this message then the overlap binding or collision setup is the problem
	*/
	UE_LOG(LogTemp, Warning, TEXT("StealableItem OnOverlapBegin - STEP 1: Something overlapped! Actor: %s, Class: %s"),
		OtherActor ? *OtherActor->GetName() : TEXT("NULL"),
		OtherActor ? *OtherActor->GetClass()->GetName() : TEXT("NULL"));

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Magenta,
			FString::Printf(TEXT("STEP 1: Overlap detected by %s - Actor: %s"),
				*GetName(),
				OtherActor ? *OtherActor->GetName() : TEXT("NULL")));
	}

	// Safety check to make sure OtherActor is valid
	if (!OtherActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("StealableItem - FAILED: OtherActor is NULL"));
		return;
	}

	/* DEBUG STEP 2: Trying to cast to player character
	   If you see Step 1 but not Step 2 then the overlapping actor is not the player
	   This means something else like a guard or a wall is triggering the overlap
	   Check what actor name was printed in Step 1
	*/
	AStealAndEscapeCharacter* Player = Cast<AStealAndEscapeCharacter>(OtherActor);
	if (!Player)
	{
		UE_LOG(LogTemp, Warning, TEXT("StealableItem - STEP 2 FAILED: Cast to AStealAndEscapeCharacter failed! Actor class: %s"),
			*OtherActor->GetClass()->GetName());

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red,
				FString::Printf(TEXT("STEP 2 FAILED: %s is not the player (class: %s)"),
					*OtherActor->GetName(),
					*OtherActor->GetClass()->GetName()));
		}
		return;
	}

	/* DEBUG STEP 3: Cast succeeded - this IS the player
	   If you see this then the player walked into the sphere and was recognized
	*/
	UE_LOG(LogTemp, Warning, TEXT("StealableItem - STEP 3: Player confirmed! Getting GameMode..."));
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green,
			TEXT("STEP 3: Player confirmed! Collecting item..."));
	}

	// Getting the GameMode to notify it that an item was collected
	AStealAndEscapeGameMode* GM = Cast<AStealAndEscapeGameMode>(
		UGameplayStatics::GetGameMode(GetWorld()));
	if (GM)
	{
		/* DEBUG STEP 4: GameMode found and calling OnItemCollected
		   If you see this then everything worked and the item count should go up
		*/
		UE_LOG(LogTemp, Warning, TEXT("StealableItem - STEP 4: GameMode found! Calling OnItemCollected..."));
		GM->OnItemCollected();
	}
	else
	{
		// GameMode cast failed which means the wrong GameMode class is being used
		UE_LOG(LogTemp, Error, TEXT("StealableItem - STEP 4 FAILED: GameMode cast failed! Check World Settings GameMode Override"));
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
				TEXT("STEP 4 FAILED: GameMode is wrong class! Check World Settings"));
		}
	}

	/* DEBUG STEP 5: Destroying the item
	   If you see Step 4 but the item does not disappear then Destroy failed
	*/
	UE_LOG(LogTemp, Warning, TEXT("StealableItem - STEP 5: Destroying item %s"), *GetName());
	Destroy();
}