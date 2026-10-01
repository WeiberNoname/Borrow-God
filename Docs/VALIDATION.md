# Prototype validation

Verified on 2026-10-01 with Unreal Engine 5.7.4, Windows 11, DirectX 12, Visual Studio 2022, and Windows SDK 10.0.26100.0.

## Results

- Development Editor C++ target: compiled successfully.
- Development Windows game target: compiled successfully.
- Windows cook, stage, and archive: completed successfully.
- Editor runtime integration: 72 assertions passed, zero failed, at 1440x900.
- Packaged executable integration: 72 assertions passed, zero failed, at 1280x720.
- Captured UI reviewed: harbor, crew briefing, route choice, relic choice, tribute bargain, shrine offerings, combat, pause, guardian, returns, controls, input settings, crew consequences, and homeward dialogue. Packaged relic, bargain, and settings screens were reviewed at 1280x720; menu backgrounds now cover the viewport beyond the safe area.
- Visual pass: stone/water materials, armour silhouettes, faceted coastline, column details, bronze inlays, glowing rings, local fire lights, and a wider gameplay camera. The content cook completed with zero errors and zero warnings. Shader compilation is allowed to finish before art-tour capture.

The 72 gameplay assertions were rerun in both runtimes after the scene and character changes. A subsequent presentation-only adjustment softens Canvas line colours; it does not change layout or input handling and was rebuilt and visually reviewed separately.

The final packaged art check also found delayed mesh visibility during pipeline preparation. Renderer settings now create required mesh proxies immediately rather than hiding them while precaching. A subsequent packaged tour verified the floor, captain, and all three enemies visible in the first combat capture. First-use pipeline creation may still cause a startup hitch; this prototype has no production loading screen or hardware performance study.

The standalone game is `Builds/Windows/BorrowedGods.exe`. Keep the entire Windows folder together when copying it to another PC.

Final game binary SHA-256: `51CB9014D480EF179B252466EC7C6D8B60FD62BB648E04A208F26AFBBABB73ED`. The built and archived game executables match. The final packaged art tour was reviewed at 1440x900.

## Coverage

The native integration sequence checks simulated controller confirmation and menu navigation, keyboard sailing and movement, both encounter routes and their enemy composition, medicine quantities, paying and refusing tribute, warned sea strikes, melee windup, sentinel shields, all three relic behaviors, dodge invulnerability, powers and accumulating curses, conduction, guardian phase change, extraction, partial failure rewards, permanent upgrades, reactive crew dialogue, chart progress, rebinding conflict swaps/cancellation, accessibility preferences, generated audio cue creation, and SaveGame round trips.

Some checks reposition enemies, resolve attacks directly, or kill enemies to reach a scenario. The chart advances through three actual extraction transitions. Tomas's recovery and delivered-medicine counters are reset as a test fixture before the third voyage to cover the untreated outcome; that reset is test-only and never happens in player voyages. The sequence is not three human-played victories. Screenshots are captured from Unreal rendering, not mockups.

## Reproduce

```powershell
.\Scripts\Build.ps1
.\Scripts\SmokeTest.ps1
.\Scripts\Build.ps1 -Target Game
.\Scripts\Package.ps1
.\Scripts\SmokeTest.ps1 -Packaged
```

`Package.ps1 -SkipBuild` cooks and stages already compiled game binaries. `-SkipCook` also reuses the prior cook and should be used only when content and cooking requirements have not changed. Packaging may require access to the installed engine's temporary staging configuration. Build executors and shader temporary directories keep project caches local. Build scripts disable cached UBT makefiles so the linker receives the Windows SDK tool environment consistently.

Editor test log: `Saved/Logs/BorrowedGods.log`. Packaged test log: `Saved/Logs/PackagedSmoke.log`. Each test uses the isolated `BorrowedGods_Automation` slot. Packaged test artifacts live under `Saved/PackagedProfile/Saved`.

Reproduce art captures with `Scripts/VisualPreview.ps1 -Packaged`. They live under `Saved/ArtProfile/Saved/Screenshots`; the log is `Saved/Logs/ArtPreview.log`. The tour captures combat HUD, the captain, all four enemy silhouettes, the sanctuary, and the harbor. The player profile is untouched.

## Still requires human validation

Combat feel, difficulty, bargain balance, story engagement, accessibility, physical controller behavior, and audible playback quality require human testing. Automated runs use `-NoSound`: they verify cue creation, not what a listener hears. Android packaging, touch controls, production art/animation, authored music/voice acting, and a second island are not included in this milestone.
