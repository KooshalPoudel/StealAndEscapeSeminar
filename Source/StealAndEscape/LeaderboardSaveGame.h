/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar

File Name: LeaderboardSaveGame.h
Author: Kushal Poudel and Alok Poudel
Last Modified: April 26, 2026

Description: This is a save game class which stores the per-level leaderboard for our game.
			 Each level name is maped to an array of score entrys sorted high to low and
			 caped at 10 entrys . The entry struct contains player name , score , time taken ,
			 items collected and the date the score was acheived. The save is written to a
			 fixed slot name called "Leaderboard_Slot" so there is only ever one leaderboard
			 file per install of the game and we dont have to worry about diffrent slot
			 names for diffrent levels.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "LeaderboardSaveGame.generated.h"

/* This is a single leaderboard row , holds all the info for one player score entry */
USTRUCT(BlueprintType)
struct FLeaderboardEntry
{
	GENERATED_BODY()
		// This is the name typed by the player on the name entry popup 
		UPROPERTY(BlueprintReadOnly, Category = "Leaderboard")
		FString PlayerName;
	// This is final score calulated by the gamemode at end of the level 
	UPROPERTY(BlueprintReadOnly, Category = "Leaderboard")
		int32 Score = 0;
	// This is for time taken to finish the level in seconds , used for tie-breaker maybe later
	UPROPERTY(BlueprintReadOnly, Category = "Leaderboard")
		float TimeTaken = 0.f;
	// how many items player collected , just shown for info on the leaderboard 
	UPROPERTY(BlueprintReadOnly, Category = "Leaderboard")
		int32 ItemsCollected = 0;
	// the date when score was acheived , stored as string for easy display 
	UPROPERTY(BlueprintReadOnly, Category = "Leaderboard")
		FString DateString;
};

/* This is a wraper around an array so TMap can hold it as a value . Unreal doesnot
   allow nesting containers like TArray inside TMap directly , so we have to wrap the
   array in a USTRUCT to make it work. Kinda annoying but its the only way.
*/
USTRUCT()
struct FLeaderboardEntries
{
	GENERATED_BODY()
		UPROPERTY()
		TArray<FLeaderboardEntry> Entries;
};

UCLASS()
class STEALANDESCAPE_API ULeaderboardSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	/* Map from level name to its top 10 scores sorted high to low . Each level has
	   its own seperate leaderboard so the score from level 1 doesnot mix with level 2
	*/
	UPROPERTY(VisibleAnywhere, Category = "Leaderboard")
		TMap<FName, FLeaderboardEntries> ScoresPerLevel;
	// Fixed slot name used for saveing and loading the leaderboard file 
	static const FString SlotName;
	// Maximum entrys kept per level , anything below rank 10 is just thrown away
	static constexpr int32 MaxEntriesPerLevel = 10;
	/* Insert a new score into the leaderboard for the given level . This keeps the
	   array sorted and trimed to top 10 . Returns the rank (1 indexed) so the UI
	   can show "you came 3rd" or similiar , or returns -1 if the score didnot make
	   the top 10.
	*/
	int32 AddEntry(FName LevelName, const FLeaderboardEntry& NewEntry);
	TArray<FLeaderboardEntry> GetEntries(FName LevelName) const;
};