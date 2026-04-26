/*
Project Name: Steal and Escape A 3D top-down stealth escape game developed in Unreal Engine
Course: CSCI 491 Seminar

File Name: StealableItem.cpp
Author: Kushal Poudel 
Last Modified: March 22, 2026

Description: 
Implementing stealableitem actor which is handling collectible objects.
whenever the player char enters the sphere collison the item notifies gamemode
to add the collected item count and the destroying itself form the world 

Static mesh component  here is providing visual representation of item.
We will add this in unreal editor adjusting the size as well as adding VFX
so that player know which item is intractable. 

Collsion sphere uses OverLabAllDynamic profile so it doesnot block the player
but still generates overlap events . Also the collision is query only collison
which implies player can walk thorough it and the overlap event  handles pickup logic.
*/

#include "StealableItem.h"
#include "StealAndEscapeGameMode.h"
#include "StealAndEscapeCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

/*
This is constructor for AStelableItem which is called in the runtime 
as well as when the actor is first created
*/

AStealableItem::AStealableItem()
{
	PrimaryActorTick.bCanEverTick = false;

	/*  
	Setting up the first component which is collison sphere 
	This is a invisible sphere that detectes when the player is colse enough 
	we are reating the sphere collision component and registering it as a default subobject

	Also some baisc collision settings are set, which we will change later from bueprint
	according to our need.
	*/
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->InitSphereRadius(200.f);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	CollisionSphere->SetGenerateOverlapEvents(true);
	RootComponent = CollisionSphere;

	/* 
	Setting up second component which is a visible 3D item mesh that player can see
	This is attached to the collision sphere so it moves with it
	 The actual mesh will be assigned in the unreal engine
	*/
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	ItemMesh->SetupAttachment(RootComponent);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// BeginPlay is called when the game starts or when the actor is spawned
void AStealableItem::BeginPlay()
{
	Super::BeginPlay();

	// Here we are Checking if CollisionSphere is valid before binding
	if (CollisionSphere)
	{
		/*Binding both overlap begin and end events
		 Firstly Begin adds this item to the player's candidate list
		 After that End removes this item from the player's candidate list*/
		CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AStealableItem::OnOverlapBegin);
		CollisionSphere->OnComponentEndOverlap.AddDynamic(this, &AStealableItem::OnOverlapEnd);
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red,
				FString::Printf(TEXT("ERROR: StealableItem %s has no CollisionSphere!"), *GetName()));
		}
	}
}

/* OnOverlapBegin  here mean Player entered the sphere collision area
   if so that Add this item to the player's candidate list so it can be grabbed
   The player has to press G to actually grab the item
*/
void AStealableItem::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor) return;

	// Casting  it to player char to make sure only the player can trigger
	AStealAndEscapeCharacter* Player = Cast<AStealAndEscapeCharacter>(OtherActor);
	if (!Player) return;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow,
			TEXT("Near a collectible item (Press G to grab)"));
	}

	// Adding this item to the player's candidate list
	Player->AddNearbyItem(this);
}

/* OnOverlapEnd mean that  Player left the collison area 
   if so Remove this from the player's candidate list so that player cannot garb it. 
*/
void AStealableItem::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor) return;

	AStealAndEscapeCharacter* Player = Cast<AStealAndEscapeCharacter>(OtherActor);
	if (!Player) return;

	// Remove this item from the player's candidate list
	Player->RemoveNearbyItem(this);
}

/* CollectItem this is  Called by the player character when the grab animation notify fires
   and this item was selected as the closest candidate
   this is same logic like last one but now it only triggers during the animation
   Notifies the GameMode to increment the collected item count after it then destroys the actor
*/
void AStealableItem::CollectItem()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green,
			TEXT("Item collected"));
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
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
				TEXT("CollectItem FAILED: GameMode is wrong class! Check World Settings"));
		}
	}

	// Destroying the item so it disappears from the world
	Destroy();
}