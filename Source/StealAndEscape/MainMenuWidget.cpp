/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: MainMenuWidget.cpp
Author: Kushal Poudel and Alok Poudel
Last Modified: April 15, 2026

Description: Implementation of the main menu widget. Handles navigation between
             panels (Main, Level Select, Load, Options, Credits), launches
             gameplay levels, loads saves, and applies options changes through
             UGameUserSettings.

             All button bindings are done in NativeConstruct after the underlying
             Slate widgets have been created. Using AddDynamic on each button's
             OnClicked delegate is the standard C++ pattern for UMG buttons.
*/

#include "MainMenuWidget.h"
#include "StealAndEscapeSaveGame.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Slider.h"
#include "Components/CheckBox.h"
#include "Components/Widget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameUserSettings.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

// Save slot identifier constants - used by SaveGameToSlot / LoadGameFromSlot
const FString UMainMenuWidget::SaveSlot1 = TEXT("SaveSlot1");
const FString UMainMenuWidget::SaveSlot2 = TEXT("SaveSlot2");
const FString UMainMenuWidget::SaveSlot3 = TEXT("SaveSlot3");

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	/* Main panel button bindings
	   AddDynamic macro registers a UFUNCTION as a delegate target which is
	   required because OnClicked is a dynamic multicast delegate. */
	if (Btn_Play)    Btn_Play->OnClicked.AddDynamic(this, &UMainMenuWidget::OnPlayClicked);
	if (Btn_Load)    Btn_Load->OnClicked.AddDynamic(this, &UMainMenuWidget::OnLoadClicked);
	if (Btn_Options) Btn_Options->OnClicked.AddDynamic(this, &UMainMenuWidget::OnOptionsClicked);
	if (Btn_Credits) Btn_Credits->OnClicked.AddDynamic(this, &UMainMenuWidget::OnCreditsClicked);
	if (Btn_Quit)    Btn_Quit->OnClicked.AddDynamic(this, &UMainMenuWidget::OnQuitClicked);

	// Level select bindings
	if (Btn_Level1)           Btn_Level1->OnClicked.AddDynamic(this, &UMainMenuWidget::OnLevel1Clicked);
	if (Btn_Level2)           Btn_Level2->OnClicked.AddDynamic(this, &UMainMenuWidget::OnLevel2Clicked);
	if (Btn_Level3)           Btn_Level3->OnClicked.AddDynamic(this, &UMainMenuWidget::OnLevel3Clicked);
	if (Btn_LevelSelectBack)  Btn_LevelSelectBack->OnClicked.AddDynamic(this, &UMainMenuWidget::OnLevelSelectBackClicked);

	// Load bindings
	if (Btn_LoadSlot1) Btn_LoadSlot1->OnClicked.AddDynamic(this, &UMainMenuWidget::OnLoadSlot1Clicked);
	if (Btn_LoadSlot2) Btn_LoadSlot2->OnClicked.AddDynamic(this, &UMainMenuWidget::OnLoadSlot2Clicked);
	if (Btn_LoadSlot3) Btn_LoadSlot3->OnClicked.AddDynamic(this, &UMainMenuWidget::OnLoadSlot3Clicked);
	if (Btn_LoadBack)  Btn_LoadBack->OnClicked.AddDynamic(this, &UMainMenuWidget::OnLoadBackClicked);

	// Options bindings - slider uses OnValueChanged not OnClicked
	if (Slider_MasterVolume) Slider_MasterVolume->OnValueChanged.AddDynamic(this, &UMainMenuWidget::OnMasterVolumeChanged);
	if (Check_Fullscreen)    Check_Fullscreen->OnCheckStateChanged.AddDynamic(this, &UMainMenuWidget::OnFullscreenChanged);
	if (Btn_OptionsBack)     Btn_OptionsBack->OnClicked.AddDynamic(this, &UMainMenuWidget::OnOptionsBackClicked);

	// Credits bindings
	if (Btn_CreditsBack) Btn_CreditsBack->OnClicked.AddDynamic(this, &UMainMenuWidget::OnCreditsBackClicked);

	// Show only the main panel on initial construction
	ShowPanel(Panel_Main);

	// Pull saved options into the option widgets so they reflect current settings
	InitializeOptionsFromSettings();
}

