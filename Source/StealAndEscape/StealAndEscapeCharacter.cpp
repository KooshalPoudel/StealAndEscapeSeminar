/*
Project Name: Steal and Escape A 3D top-down stealth semi escape game developed in Unreal Engine 
Course: CSCI 491 Seminar
Template Used: Unreal Engine 4 TopDown C++ Template (Epic Games)
Original Template Author: Epic Games

File Name: StealAndEscapeCharacter.cpp
Modified By: Kushal Poudel and Alok Poudel
Last Modified: April 26, 2026

Description: StealAndEscapeCharacter is the player character class which is based on Unreal Engine's
			 Top-Down C++ template. The template generated components like camera setup, cursor decal
			 logic and click-to-move input are kept as it is from the template. Custom gameplay features
			 that we added for this project are WASD keyboard movement, sprint functionality on Shift key,
			 adjustable WalkSpeed and RunSpeed variables and also a grab system trigerred with G key.
			 The grab system works through a candidate list where any StealableItem that overlaps the
			 player is added to a TArray of nearby items. When the player presses G the GrabPressed()
			 funtion checks if the list is not empty and plays a grab Animation Montage. At the hand
			 contact frame an Animation Notify calls CollectNearbyItem() which picks the closest item
			 from the candidate list, awards the score, notifies the GameMode and destroys the item.
			 This way it correctly handles the case where multiple items are in overlap range at the
			 same time.
*/
#include "StealAndEscapeCharacter.h"
#include "StealableItem.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
// Including hearing sense header for reporting footstep noise events to the AI perception system
#include "Perception/AISense_Hearing.h"
// Including Engine.h for GEngine->AddOnScreenDebugMessage to show debug text during gameplay
#include "Engine/Engine.h"

AStealAndEscapeCharacter::AStealAndEscapeCharacter()
{

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Setting initial walkinh speed using walkspeed variable - Modified by Alok
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	// Create camera boom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false;

	//remove this cameraboon line if you do not like the camera angle
	// Shift camera target forward so player appears slightly lower/back on screen
	CameraBoom->TargetOffset = FVector(250.f, 0.f, 0.f);

	// Create camera
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false;

	// Create cursor decal
	CursorToWorld = CreateDefaultSubobject<UDecalComponent>("CursorToWorld");
	CursorToWorld->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UMaterial> DecalMaterialAsset(TEXT("Material'/Game/TopDownCPP/Blueprints/M_Cursor_Decal.M_Cursor_Decal'"));
	if (DecalMaterialAsset.Succeeded())
	{
		CursorToWorld->SetDecalMaterial(DecalMaterialAsset.Object);
	}

	CursorToWorld->DecalSize = FVector(16.0f, 32.0f, 32.0f);
	CursorToWorld->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f).Quaternion());

	PrimaryActorTick.bCanEverTick = true;
}

