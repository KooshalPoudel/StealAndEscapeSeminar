/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: LeaderboardSaveGame.h
Author: Kushal Poudel and Alok Poudel
Last Modified: April 18, 2026

Description: Save game class that stores the per-level leaderboard.
             Each level name maps to an array of score entries sorted high-to-low
             and capped at 10 entries. Entries contain player name, score, time
             taken, items collected, and the date the score was achieved.

             The save is written to a fixed slot name "Leaderboard_Slot" so
             there is only ever one leaderboard file per install.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "LeaderboardSaveGame.generated.h"

/* Single leaderboard row */
USTRUCT(BlueprintType)
struct FLeaderboardEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Leaderboard")
		FString PlayerName;

	UPROPERTY(BlueprintReadOnly, Category = "Leaderboard")
		int32 Score = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Leaderboard")
		float TimeTaken = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Leaderboard")
		int32 ItemsCollected = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Leaderboard")
		FString DateString;
};

/* Wrapper around an array so TMap can hold it as a value.
   Unreal requires a USTRUCT wrapper when nesting containers in TMap. */
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
	/* Map from level name to its top 10 scores sorted high-to-low */
	UPROPERTY(VisibleAnywhere, Category = "Leaderboard")
		TMap<FName, FLeaderboardEntries> ScoresPerLevel;

	/* Fixed slot name used for saving and loading */
	static const FString SlotName;

	/* Maximum entries kept per level */
	static constexpr int32 MaxEntriesPerLevel = 10;

	/* Insert a new score. Keeps array sorted and trimmed to top 10.
	   Returns the rank (1-indexed) or -1 if score did not make top 10. */
	int32 AddEntry(FName LevelName, const FLeaderboardEntry& NewEntry);

	/* Read entries for a level. Returns empty array if no scores exist. */
	TArray<FLeaderboardEntry> GetEntries(FName LevelName) const;
};