/* =============================================================================
   MAIN PANEL BUTTONS
   ============================================================================= */

void UMainMenuWidget::OnPlayClicked()
{
	// Play opens the Level Select submenu rather than directly launching a level
	ShowPanel(Panel_LevelSelect);
}

void UMainMenuWidget::OnLoadClicked()
{
	// Refresh slot labels so they show the current saved data before displaying
	RefreshAllSaveSlotLabels();
	ShowPanel(Panel_Load);
}

void UMainMenuWidget::OnOptionsClicked()
{
	ShowPanel(Panel_Options);
}

void UMainMenuWidget::OnCreditsClicked()
{
	ShowPanel(Panel_Credits);
}

void UMainMenuWidget::OnQuitClicked()
{
	// Using ConsoleCommand("quit") is the simplest cross-platform way to quit
	// from a widget. UKismetSystemLibrary::QuitGame also works if preferred.
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		PC->ConsoleCommand(TEXT("quit"));
	}
}

/* =============================================================================
   LEVEL SELECT PANEL
   ============================================================================= */

void UMainMenuWidget::OnLevel1Clicked() { LaunchLevel(Level1Name); }
void UMainMenuWidget::OnLevel2Clicked() { LaunchLevel(Level2Name); }
void UMainMenuWidget::OnLevel3Clicked() { LaunchLevel(Level3Name); }

void UMainMenuWidget::OnLevelSelectBackClicked()
{
	ShowPanel(Panel_Main);
}

/* =============================================================================
   LOAD PANEL
   ============================================================================= */

void UMainMenuWidget::OnLoadSlot1Clicked() { LoadFromSlot(SaveSlot1); }
void UMainMenuWidget::OnLoadSlot2Clicked() { LoadFromSlot(SaveSlot2); }
void UMainMenuWidget::OnLoadSlot3Clicked() { LoadFromSlot(SaveSlot3); }

void UMainMenuWidget::OnLoadBackClicked()
{
	ShowPanel(Panel_Main);
}

/* =============================================================================
   OPTIONS PANEL
   ============================================================================= */

void UMainMenuWidget::OnMasterVolumeChanged(float NewValue)
{
	/* Setting the master sound class volume live so the player can hear the
	   change as they drag the slider. This uses a console command because the
	   Sound Class system requires editor-defined assets; for our use case the
	   command approach is the least-setup way to get working volume control.

	   Range is 0.0 to 1.0 which matches the slider's default min/max. */
	if (GEngine)
	{
		FString Cmd = FString::Printf(TEXT("SetMasterVolume %f"), NewValue);
		GEngine->Exec(GetWorld(), *Cmd);
	}

	// Persist the value in GameUserSettings so it survives app restart
	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
	if (Settings)
	{
		// Note: UGameUserSettings has no built-in volume slot, so we only persist
		// on Apply. For a production system a custom settings subclass would
		// store volume. For now the console command applies the value live.
	}
}

void UMainMenuWidget::OnFullscreenChanged(bool bIsChecked)
{
	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
	if (!Settings) return;

	/* Fullscreen toggles between Fullscreen and Windowed modes. Windowed
	   Fullscreen is often smoother during alt-tab so we use that here.
	   Apply the change then save settings to disk. */
	Settings->SetFullscreenMode(bIsChecked ? EWindowMode::WindowedFullscreen : EWindowMode::Windowed);
	Settings->ApplySettings(false);
	Settings->SaveSettings();
}

void UMainMenuWidget::OnOptionsBackClicked()
{
	ShowPanel(Panel_Main);
}

/* =============================================================================
   CREDITS PANEL
   ============================================================================= */

void UMainMenuWidget::OnCreditsBackClicked()
{
	ShowPanel(Panel_Main);
}

/* =============================================================================
   HELPERS
   ============================================================================= */

