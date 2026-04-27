/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar

File Name: EndScreenWidget.cpp
Author: Kushal Poudel and Alok Poudel
Last Modified: April 26, 2026

Description: This is the End Screen Widget which is shown when the game ends , either by
			 winning or by getting caught . It shows a title (ESCAPED or BUSTED) , the items
			 collected , time taken and final score for win condition . It also has buttons
			 for submiting score , retrying the level , going back to main menu and quiting
			 the game.

			 The submit score button opens up a NameEntryWidget popup so player can enter
			 their name and save thier score to the leaderboard . The retry button reloads
			 the current level and the main menu button takes the player back to the main
			 menu level. Each button also plays a click sound for audio feedback.
*/

#include "EndScreenWidget.h"
#include "NameEntryWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

/* NativeConstruct is a UMG funtion that is called when the widget is being created
   and is ready to use . Here we bind all the button click events to thier handler
   functions so that when player clicks any button the right function is called .
   We use AddDynamic which is a UE macro to bind the click event to our funtion .

   Issue: Some buttons were  null and game keeps on crashing -- kushal  

   Fixed: Each button is checked first to make sure it is not null before binding because
   if any button is missing in the blueprint the bind would crash the game.
*/
void UEndScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Binding all the button click events to thier handler funtions
	if (Btn_SubmitScore) Btn_SubmitScore->OnClicked.AddDynamic(this, &UEndScreenWidget::OnSubmitScoreClicked);
	if (Btn_Retry)       Btn_Retry->OnClicked.AddDynamic(this, &UEndScreenWidget::OnRetryClicked);
	if (Btn_MainMenu)    Btn_MainMenu->OnClicked.AddDynamic(this, &UEndScreenWidget::OnMainMenuClicked);
	if (Btn_Quit)        Btn_Quit->OnClicked.AddDynamic(this, &UEndScreenWidget::OnQuitClicked);
}

/* ShowWinScreen is called by the GameMode when the player wins the game . We cache
   the items , time and score values so that we can use them later when player clicks
   the submit score button.Then we set the title to "ESCAPED" with the win color ,
   show the items collected , the time taken and the final score on the widget. We
   also make sure the submit score button is visible and enabled so player can click
   it to enter thier name for the leaderboard.
*/
void UEndScreenWidget::ShowWinScreen(int32 ItemsCollected, int32 ItemsRequired, float TimeTaken, int32 FinalScore)
{
	// Caching the values so we can use them later when submit score is clicked.
	CachedItems = ItemsCollected;
	CachedTime = TimeTaken;
	CachedScore = FinalScore;

	// Setting the title to ''ESCAPED'' and applying the win color (usually green for win , red for loss)
	if (Txt_Title)
	{
		Txt_Title->SetText(FText::FromString(TEXT("ESCAPED!")));
		Txt_Title->SetColorAndOpacity(FSlateColor(WinColor));
	}

	// Showing how many items the player collected out of total required
	if (Txt_Stats)
		Txt_Stats->SetText(FText::FromString(FString::Printf(TEXT("Items Collected: %d / %d"), ItemsCollected, ItemsRequired)));

	// Showing time taken in MM:SS format using the FormatTime helper
	if (Txt_Time)
		Txt_Time->SetText(FText::FromString(FString::Printf(TEXT("Time: %s"), *FormatTime(TimeTaken))));

	// Showing the final score and making sure the score text is visible 
	if (Txt_Score)
	{
		Txt_Score->SetText(FText::FromString(FString::Printf(TEXT("Score: %d"), FinalScore)));
		Txt_Score->SetVisibility(ESlateVisibility::Visible);
	}

	// Showing and enabling the submit score button so player can save thier score/
	if (Btn_SubmitScore)
	{
		Btn_SubmitScore->SetVisibility(ESlateVisibility::Visible);
		Btn_SubmitScore->SetIsEnabled(true);
	}
}

/* ShowLoseScreen is called by the GameMode when the player gets caught by the guard.
   It is similiar to ShowWinScreen but we donot show the score because the player did
   not finish the level . We also hide the submit score button because there is no
   score to submit on a lose . The title is set to "BUSTED" with the lose color (usually
   red) so player knows they lost.
*/
void UEndScreenWidget::ShowLoseScreen(int32 ItemsCollected, int32 ItemsRequired, float TimeTaken)
{
	// Setting the title to BUSTED and applying the lose color (usually red)
	if (Txt_Title)
	{
		Txt_Title->SetText(FText::FromString(TEXT("BUSTED!")));
		Txt_Title->SetColorAndOpacity(FSlateColor(LoseColor));
	}

	// Still showing the items collected so player can see thier progress--
	if (Txt_Stats)
		Txt_Stats->SetText(FText::FromString(FString::Printf(TEXT("Items Collected: %d / %d"), ItemsCollected, ItemsRequired)));

	// Showing the time taken till they got caught
	if (Txt_Time)
		Txt_Time->SetText(FText::FromString(FString::Printf(TEXT("Time: %s"), *FormatTime(TimeTaken))));

	// Hiding the score text because there is no score on a lose 
	if (Txt_Score)
		Txt_Score->SetVisibility(ESlateVisibility::Collapsed);

	// Hiding submit score button because there is no score to submit
	if (Btn_SubmitScore)
		Btn_SubmitScore->SetVisibility(ESlateVisibility::Collapsed);
}

