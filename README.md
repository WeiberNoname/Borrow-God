# Borrowed Gods

**Steal their power. Bear their curse. Find your way home.**

A native Unreal Engine 5.7 C++ RPG prototype. Windows, keyboard/mouse, and controller are the first playable target. All character and environment geometry is intentionally placeholder art. Combat cues are synthesized at runtime. No Tripo account, generated assets, paid plugins, backend, or network connection is required to play.

The visual pass gives the island a drowned sanctuary identity: weathered blue stone, bronze compass inlays, fluted ruins, coastal rock, rippling teal water, braziers, and the Argo offshore. The captain and enemy roles have distinct armour, cloaks, weapons, and shields. Divine effects and attack warnings use open glowing rings. These are replaceable procedural/primitive models, not final rigged character art.

## Play

Double-click **Play Borrowed Gods.bat**. It prefers the standalone Windows game in `Builds/Windows`, and falls back to the installed Unreal Engine runtime if no package exists. Choose a god, then click **Set Sail** or press Enter. The standalone build does not require Unreal Editor.

To edit the project, open `BorrowedGods.uproject` in Unreal Engine 5.7.4. Click Play to run the level. The island is assembled at runtime, so the saved map appears empty before Play.

If the C++ binaries need rebuilding, run `powershell -ExecutionPolicy Bypass -File Scripts/Build.ps1`. Requires Visual Studio 2022 with C++ game development tools and a Windows SDK. All engine paths can be changed in the launch/build scripts.

## Prototype loop

1. At the Argo, choose a god and hear Nera's request: bring medicine for Tomas, the injured sailor. Two delivered doses return him to duty.
2. Clear the shore, then choose the infirmary (three enemies, one flask) or the guarded vault (five enemies including three sentinels, two flasks).
3. Choose a shrine gift and a relic that changes how you fight. Switch borrowed powers freely; every curse remains active.
4. Recover the medicine. Pay Poseidon one flask for +20 divine damage and extra Tidebreak range, or preserve it and face three warned sea strikes during the guardian fight.
5. Choose your second shrine gift, defeat Talos and his guards, then return to the southwest green beacon and press E.
6. Deliver surviving medicine to Nera, see Tomas's saved condition change, and hear the crew's response to your choices. Spend banked obols on permanent upgrades.
7. Recover three chart fragments across successful voyages to reveal the course home. The two encounter routes currently use the same island blockout.

An escape banks all carried obols. Death or ending a voyage banks one quarter, rounded down. Already banked currency and permanent upgrades are never lost. Quitting does not preserve an in-progress expedition.

## Controls

| Input | Action |
|---|---|
| WASD | Camera-relative movement |
| Mouse | Aim |
| Left mouse, hold | Three-hit melee combo; final hit does extra damage and staggers |
| Space | Dodge, 30 stamina, brief invulnerability |
| Q | Stolen divine power |
| E | Board ship at unlocked green extraction beacon |
| R | Cycle borrowed powers |
| 1 / 2 / 3 | Select a god/blessing in menus; switch to an owned god in combat |
| Enter | Begin briefing / confirm departure / return to harbor |
| Tab | Journal; freezes combat |
| C | Crew conversations and voyage ledger |
| Escape | Pause or close an overlay |
| F1 | Controls and tutorial |

Pause > **Input & Accessibility** provides keyboard/mouse rebinding with conflict swapping, reduced motion, sound mute, and assistance. Escape and 1/2/3 remain reserved. Reduced motion removes hit-stop and camera smoothing. Assistance reduces incoming damage by 35% and increases enemy warning time by 25%; rewards are unchanged.

Controller: left stick moves, right stick aims, X attacks, A dodges, RT casts, LB/RB switch powers, Y boards, Start pauses, and Back opens the journal. D-pad selects menu controls, A confirms, and B goes back. The on-screen focus border shows the selected control. Controller bindings use Xbox-style names and are fixed in this prototype.

## Relics and enemy roles

Choose one relic per voyage:

- **Stormglass:** Wet/lightning conduction arcs for 18 damage to neighbors within 320 units. Plan to borrow both Poseidon and Zeus.
- **Ferryman's Thread:** dodging from inside an enemy warning in its final 0.35 seconds heals 8 and sheds 12 wrath, with a 1.5-second recovery.
- **Oathstone:** a connecting third melee strike interrupts ordinary enemies and restores 24 stamina. Talos cannot be interrupted.

