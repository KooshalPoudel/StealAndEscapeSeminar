/*
Project Name: Steal and Escape
Description: A 3D top-down stealth / escape game developed in Unreal Engine 4.27
Course: CSCI 491 Seminar
Template Used: Unreal Engine 4 TopDown C++ Template (Epic Games)
Original Template Author: Epic Games

File Name: StealAndEscapeCharacter.cpp
Modified By: Kushal Poudel and Alok Poudel
Last Modified: March 18, 2026

Description: StealAndEscapeCharacter is the player character class based on Unreal Engine's 
             Top-Down C++ template. The template-generated components including camera setup, 
			 cursor decal logic, and click-to-move input are preserved as provided. Custom 
			 gameplay features added for this project include WASD keyboard movement, sprint 
			 functionality on the Shift key, adjustable WalkSpeed and RunSpeed variables, and 
			 a grab system triggered by the G key. The grab system works through a candidate list 
			 where any StealableItem that overlaps the player is added to a TArray of nearby items. 
			 When the player presses G the GrabPressed() function checks if the list is non-empty 
			 and plays a grab Animation Montage. At the hand-contact frame an Animation Notify calls 
			 CollectNearbyItem() which picks the closest item from the candidate list, awards the score, 
			 notifies the GameMode, and destroys the item. This approach correctly handles cases where 
			 multiple items are in overlap range at the same time.
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

/* Custom Method for Grab functionality using G key Now checks if there are any candidate items in range before playing
   the grab animation. If no items are nearby pressing G does nothing.
   The actual item collection happens later when the animation notify fires
   at the hand-contact frame through CollectNearbyItem()
*/
void AStealAndEscapeCharacter::GrabPressed()
{
	// Only allow grab if there is at least one nearby item to pick up
	if (!HasNearbyItems())
	{
		UE_LOG(LogTemp, Warning, TEXT("GrabPressed - No nearby items to grab"));
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
		UE_LOG(LogTemp, Warning, TEXT("GrabPressed - Playing grab animation (%d items in range)"),
			NearbyItems.Num());
		PlayAnimMontage(GrabMontage);
	}
}

/* CollectNearbyItem - Called by AnimNotify_GrabItem at the hand-contact frame
   This is the moment in the grab animation where the player's hand touches the item
   We find the closest valid candidate item from the NearbyItems list and collect it
   This handles the case where multiple items are in range by always picking the nearest one
   It also handles the case where the player walked away during the animation
*/
void AStealAndEscapeCharacter::CollectNearbyItem()
{
	// Find the closest item at the moment of collection
	// The player may have moved during the animation so we check distance NOW
	AStealableItem* ClosestItem = GetClosestNearbyItem();

	if (!ClosestItem)
	{
		UE_LOG(LogTemp, Warning, TEXT("CollectNearbyItem - No nearby items (walked away during animation)"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("CollectNearbyItem - Collecting closest item: %s"), *ClosestItem->GetName());

	// Remove from the candidate list BEFORE calling CollectItem because CollectItem
	// calls Destroy and we do not want a dangling pointer in the array
	NearbyItems.Remove(ClosestItem);

	// Tell the item to collect itself (notify GameMode and destroy)
	ClosestItem->CollectItem();
}

/* AddNearbyItem - Called by StealableItem OnOverlapBegin
   Adds the item to the candidate list when the player enters its sphere collision
   Uses AddUnique to prevent duplicate entries in case overlap fires twice
*/
void AStealAndEscapeCharacter::AddNearbyItem(AStealableItem* Item)
{
	if (!Item) return;

	NearbyItems.AddUnique(Item);

	UE_LOG(LogTemp, Warning, TEXT("AddNearbyItem - Added: %s (total candidates: %d)"),
		*Item->GetName(), NearbyItems.Num());
}

/* RemoveNearbyItem - Called by StealableItem OnOverlapEnd
   Removes the item from the candidate list when the player leaves its sphere collision
   Safe to call even if the item is not in the list
*/
void AStealAndEscapeCharacter::RemoveNearbyItem(AStealableItem* Item)
{
	if (!Item) return;

	NearbyItems.Remove(Item);

	UE_LOG(LogTemp, Warning, TEXT("RemoveNearbyItem - Removed: %s (total candidates: %d)"),
		*Item->GetName(), NearbyItems.Num());
}

/* HasNearbyItems - Returns true if the candidate list is not empty
   Used by GrabPressed to decide whether to play the grab animation
*/
bool AStealAndEscapeCharacter::HasNearbyItems() const
{
	return NearbyItems.Num() > 0;
}

/* GetClosestNearbyItem - Finds the closest item in the NearbyItems list
   Compares squared distances from the player's current location to each candidate
   Using DistSquared instead of Dist to avoid the square root calculation
   Returns nullptr if the list is empty
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
   This function reports a noise event at the player's current location using UAISense_Hearing
   The guard AI perception system picks up this noise and investigates the sound location
   This is triggered from the animation notify on the running animation when foot hits ground
   Loudness of 1.0 is the base volume and MaxRange of 0 means it uses the listener's HearingRange
*/
void AStealAndEscapeCharacter::MakeFootstepNoise()
{
	// Reporting a noise event at the player's location so the guard AI can detect it
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
