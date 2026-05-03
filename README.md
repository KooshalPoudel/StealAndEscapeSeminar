# Steal and Escape

A 3D top-down semi-escape stealth game developed in Unreal Engine 4.27.2 using C++.
Built on top of Epic Games' Top-Down C++ template and extended with custom gameplay,
AI, UI, and persistence systems.

**Course:** CSCI 491 Seminar
**Authors:** Kushal Poudel & Alok Poudel

---

## About the Game

The player controls a thief in a top-down stealth environment. The objective is to
collect all required stealable items in the level and reach the exit zone without
being caught by patrolling guards. Guards use Unreal's AI Perception system to detect
the player by sight and sound, and respond with a three-state behavior machine
(Patrolling, Chasing, Investigating). Score is calculated from items collected and
elapsed time, then saved to a persistent leaderboard.

### Features

- WASD movement with Shift-to-sprint and footstep noise propagation
- Click-to-move retained from the Top-Down template
- Animation-driven grab system (G key) with a candidate list for nearby items
- AI guards with sight + hearing perception and patrol waypoints
- Main menu, name entry, pause menu, in-game HUD, and end screen widgets
- Tutorial level with step-trigger system and tutorial widget
- Persistent leaderboard via `LeaderboardSaveGame`
- Master volume slider and UI sound effects

### Controls

| Input        | Action                          |
|--------------|---------------------------------|
| WASD         | Move                            |
| Shift (hold) | Sprint                          |
| G            | Grab nearby item                |
| Mouse Click  | Click-to-move (template)        |
| Esc          | Toggle pause menu               |

---

## Required Software

- Unreal Engine **4.27.2**
- Visual Studio **2019**
  - Desktop Development with C++ workload
  - Windows 10 SDK
  - .NET Framework 4.6.2 Developer Pack

---

## Setup Instructions

1. Clone the repository from GitHub or download the zipped folder.
2. Install all required software listed above.
3. Navigate to the project folder after cloning.
4. Right-click `StealAndEscape.uproject` and select **Generate Visual Studio project files**.
5. Double-click `StealAndEscape.uproject` to open the project in Unreal Engine 4.27.2.
   The engine will automatically compile the C++ source code on first launch.
6. To open the project in Visual Studio 2019, open the generated `StealAndEscape.sln`,
   set the configuration to **Development Editor** and platform to **Win64**, then build the solution.
7. If the project fails to open or compile, delete the following folders and regenerate
   the Visual Studio project files:
   - `Binaries/`
   - `Intermediate/`
   - `.vs/`
8. Always open the `.uproject` file located inside the project directory.
   Do **not** open the Unreal Engine engine solution.

---

## Module Dependencies

Defined in `StealAndEscape.Build.cs`:

`Core`, `CoreUObject`, `Engine`, `InputCore`, `HeadMountedDisplay`,
`NavigationSystem`, `AIModule`, `UMG`, `Slate`, `SlateCore`

---

## Source Overview

### Gameplay Core
| File | Purpose |
|------|---------|
| `StealAndEscapeCharacter.{h,cpp}` | Player character (movement, sprint, grab system) |
| `StealAndEscapePlayerController.{h,cpp}` | Player input + pause menu toggle |
| `StealAndEscapeGameMode.{h,cpp}` | Win/lose state, score, timer, item tracking |
| `StealableItem.{h,cpp}` | Pickup actor with overlap detection |
| `ExitZone.{h,cpp}` | End-of-level trigger |

### AI
| File | Purpose |
|------|---------|
| `GuardCharacter.{h,cpp}` | Guard pawn |
| `GuardAIController.{h,cpp}` | Patrol / Chase / Investigate state machine with AI Perception |

### UI Widgets
| File | Purpose |
|------|---------|
| `MainMenuWidget.{h,cpp}` | Main menu, leaderboard, volume slider |
| `NameEntryWidget.{h,cpp}` | Player name input before a run |
| `PauseMenuWidget.{h,cpp}` | In-game pause menu |
| `HUDWidget.{h,cpp}` | In-game HUD (score, timer, items) |
| `EndScreenWidget.{h,cpp}` | Win/lose end screen |
| `TutorialWidget.{h,cpp}` | Tutorial step prompts |

### Tutorial
| File | Purpose |
|------|---------|
| `TutorialGameMode.{h,cpp}` | Tutorial-specific game mode |
| `TutorialStepTrigger.{h,cpp}` | Volume that advances tutorial steps |

### Main Menu
| File | Purpose |
|------|---------|
| `MainMenuGameMode.{h,cpp}` | Game mode for the main menu level |
| `MainMenuPlayerController.{h,cpp}` | UI-only input mode for the menu |

### Animation Notifies
| File | Purpose |
|------|---------|
| `AnimNotify_Footstep.{h,cpp}` | Reports footstep noise to AI hearing |
| `AnimNotifyGrabItem.{h,cpp}` | Triggers item collection at grab animation contact frame |

### Persistence
| File | Purpose |
|------|---------|
| `StealAndEscapeSaveGame.h` | Save game data structure |
| `LeaderboardSaveGame.{h,cpp}` | Persistent leaderboard storage |

---

## Notes

- Audio (item pickup, win, lose sounds) and UI widget classes are configured on
  `BP_StealAndEscapeGameMode` Class Defaults under the **Audio** and **UI** categories.
- The pause menu widget class is set on `BP_StealAndEscapePlayerController` defaults.
- Camera offset, walk speed, and run speed are all editable on the character blueprint.
