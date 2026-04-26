/*
Project Name: Steal and Escape
Description: A 3D top-down stealth  escape game developed in Unreal Engine 
Course: CSCI 491 Seminar

File Name: ExitZone.cpp
Author: Kushal Poudel 
Last Modified: March 22, 2026

Description: This is a actor class which is created to for checking win and 
             loss condition. Firstly we created a box collison and placed near the door
			 then when the player char enters the box collison . this actor class checks
			 with the gamemode , and checks wheather all the required items are collected 
			 or not if is collected then itis a win condition. if not it logs a message
			 so that player knows they need to collect more items. 

			 The collison trigger box is setup with overlap events and only responds to
			 pawn . This is a query only collison which means it doesnot block the player 
			 from walking through this way the player can walk into the exit area and overlap
			 handles the logic. 
*/

#include "ExitZone.h"
#include "StealAndEscapeGameMode.h"
#include "StealAndEscapeCharacter.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

//This is Constructor for AEXitZone. 
AExitZone::AExitZone()
{
	//Disabling tick for this actor because it doesnot need to run logic every frame. 
	PrimaryActorTick.bCanEverTick = false;

	/*
	 Creating a box collsion component then passing that as defult subobject
	 which is a ureal function that creates a component and attached it to this actor
	 Trigger box is intenal component used by unreal to indentify this component
	*/
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));

	//Setting the box randomly, this is modified in unreal engine to match the door.
	TriggerBox->SetBoxExtent(FVector(100.f, 100.f, 100.f));  

	/*Generating built in unreal engine preset in which  player can walk through the 
	  boxand overlap event fires without blocking the player.
	*/
	TriggerBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	TriggerBox->SetGenerateOverlapEvents(true);
	RootComponent = TriggerBox;
}

// BeginPlay an unreal function is calledwhen the game starts or when the actor is spawned
void AExitZone::BeginPlay()
{
	Super::BeginPlay();

	/*
	 Binding the delegate to our custon overlap function 
	 Adddynamic is a UE macro that connects a multicast delegete to ufunction
	 using this binding whenever any actor enters the trigger box collsion the 
	 engine will automatically call our custom  function which is OnOverlapBegin 
    */
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AExitZone::OnOverlapBegin);
}

/* 
   This is our custom function that is called whever any actor enters the box
   Overlap comp is pointer to comp that was overlap i.e triggerbox
   otheractor is pointer to actor that entered collion box
   othercomp pointer to specific comp on otheractor which called overlap
   otherbodyindex index of the body othercomp
   bfromsweep is truue when ovelap was caused by sweepi.e moving collison test 
   Sweepresult contains hit result data like impact points 

*/
void AExitZone::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	// Checking to makesure theotheractor is not null, if so it crashes 
	if (!OtherActor) return;

	/* 
	   Casting otheractor to out player char , returns null pointer if player is not
	   correct type . Making sure that player char triggers the exit not the guard
	*/

	AStealAndEscapeCharacter* Player = Cast<AStealAndEscapeCharacter>(OtherActor);
	if (!Player) return;

	/*
	   Getting reference to current gamemode, getgamemode returns tha base AGameModeBase pointer
	   we then cast it to our custom funcion . Then calling win condition check. 
	*/
	AStealAndEscapeGameMode* GM = Cast<AStealAndEscapeGameMode>(
		UGameplayStatics::GetGameMode(GetWorld()));
	if (GM)
	{
		GM->OnPlayerReachedExit();
	}
}