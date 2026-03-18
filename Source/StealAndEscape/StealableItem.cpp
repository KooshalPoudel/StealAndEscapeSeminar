/*
Project Name: Steal and Escape
Description: A 3D top-down stealth / escape game developed in Unreal Engine 4.27
Course: CSCI 491 Seminar

File Name: StealableItem.cpp
Author: Kushal Poudel and Alok Poudel
Last Modified: March 18, 2026

Description: Implementation of the StealableItem actor which handles collectible objects.

Updated: Changed from auto-collect to manual grab system with candidate list.
Previously OnOverlapBegin would immediately collect the item and destroy it.
Now OnOverlapBegin adds this item to the player character's candidate list
and OnOverlapEnd removes it. The actual collection happens when the player
presses G and the grab animation notify calls CollectItem() through the
player character's CollectNearbyItem() which picks the closest candidate.

CollectItem() handles the same logic as before: notifying the GameMode to increment
the collected count and then destroying the actor. But now it only happens when the
player intentionally grabs the item during the animation.

The collision sphere uses OverlapAllDynamic profile so it generates overlap events
without physically blocking the player. Query only collision means the player
walks through the item and the overlap events track proximity.
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

	/* Creating the sphere collision component for detecting player proximity
	   InitSphereRadius sets the radius of the detection area around the item
	   200 units gives enough range for the player to be close but not too far

	   Collision is set up manually instead of using a profile name to make sure
	   it works correctly. QueryOnly means it does not physically block anything.
	   We set all channels to Overlap so it detects the player capsule entering.
	   GenerateOverlapEvents must be true on both this sphere and the player capsule
	   for OnComponentBeginOverlap and OnComponentEndOverlap to fire.
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

	/* DEBUG: Confirming that the StealableItem spawned and checking collision state */
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange,
			FString::Printf(TEXT("StealableItem spawned: %s"), *GetName()));
	}
	UE_LOG(LogTemp, Warning, TEXT("StealableItem BeginPlay: %s"), *GetName());

	// Checking if CollisionSphere is valid before binding
	if (CollisionSphere)
	{
		// Binding both overlap begin and end events
		// Begin adds this item to the player's candidate list
		// End removes this item from the player's candidate list
		CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AStealableItem::OnOverlapBegin);
		CollisionSphere->OnComponentEndOverlap.AddDynamic(this, &AStealableItem::OnOverlapEnd);

		/* DEBUG: Logging the collision state so we can verify it is set up correctly */
		UE_LOG(LogTemp, Warning, TEXT("StealableItem %s - Overlap events enabled: %s, Collision enabled: %d, Sphere radius: %.1f"),
			*GetName(),
			CollisionSphere->GetGenerateOverlapEvents() ? TEXT("YES") : TEXT("NO"),
			(int32)CollisionSphere->GetCollisionEnabled(),
			CollisionSphere->GetScaledSphereRadius());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("StealableItem %s - CollisionSphere is NULL!"), *GetName());
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red,
				FString::Printf(TEXT("ERROR: StealableItem %s has no CollisionSphere!"), *GetName()));
		}
	}
}

/* OnOverlapBegin - Player entered the sphere collision area
   Adds this item to the player's candidate list so it can be grabbed
   The player must still press G to actually collect the item
*/
void AStealableItem::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor) return;

	// Cast to player character to make sure only the player triggers this
	AStealAndEscapeCharacter* Player = Cast<AStealAndEscapeCharacter>(OtherActor);
	if (!Player) return;

	UE_LOG(LogTemp, Warning, TEXT("StealableItem %s - Player entered grab range"), *GetName());

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow,
			FString::Printf(TEXT("Near item: %s (Press G to grab)"), *GetName()));
	}

	// Add this item to the player's candidate list
	Player->AddNearbyItem(this);
}

/* OnOverlapEnd - Player left the sphere collision area
   Removes this item from the player's candidate list so it can no longer be grabbed
*/
void AStealableItem::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor) return;

	AStealAndEscapeCharacter* Player = Cast<AStealAndEscapeCharacter>(OtherActor);
	if (!Player) return;

	UE_LOG(LogTemp, Warning, TEXT("StealableItem %s - Player left grab range"), *GetName());

	// Remove this item from the player's candidate list
	Player->RemoveNearbyItem(this);
}

/* CollectItem - Called by the player character when the grab animation notify fires
   and this item was selected as the closest candidate
   This is the same collection logic as before but now it only triggers during the animation
   Notifies the GameMode to increment the collected item count then destroys the actor
*/
void AStealableItem::CollectItem()
{
	UE_LOG(LogTemp, Warning, TEXT("StealableItem %s - Collected by player grab!"), *GetName());

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green,
			FString::Printf(TEXT("Grabbed item: %s"), *GetName()));
	}

	// Getting the GameMode to notify it that an item was collected
	AStealAndEscapeGameMode* GM = Cast<AStealAndEscapeGameMode>(
		UGameplayStatics::GetGameMode(GetWorld()));
	if (GM)
	{
		GM->OnItemCollected();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("StealableItem - CollectItem FAILED: GameMode cast failed! Check World Settings GameMode Override"));
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
				TEXT("CollectItem FAILED: GameMode is wrong class! Check World Settings"));
		}
	}

	// Destroy the item so it disappears from the world
	Destroy();
}