Sentinels reduce ordinary melee damage by 65% while their shield is closed. Their attack windup, the period after their attack, and your combo finisher expose them. Powers bypass shields. Purple channelers retreat when approached. Talos's pursuit warning tracks you until its final half-second, then locks in place. Melee has a short windup before damage; dodging can cancel it.

## Powers and curses

| God | Power | Curse |
|---|---|---|
| Poseidon | Tidebreak: area damage, pushback, three-second slow, five-second Wet status | Wrath increases faster over time; Tidebreak adds extra wrath |
| Zeus | Stormcall: strongest burst, largest area; consumes Wet for 26 bonus damage | Every divine cast costs eight health, never fatal by itself |
| Hades | Soul Harvest: area damage and healing per target hit | Maximum stamina reduced to 75 |

Tidebreak followed by Stormcall conducts lightning through wet targets. Each god has its own cooldown. Curses stay active when switching, and each additional god increases passive wrath. Shrine refusal can increase maximum health, improve damage, or reduce cooldowns depending on the available offers.

Wrath reaches 100 to trigger a delayed, avoidable strike at the player's last position. Killing an enemy reduces wrath. Talos changes attack timing and appearance below half health. Health, stamina, cooldowns, curses, objectives, carried currency, and encounter progress are visible in the HUD.

## Saves

Local Unreal SaveGame slot: `BorrowedGods_Profile_v1`. Usually under `Saved/SaveGames` in the editor runtime, or `%LOCALAPPDATA%/BorrowedGods/Saved/SaveGames` in the standalone build. Profile includes banked obols, voyage/win counts, selected god, upgrades, accessibility preferences, custom key bindings, homeward chart progress, Tomas's recovery, delivered medicine, the last bargain/route, and the last six completed voyage records. Existing version-1 profiles keep their previous progress and begin with Tomas injured. There is no cloud save or mid-expedition resume.

## Code layout

- `BGGameMode`: island blockout, expedition state machine, rewards, save/load, actions, integration smoke test.
- `BGCharacter`: movement, attacks, dodge, abilities, replaceable character appearance.
- `BGEnemy`: melee/ranged telegraphs and guardian behavior.
- `BGHUD`: resolution-scaled native Canvas UI, custom icons, chart, menus, bars, and clickable controls.
- `BGPlayerController`: keyboard/mouse input and aiming.
- `BGIsland` and `BGVisuals`: sanctuary composition, non-colliding presentation props, procedural rings/rock, and costume parts.
- `BGTypes`: shared game rules and identifiers.
- `BGSaveGame`: versioned persistent profile.

## Replacing assets

See `Docs/ART_HANDOFF.md`. Character meshes, animation classes, and presentation events are separated from combat rules. The first art pass can replace the capsule-like placeholders without changing progression or UI.

## Verification

Run `powershell -ExecutionPolicy Bypass -File Scripts/SmokeTest.ps1` for an engine integration run. It uses a separate `BorrowedGods_Automation` save slot and writes `Saved/SmokeTestResult.txt`, logs, and screenshots. It exercises state transitions and combat rules programmatically; it is not a substitute for a human balance or usability playtest.

Add `-Packaged` to test the standalone build at 1280x720. That run uses `Saved/PackagedProfile` and leaves the player profile untouched. See `Docs/VALIDATION.md` for the verified build and test coverage.

`Scripts/VisualPreview.ps1 -Packaged` captures an automated art tour at 1440x900 under `Saved/ArtProfile/Saved/Screenshots`. Omit `-Packaged` to use the editor runtime. The `-BGVisualPreview` launch argument uses the automation profile, temporarily freezes enemies, and exits after capture. `Scripts/CreateVisuals.py` recreates missing versioned stone/water materials through Unreal's Python commandlet; existing assets are preserved.

## Scope

This is one replayable island with two encounter routes and three combat encounters per voyage, not a finished campaign. No multiplayer, Android package/touch controls, final character animation, voice acting, or generated art is included. The routes change combat and supplies, and reuse the same arena geometry. The next milestone is hands-on combat, choice, and controller feedback, followed by character and animation integration.

See `Docs/CORE_SPIRIT.md` for the design pillars and the next gameplay milestone.