/* OnSubmitScoreClicked is called when the player clicks the submit score button on
   the win screen . This opens up the NameEntryWidget popup where player can type
   thier name and save the score to the leaderboard. We pass the cached score , time
   and items to the popup so it has all the info it needs to save . We also disable
   the submit button after clicking so player cannot submit twice for the same run.
*/
void UEndScreenWidget::OnSubmitScoreClicked()
{
	// Playing click sound for audio feedback
	PlayClickSound();

	// Safety check , if the popup widget class is not set we cannot do anything
	if (!NameEntryWidgetClass) return;

	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	// Creating the name entry popup widget instance 
	UNameEntryWidget* Popup = CreateWidget<UNameEntryWidget>(PC, NameEntryWidgetClass);
	if (!Popup) return;

	// Getting the current level name to use as the leaderboard key 
	FName LevelName = FName(*UGameplayStatics::GetCurrentLevelName(GetWorld(), true));

	// Passing all the score data to the popup so it can save it later
	Popup->ConfigureForScore(LevelName, CachedScore, CachedTime, CachedItems);

	// Adding to viewport with a high z-order so it shows on top of the end screen
	Popup->AddToViewport(100);

	// Disabling submit button so player cannot submit twice for same run
	if (Btn_SubmitScore) Btn_SubmitScore->SetIsEnabled(false);
}

/* OnRetryClicked is called when the player clicks the retry button . It unpauses
   the game and reloads the current level so the player can try again from the start.
*/
void UEndScreenWidget::OnRetryClicked()
{
	PlayClickSound();

	// Unpausing and removing widget before loading the level again
	PrepareForLevelTransition();

	// Getting current level name and reloading it
	FString CurrentLevel = UGameplayStatics::GetCurrentLevelName(GetWorld(), true);
	UGameplayStatics::OpenLevel(GetWorld(), FName(*CurrentLevel));
}

/* OnMainMenuClicked is called when player clicks main menu button . It unpauses
   the game and loads the main menu level which is set in the blueprint defaults.
*/
void UEndScreenWidget::OnMainMenuClicked()
{
	PlayClickSound();

	// Unpausing and removing widget before loading main menu level
	PrepareForLevelTransition();

	// Loading the main menu level using the level name set in blueprint
	UGameplayStatics::OpenLevel(GetWorld(), MainMenuLevelName);
}

/* OnQuitClicked is called when player clicks the quit button . It unpauses the game
   first because if game is paused the quit command might not work properly , then it
   runs the quit console command which closes the game.
*/
void UEndScreenWidget::OnQuitClicked()
{
	PlayClickSound();

	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		// Unpausing first so quit command works properly 
		UGameplayStatics::SetGamePaused(GetWorld(), false);

		// Running quit console command to close the game!!
		PC->ConsoleCommand(TEXT("quit"));
	}
}

/* PrepareForLevelTransition is a helper funtion which is called before loading a
   diffrent level. It unpauses the game so the new level loads in normal state and
   removes the end screen widget from the viewport so it doesnot stay on screen
   during level transition.
*/
void UEndScreenWidget::PrepareForLevelTransition()
{
	// Unpausing the game before loading new level 
	UGameplayStatics::SetGamePaused(GetWorld(), false);

	// Removing this widget from viewport so it doesnot show during transition
	RemoveFromParent();
}

/* FormatTime is a helper funtion that converts seconds into MM:SS format string .
   We use FloorToInt to get whole minutes and seconds , and the modulo 60 makes sure
   the seconds part is always between 0 and 59. The %02d format makes sure each part
   is always 2 digits like 01:05 instead of 1:5 .
*/
FString UEndScreenWidget::FormatTime(float Seconds) const
{
	// Safety check , if seconds is negative we just set it to zero. 
	if (Seconds < 0.f) Seconds = 0.f;

	// Getting whole minutes by dividing by 60 and flooring.
	int32 Minutes = FMath::FloorToInt(Seconds / 60.f);

	// Getting remainder seconds using modulo 60. 
	int32 Secs = FMath::FloorToInt(Seconds) % 60;

	// Formating as MM:SS with leading zeros . 
	return FString::Printf(TEXT("%02d:%02d"), Minutes, Secs);
}

/* PlayClickSound is a small helper funtion that plays the button click sound .
   We use PlaySound2D so the sound is played at full volume regardless of the
   players location in the world .The sound is set in the blueprint defaults.
*/
void UEndScreenWidget::PlayClickSound()
{
	if (ButtonClickSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), ButtonClickSound);
	}
}