void UMainMenuWidget::ShowPanel(UWidget* PanelToShow)
{
	/* Hide every panel first then show the target. This guarantees only one
	   panel is ever visible regardless of the previous state. Using Collapsed
	   rather than Hidden so the hidden panels do not take up layout space. */
	if (Panel_Main)        Panel_Main->SetVisibility(ESlateVisibility::Collapsed);
	if (Panel_LevelSelect) Panel_LevelSelect->SetVisibility(ESlateVisibility::Collapsed);
	if (Panel_Load)        Panel_Load->SetVisibility(ESlateVisibility::Collapsed);
	if (Panel_Options)     Panel_Options->SetVisibility(ESlateVisibility::Collapsed);
	if (Panel_Credits)     Panel_Credits->SetVisibility(ESlateVisibility::Collapsed);

	if (PanelToShow)
	{
		PanelToShow->SetVisibility(ESlateVisibility::Visible);
	}
}

void UMainMenuWidget::LaunchLevel(FName LevelName)
{
	if (LevelName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenuWidget - Tried to launch a level with no name set!"));
		return;
	}

	// Remove the menu from the viewport so it does not overlay the gameplay level
	RemoveFromParent();

	// OpenLevel loads the new map which also destroys all current actors including
	// this widget's outer so there is no further cleanup needed here
	UGameplayStatics::OpenLevel(GetWorld(), LevelName);
}

void UMainMenuWidget::RefreshSaveSlotLabel(const FString& SlotName, UTextBlock* LabelWidget)
{
	if (!LabelWidget) return;

	// DoesSaveGameExist is a cheap disk check that returns true if the file is on disk
	if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(SlotName, 0);
		UStealAndEscapeSaveGame* Save = Cast<UStealAndEscapeSaveGame>(Loaded);
		if (Save)
		{
			// Display the saved info so the player can pick the right slot
			FString Label = FString::Printf(TEXT("%s - %s (%d items) - %s"),
				*SlotName,
				*Save->SavedLevelName.ToString(),
				Save->SavedCollectedItems,
				*Save->SaveDate);
			LabelWidget->SetText(FText::FromString(Label));
			return;
		}
	}

	// No save in this slot - show placeholder text
	LabelWidget->SetText(FText::FromString(FString::Printf(TEXT("%s - Empty Slot"), *SlotName)));
}

void UMainMenuWidget::RefreshAllSaveSlotLabels()
{
	RefreshSaveSlotLabel(SaveSlot1, Txt_LoadSlot1);
	RefreshSaveSlotLabel(SaveSlot2, Txt_LoadSlot2);
	RefreshSaveSlotLabel(SaveSlot3, Txt_LoadSlot3);
}

void UMainMenuWidget::LoadFromSlot(const FString& SlotName)
{
	// Check the slot exists before attempting to load to avoid noisy log warnings
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenuWidget - Load requested for empty slot: %s"), *SlotName);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow,
				FString::Printf(TEXT("Save slot '%s' is empty"), *SlotName));
		}
		return;
	}

	USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(SlotName, 0);
	UStealAndEscapeSaveGame* Save = Cast<UStealAndEscapeSaveGame>(Loaded);
	if (!Save)
	{
		UE_LOG(LogTemp, Error, TEXT("MainMenuWidget - Failed to cast loaded save for slot %s"), *SlotName);
		return;
	}

	/* Hand off the loaded level name to the gameplay GameMode via OpenLevel.
	   For restoring the collected-items count on the new level we would need
	   to pass it through a save-state singleton or the game instance. Leaving
	   that as a follow-up TODO once GameInstance is added - for now the load
	   restores which level was active. */
	UE_LOG(LogTemp, Log, TEXT("MainMenuWidget - Loading save: Level=%s, Items=%d"),
		*Save->SavedLevelName.ToString(), Save->SavedCollectedItems);

	LaunchLevel(Save->SavedLevelName);
}

void UMainMenuWidget::InitializeOptionsFromSettings()
{
	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
	if (!Settings) return;

	// Set the fullscreen checkbox to match the current window mode
	if (Check_Fullscreen)
	{
		const bool bIsFullscreen =
			Settings->GetFullscreenMode() == EWindowMode::Fullscreen ||
			Settings->GetFullscreenMode() == EWindowMode::WindowedFullscreen;
		Check_Fullscreen->SetIsChecked(bIsFullscreen);
	}

	// Default the volume slider to 1.0 since UGameUserSettings does not store it
	if (Slider_MasterVolume)
	{
		Slider_MasterVolume->SetValue(1.0f);
	}
}