/*Overiding Unreal Engine Method to add custom input binding
  necessary for WASD Movement as well as spint and grab item feature
*/
void AStealAndEscapeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AStealAndEscapeCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AStealAndEscapeCharacter::MoveRight);

	// Shift for runing - Added by Alok
	PlayerInputComponent->BindAction("Run", IE_Pressed, this, &AStealAndEscapeCharacter::StartRun);
	PlayerInputComponent->BindAction("Run", IE_Released, this, &AStealAndEscapeCharacter::StopRun);

	//G for grab item- Added by  Kushal 
	PlayerInputComponent->BindAction("Grab", IE_Pressed, this, &AStealAndEscapeCharacter::GrabPressed);
}
/* Custom Method for forward movement with W key as well as Backward movement with S key
   Movement direction is being calculated  in relation with conteollers yaw rotation
   Character moves depending upon the direcction where the camrea is faced-- Alok
*/
void AStealAndEscapeCharacter::MoveForward(float Value)
{
	if (Controller && Value != 0.0f)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

// Custom Method for Right movement with D key as well as Left movement with A key-- Alok
void AStealAndEscapeCharacter::MoveRight(float Value)
{
	if (Controller && Value != 0.0f)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

/* Custom Methods for Sprint functionality using Shift key
   When Shift key is pressed character speed is increased to  matchRunSpeed
   and when released character speed is returned back to WalkSpeed -- Alok
*/
void AStealAndEscapeCharacter::StartRun()
{
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
}

void AStealAndEscapeCharacter::StopRun()
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

/* Custom Method for Grab functionality using G key. This now checks if there are any
   candidate items in range before playing the grab animation. If no item is nearby
   then pressing G does nothing.
   The actual item collection happens later when the animation notify fires at the
   hand contact frame through CollectNearbyItem() function -- kushal
*/
void AStealAndEscapeCharacter::GrabPressed()
{
	// Only allow garb if there is at least one nearby item to pick up
	if (!HasNearbyItems())
	{
		return;
	}

	if (!GrabMontage)
	{
		return;
	}

	if (!GetMesh())
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		return;
	}

	//not to restart if already playing
	if (!AnimInstance->Montage_IsPlaying(GrabMontage))
	{
		PlayAnimMontage(GrabMontage);
	}
}

/* CollectNearbyItem this is Called by AnimNotify_GrabItem at the hand contact frame
   This is the moment in the grab animation where the player's hand touches the item.
   Here we find the closest valid candidate item from the NearbyItems list and collect it.
   This handles the case where multiple items are in range by always picking the nearest one
   it also handles the case where the player walked away during the animation -- kushal
*/
void AStealAndEscapeCharacter::CollectNearbyItem()
{
	/* Find the closest item at the moment of collection
	   The player may have moved during the animation so we check the distance NOW
	*/
	AStealableItem* ClosestItem = GetClosestNearbyItem();

	if (!ClosestItem)
	{
		return;
	}

	/* Removing it form the candidate list BEFORE calling CollectItem because CollectItem
	   calls Destroy and we donot want a dangling pointer in the array
	*/
	NearbyItems.Remove(ClosestItem);

	// Telling the item to collect itself which then notifies GameMode and destroys itself
	ClosestItem->CollectItem();
}

/* AddNearbyItem this is Called by StealableItem OnOverlapBegin
   Adds the item to the candidate list when the player enters its sphere collison
   Uses AddUnique to prevent dupicate entries in case overlap fires twice -- kushal
*/
void AStealAndEscapeCharacter::AddNearbyItem(AStealableItem* Item)
{
	if (!Item) return;

	NearbyItems.AddUnique(Item);
}

/* RemoveNearbyItem this is Called by StealableItem OnOverlapEnd
   Removes the item from the candidate list when the player leaves its sphere collison
   Safe to call even if the item is not in the list -- kushal
*/
void AStealAndEscapeCharacter::RemoveNearbyItem(AStealableItem* Item)
{
	if (!Item) return;

	NearbyItems.Remove(Item);
}

/* HasNearbyItems  Returns true if the candidate list is not empty
   This is used by GrabPressed to decide wheather to play the grab animation or not
*/
bool AStealAndEscapeCharacter::HasNearbyItems() const
{
	return NearbyItems.Num() > 0;
}

/* GetClosestNearbyItem  Finds the closest item in the NearbyItems list
   It compares squared distances from the players current location to each candidate
   We are using DistSquared instead of Dist to avoid the square root calculation
   Returns nullptr if the list is empty -- kushal
*/
AStealableItem* AStealAndEscapeCharacter::GetClosestNearbyItem() const
{
	AStealableItem* ClosestItem = nullptr;
	float ClosestDistSq = TNumericLimits<float>::Max();

	const FVector PlayerLocation = GetActorLocation();

	for (AStealableItem* Item : NearbyItems)
	{
		// Safety check in case an item was destroyed but not removed from the list
		if (!Item || !IsValid(Item))
		{
			continue;
		}

		float DistSq = FVector::DistSquared(PlayerLocation, Item->GetActorLocation());
		if (DistSq < ClosestDistSq)
		{
			ClosestDistSq = DistSq;
			ClosestItem = Item;
		}
	}

	return ClosestItem;
}

/* Custom Method for generating footstep noise for the AI hearing detection system
   This function reports a noise event at the players current location using UAISense_Hearing
   The guard AI perception system  picks up this noise and investigate the sound location.
   This is trigerred from the animation notify on the running animation when foot hits ground.
   Loudness of 1.0 is the base volume and MaxRange of 0 means it uses the listeners HearingRange
   -- kushal
*/
void AStealAndEscapeCharacter::MakeFootstepNoise()
{
	// Reporting a noise event at the players location so guard AI can detect it
	MakeNoise(1.0f, this, GetActorLocation(), 0.0f, FName("Footstep"));
}

//Unreal Engine TopDown C++ template method
void AStealAndEscapeCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (CursorToWorld != nullptr)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			FHitResult TraceHitResult;
			PC->GetHitResultUnderCursor(ECC_Visibility, true, TraceHitResult);

			FVector CursorFV = TraceHitResult.ImpactNormal;
			FRotator CursorR = CursorFV.Rotation();

			CursorToWorld->SetWorldLocation(TraceHitResult.Location);
			CursorToWorld->SetWorldRotation(CursorR);
		}
	}
}