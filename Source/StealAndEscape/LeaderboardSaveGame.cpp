/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar

File Name: LeaderboardSaveGame.cpp
Author: Kushal Poudel and Alok Poudel
Last Modified: April 26, 2026

Description: Implementation of ULeaderboardSaveGame which is the savegame class that holds
			 the leaderboard data for each level. It handles inserting new entries into the
			 array and keeps the array capped at MaxEntriesPerLevel so we
			 only store the top scores to be precise top 10. The array is kept sorted high to low by score so
			 rank 1 is always at index 0.
*/

#include "LeaderboardSaveGame.h"

/* SlotName is the name of the savegame slot used by UGameplayStatics::SaveGameToSlot
   and LoadGameFromSlot . Using a static const string so the name is consitent every
   where we save or load.
*/
const FString ULeaderboardSaveGame::SlotName = TEXT("Leaderboard_Slot");

/* AddEntry inserts a new score entry into the leaderboard for the given level .Firstly
   we find or create the entries array for this level using FindOrAdd . Then we loop
   through existing entries to find the right position to insert the new score so that
   the array stays sorted from high to low .If the score is too low to make the top
   MaxEntriesPerLevel we return -1 so caller knows it didnot make the leaderboard.
   Otherwise, we insert the new entry , cut the array if it grew over the max length
   and return the 1 indexed rank/
*/
int32 ULeaderboardSaveGame::AddEntry(FName LevelName, const FLeaderboardEntry& NewEntry)
{
	// Find or create the entries array for this level using FindOrAdd 
	FLeaderboardEntries& LevelEntries = ScoresPerLevel.FindOrAdd(LevelName);
	/* Finding insertion index , first place where existing score is less than
	   the new score. This keeps the array sorted high2low.
	*/
	int32 InsertIndex = LevelEntries.Entries.Num();
	for (int32 i = 0; i < LevelEntries.Entries.Num(); i++)
	{
		if (NewEntry.Score > LevelEntries.Entries[i].Score)
		{
			InsertIndex = i;
			break;
		}
	}
	// If the score would fall beyond the top 10 we just skip it , no leaderboard spot 
	if (InsertIndex >= MaxEntriesPerLevel)
	{
		return -1;
	}
	// Inserting the new entry at the correct position 
	LevelEntries.Entries.Insert(NewEntry, InsertIndex);
	// Triming to max length so the array doesnot grow unboundedly 
	if (LevelEntries.Entries.Num() > MaxEntriesPerLevel)
	{
		LevelEntries.Entries.SetNum(MaxEntriesPerLevel);
	}
	// Here Returning 1-indexed rank so the UI can display it directly to the player 
	return InsertIndex + 1;
}

/* The method GetEntries returns the leaderboard entries for the given level. If the level has no
   entries yet we just return an empty array so caller doesnot crash on null . Otherwise
   we return a copy of the entries which the caller can use to populate the leaderboard
   display widget.
*/
TArray<FLeaderboardEntry> ULeaderboardSaveGame::GetEntries(FName LevelName) const
{
	// This is Looking up the entries for this level , returns null if level has no scores yet 
	const FLeaderboardEntries* Found = ScoresPerLevel.Find(LevelName);
	if (!Found)
	{
		// No entries yet for this level so return empty array
		return TArray<FLeaderboardEntry>();
	}
	return Found->Entries;
}