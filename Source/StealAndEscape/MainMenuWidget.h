/*
Project Name: Steal and Escape: A 3D top-down semi-escape stealth game developed in Unreal Engine
Course: CSCI 491 Seminar
File Name: MainMenuWidget.h
Author: Kushal Poudel and Alok Poudel
Last Modified: April 15, 2026

Description: Main menu widget for the title screen. Contains five main buttons
             (Play, Load, Options, Credits, Quit) and handles navigation to
             submenu panels for Level Select, Load, Options, and Credits.

             All button widgets and panel widgets are bound by name from the
             UMG Widget Blueprint that inherits from this class. BindWidget
             means the UMG designer MUST create widgets with these exact names
             or compilation of the Widget Blueprint will fail which catches
             missing bindings early.

             Level names, save slot names, and other configurable values are
             exposed as EditDefaultsOnly so the team can tune them per build
             without recompiling C++.
*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UButton;
class UWidget;
class UTextBlock;
class USlider;
class UCheckBox;
class UVerticalBox;

UCLASS()
class STEALANDESCAPE_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Called after the underlying Slate widget is constructed. Binds button
	// click delegates to their handler functions.
	virtual void NativeConstruct() override;

protected:
	/* ======================================================================
	   MAIN PANEL BUTTONS
	   Bound by name - the UMG asset must contain Button widgets with these
	   exact variable names or the Widget Blueprint will fail to compile.
	   ====================================================================== */

	UPROPERTY(meta = (BindWidget))
		UButton* Btn_Play;

	UPROPERTY(meta = (BindWidget))
		UButton* Btn_Load;

	UPROPERTY(meta = (BindWidget))
		UButton* Btn_Options;

	UPROPERTY(meta = (BindWidget))
		UButton* Btn_Credits;

	UPROPERTY(meta = (BindWidget))
		UButton* Btn_Quit;

	/* ======================================================================
	   PANELS - one per menu screen. Only the active panel is visible.
	   Switching panels is done by setting Visibility on each.
	   ====================================================================== */

	UPROPERTY(meta = (BindWidget))
		UWidget* Panel_Main;

	UPROPERTY(meta = (BindWidget))
		UWidget* Panel_LevelSelect;

	UPROPERTY(meta = (BindWidget))
		UWidget* Panel_Load;

	UPROPERTY(meta = (BindWidget))
		UWidget* Panel_Options;

	UPROPERTY(meta = (BindWidget))
		UWidget* Panel_Credits;

	/* ======================================================================
	   LEVEL SELECT PANEL - one button per level, plus a Back button.
	   ====================================================================== */

	UPROPERTY(meta = (BindWidget))
		UButton* Btn_Level1;

	UPROPERTY(meta = (BindWidget))
		UButton* Btn_Level2;

	UPROPERTY(meta = (BindWidget))
		UButton* Btn_Level3;

	UPROPERTY(meta = (BindWidget))
		UButton* Btn_LevelSelectBack;

	/* ======================================================================
	   LOAD PANEL - three save slots plus a Back button.
	   Slot label text is updated at runtime from save files if they exist.
	   ====================================================================== */

	UPROPERTY(meta = (BindWidget))
		UButton* Btn_LoadSlot1;

	UPROPERTY(meta = (BindWidget))
		UButton* Btn_LoadSlot2;

	UPROPERTY(meta = (BindWidget))
		UButton* Btn_LoadSlot3;

	UPROPERTY(meta = (BindWidget))
		UTextBlock* Txt_LoadSlot1;

	UPROPERTY(meta = (BindWidget))
		UTextBlock* Txt_LoadSlot2;

	UPROPERTY(meta = (BindWidget))
		UTextBlock* Txt_LoadSlot3;

	UPROPERTY(meta = (BindWidget))
		UButton* Btn_LoadBack;

	/* ======================================================================
	   OPTIONS PANEL - master volume slider, fullscreen checkbox, Back.
	   ====================================================================== */

	UPROPERTY(meta = (BindWidget))
		USlider* Slider_MasterVolume;

	UPROPERTY(meta = (BindWidget))
		UCheckBox* Check_Fullscreen;

	UPROPERTY(meta = (BindWidget))
		UButton* Btn_OptionsBack;

	/* ======================================================================
	   CREDITS PANEL - just a Back button; the credits text is static in UMG.
	   ====================================================================== */

	UPROPERTY(meta = (BindWidget))
		UButton* Btn_CreditsBack;

	/* ======================================================================
	   CONFIGURABLE LEVEL NAMES
	   Set these on the Widget Blueprint defaults. Using FName so Unreal's
	   OpenLevel function can accept them directly.
	   ====================================================================== */

	UPROPERTY(EditDefaultsOnly, Category = "Levels")
		FName Level1Name = TEXT("L_Level01");

	UPROPERTY(EditDefaultsOnly, Category = "Levels")
		FName Level2Name = TEXT("L_Level02");

	UPROPERTY(EditDefaultsOnly, Category = "Levels")
		FName Level3Name = TEXT("L_Level03");

	/* ======================================================================
	   SAVE SLOT NAMES - fixed identifiers used with SaveGameToSlot.
	   ====================================================================== */

	static const FString SaveSlot1;
	static const FString SaveSlot2;
	static const FString SaveSlot3;

private:
	/* ======================================================================
	   BUTTON HANDLERS - one per button. Must be UFUNCTION because OnClicked
	   is a dynamic multicast delegate which requires reflected functions.
	   ====================================================================== */

	// Main panel
	UFUNCTION() void OnPlayClicked();
	UFUNCTION() void OnLoadClicked();
	UFUNCTION() void OnOptionsClicked();
	UFUNCTION() void OnCreditsClicked();
	UFUNCTION() void OnQuitClicked();

	// Level select
	UFUNCTION() void OnLevel1Clicked();
	UFUNCTION() void OnLevel2Clicked();
	UFUNCTION() void OnLevel3Clicked();
	UFUNCTION() void OnLevelSelectBackClicked();

	// Load
	UFUNCTION() void OnLoadSlot1Clicked();
	UFUNCTION() void OnLoadSlot2Clicked();
	UFUNCTION() void OnLoadSlot3Clicked();
	UFUNCTION() void OnLoadBackClicked();

	// Options
	UFUNCTION() void OnMasterVolumeChanged(float NewValue);
	UFUNCTION() void OnFullscreenChanged(bool bIsChecked);
	UFUNCTION() void OnOptionsBackClicked();

	// Credits
	UFUNCTION() void OnCreditsBackClicked();

	/* ======================================================================
	   HELPERS
	   ====================================================================== */

	// Hides every panel then shows the one passed in. Null-safe.
	void ShowPanel(UWidget* PanelToShow);

	// Opens the given level and removes this menu from the viewport.
	void LaunchLevel(FName LevelName);

	// Reads the save file from the given slot and updates its label text.
	// If the slot is empty the label shows "Empty Slot".
	void RefreshSaveSlotLabel(const FString& SlotName, UTextBlock* LabelWidget);

	// Called when the Load panel is opened - refreshes all three slot labels.
	void RefreshAllSaveSlotLabels();

	// Loads a save from the given slot and opens the saved level.
	void LoadFromSlot(const FString& SlotName);

	// Applies current options values from GameUserSettings into the widgets
	// so the sliders and checkboxes reflect saved settings on menu open.
	void InitializeOptionsFromSettings();
};
