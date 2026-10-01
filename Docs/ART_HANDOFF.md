# Character and art handoff

The gameplay does not depend on the placeholder meshes. Finish the prototype feedback pass before generating production assets.

## Player character

1. Import a rigged skeletal mesh, materials, and animation assets into `/Game/Art/Characters`.
2. Create a Blueprint derived from `BGCharacter`, for example `BP_Oathbreaker`.
3. Assign **Character Asset** and **Animation Class** in the Art replacement category. Adjust **Character Mesh Offset** and **Character Mesh Rotation** to match the imported skeleton. The gameplay capsule is radius 34, half-height 88; Unreal units are centimeters.
4. The placeholder body, head, blade, and costume parts hide automatically when Character Asset is assigned. Use a weapon socket on the final skeletal mesh if desired.
5. Implement `OnAttackPresentation`, `OnPowerPresentation`, and `OnDamagePresentation` to play montages, particles, or sounds. Melee presentation begins at windup: ordinary damage resolves after 0.08 seconds, the finisher after 0.14 seconds. Dodging cancels pending melee. These are presentation hooks: keep damage and costs in the existing authoritative gameplay methods.
6. Create a Blueprint derived from `BGGameMode` and assign `BP_Oathbreaker` as its Default Pawn Class. Set that GameMode as the project's default or the map's override.

## Enemies

Create Blueprint subclasses of `BGEnemy`. Add skeletal mesh components and hide the inherited placeholder Body and Head components. Implement `OnHitPresentation` for animation and effects. Assign the subclasses to Enemy Class and Guardian Class on the GameMode Blueprint. Keep the attack telegraph visible until a replacement is equally clear.

Disable **Use Prototype Costume** on the replacement enemy Blueprint to omit the procedural armour, cloak, weapons, and legs. The glowing warning ring is a separate ProceduralMeshComponent and should remain visible.

The base class has oathbound, retreating channeler, and shielded sentinel roles. Differentiate silhouettes and shield opening states. Oathstone can interrupt ordinary enemies; the guardian resists interruption. Preserve the pursuit warning's final half-second lock before impact.

Runtime synthesized sounds are temporary combat/UI cues. Replace `BGGameMode::Feedback` with authored cues for the audio pass; four managed audio voices currently prevent uncontrolled component creation. Keep the mute setting and reduced-motion behavior intact.

## Useful first asset batch

- One rigged mortal character, with idle, walk/run, attack, dodge, hit, and death animations.
- One adaptable oathbound enemy skeleton and one guardian.
- Three god-mask or weapon motifs, visually distinct at gameplay camera distance.
- A small modular temple set and ship props after combat readability is proven.

Prefer consistent scale, pivots, silhouette, and material treatment. Inspect generated topology and deformation before importing an entire roster. Keep material counts low and create sensible levels of detail if Android remains a target.

## UI identity

Obsidian blue backgrounds, warm bronze/gold hierarchy, sea-green feedback, vermilion danger. The current UI is native C++ Canvas and scales into a 1440x900 safe area; no web runtime or external fonts are needed. Character art does not require UI rewrites. A later designer-facing UI pass can migrate the same game state/actions to UMG widgets.

## Sanctuary visual pass

`BGIsland.cpp` contains the scene composition; the original walkable floor and combat bounds are preserved. Added architecture, coastal rocks, fire bowls, ship, and inlays have no collision. The Greek key frieze uses instanced geometry. `BGVisuals` builds faceted rocks and open rings without external assets. Four local fire lights cast no shadows; one directional light and a specified ambient cubemap provide the main lighting. Bloom, ambient occlusion, and exposure are restrained; motion blur remains disabled.

`M_SanctuaryStone_v2` provides subtle world-space surface variation with Tint, Glow, Roughness, and Metallic parameters. `M_AegeanWater_v3` is an opaque unlit surface with two animated ripple fields. The procedural stone noise and desktop lighting still need profiling on target Android hardware; this pass does not establish mobile performance.
