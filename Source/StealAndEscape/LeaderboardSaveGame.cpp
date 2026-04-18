/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: LeaderboardSaveGame.cpp
Author: Kushal Poudel and Alok Poudel
Last Modified: April 18, 2026

Description: Implementation of ULeaderboardSaveGame. Handles insertion into
             the per-level sorted array, keeping it capped at MaxEntriesPerLevel.
*/

#include "LeaderboardSaveGame.h"

const FString ULeaderboardSaveGame::SlotName = TEXT("Leaderboard_Slot");

int32 ULeaderboardSaveGame::AddEntry(FName LevelName, const FLeaderboardEntry& NewEntry)
{
	// Find or create the entries array for this level
	FLeaderboardEntries& LevelEntries = ScoresPerLevel.FindOrAdd(LevelName);

	// Find insertion index - first position where existing score is less
	// than the new score. Keeps array sorted high-to-low.
	int32 InsertIndex = LevelEntries.Entries.Num();
	for (int32 i = 0; i < LevelEntries.Entries.Num(); i++)
	{
		if (NewEntry.Score > LevelEntries.Entries[i].Score)
		{
			InsertIndex = i;
			break;
		}
	}

	// If score would fall beyond the top 10, skip
	if (InsertIndex >= MaxEntriesPerLevel)
	{
		return -1;
	}

	// Insert and trim to max length
	LevelEntries.Entries.Insert(NewEntry, InsertIndex);
	if (LevelEntries.Entries.Num() > MaxEntriesPerLevel)
	{
		LevelEntries.Entries.SetNum(MaxEntriesPerLevel);
	}

	// Return 1-indexed rank
	return InsertIndex + 1;
}

TArray<FLeaderboardEntry> ULeaderboardSaveGame::GetEntries(FName LevelName) const
{
	const FLeaderboardEntries* Found = ScoresPerLevel.Find(LevelName);
	if (!Found)
	{
		return TArray<FLeaderboardEntry>();
	}
	return Found->Entries;
}
