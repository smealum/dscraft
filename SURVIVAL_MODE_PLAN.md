# Lightweight Minecraft 1.1-inspired DS Edition

This fork targets a compact, Minecraft Java Edition 1.1-inspired survival experience for Nintendo DS/DSi hardware. Keep features only when they fit the existing renderer, save format, and memory budget.

## Scope and performance rules

- Prefer small, static features over large simulation systems.
- Keep the starting active mob count low; current startup spawns four.
- Defer high-cost world generation, large underground structures, complex villager AI/trading, and heavy pathfinding unless profiling proves they fit.
- Zoom is implemented with SELECT + UP/DOWN by changing the camera field of view.
- New block types require verified texture-atlas entries and compatible save/render handling. Do not assign IDs to placeholder texture entries and call them finished blocks.
- Build and test with devkitPro/devkitARM and an emulator or real DSi before calling any feature complete.

## Practical priorities

1. Preserve reliable movement, block placement, saving, and loading.
2. Prefer low-cost additions that reuse existing assets and renderer paths.
3. Add small, static structures only if the world-generation lifecycle supports them safely.
4. Add new blocks only alongside actual atlas textures and verified block/item IDs.
5. Defer hunger/sprint systems, large biome/structure generation, trading, and other costly mechanics until a performance baseline is measured.


## Feature roadmap

### 1. Survival basics
- Health and hunger state, initialized when a world/player starts.
- Hunger drains slowly with activity; sprinting costs more.
- At empty hunger, disable sprinting and apply a conservative survival penalty.
- Rate-limited healing based on hunger.
- Add health/hunger indicators to the existing HUD without per-frame allocations.

### 2. Movement and controls
- Add a sprint action that increases horizontal movement only.
- Preserve all existing control schemes, collision, gravity, and current water/ladder behavior.
- Sprinting should drain hunger and be unavailable when hunger is depleted.

### 3. Mobs
- Improve existing `SURVIVAL`-guarded mob logic in small steps: stable wandering, player awareness, then contact damage.
- Reuse existing collision/rendering and avoid expensive pathfinding.
- Keep entity counts low and cap active mobs to protect frame rate and memory.
- Pillagers, Blazes, and Wither Skeletons are later mob types; do not reuse the creeper model/texture as if it were their final art.

### 4. Biomes
Build a small set of visually distinct, deterministic biomes using the existing world-generation and block systems:
- **Plains** — open grassy terrain, scattered trees, village-friendly.
- **Forest** — denser trees and green ground.
- **Desert** — sand, sparse vegetation, desert temples.
- **Taiga** — conifer-like tree patterns and cooler palette where supported.
- **Swamp** — low, wet terrain and sparse trees if generation budget permits.
- **Snow** — snow-covered surfaces and sparse conifers, subject to existing block/texture support.

Generation should use seeded, low-cost rules and smooth transitions between regions. Biomes must not require large extra lookup tables or expensive per-block calculations.

### 5. Structures and world landmarks
Implement structures as small, deterministic block templates, not as a large procedural simulation:
- **Desert temple** — compact sandstone pyramid, hidden treasure room, pressure plate/TNT trap only if existing block and redstone support can safely represent it. Otherwise use a visual trap room without functional TNT.
- **Village** — small set of houses, paths, a well and tiny farms; later add biome-specific house palettes and more varied street layouts.
- **Pillager outpost** — wooden watchtower, small camp and loot chest; pillager AI is a separate task.
- **Stronghold** — underground stone-brick corridors and rooms, library and portal-room landmark; Eye-of-Ender search mechanics are a later feature.
- **Nether fortress** — Nether-brick bridge/corridor templates, with fortress generation gated on a real Nether dimension or a clearly marked prototype area.
- Also allow small huts, wells, ruins and towers from the same template system.

**Placement requirements:** verify all template coordinates against map bounds, inspect terrain height and flatness, reject collisions/overlap, and never overwrite bedrock or the player. Use a stable world/region seed so reopening or streaming an area does not duplicate structures. Limit structure count and template size for DS memory and frame-time budgets.

### 6. Villages: classic baseline plus expanded style
- Generate small villages in suitable flat/plains-like areas first.
- Build several house layouts around a path or central well, with farms where supported by the block set.
- Add a lightweight village layout/anchor so nearby chunks do not generate duplicate or overlapping villages.
- Expand later with more varied house shapes, street layouts, lighting, farms, and decoration to evoke newer Minecraft villages.
- Villagers, professions, schedules, trading, and modern biome-specific village sets are stretch goals only after static villages and performance are reliable.

### 7. HUD and feedback
- Health and hunger indicators sized for the DS screen.
- Optional small biome name notification on entering a new region, if it fits the interface.
- Avoid adding high-cost texture or font systems solely for these features.

### 8. Build and testing
- Build with the project's devkitPro/devkitARM setup.
- Test all control schemes, water/ladder movement, world streaming, seed reproducibility, biome boundaries, and structure placement.
- Test on emulator and DSi hardware where available.
- Do not claim a playable `.nds` build until it compiles and has been smoke-tested with the required Nintendo DS toolchain.

## Technical findings and implementation order

- The current startup path in `source/game/game_main.c` calls `loadTestMap(&map)`; `source/game/map.c` labels `generateTestMap` as invalid/obsolete. This means structures cannot responsibly be added by simply sprinkling blocks into a supposed live procedural generator: a reliable world-generation/structure-generation entry point must be designed first.
- `changeBlock` updates existing block/render state but also has player-collision and cluster assumptions. Bulk structure placement should be a dedicated, bounds-checked generation operation before final display-list generation, rather than thousands of runtime edits while the player is nearby.
- The existing map is vertically limited to 16 blocks in the current load path. Full strongholds and Nether fortresses need a deliberate vertical/world-layout decision; they should not be forced into the current map and silently clipped.
- Existing mobs are guarded by `#ifdef SURVIVAL`; preserve compatibility with non-survival builds.
- Prefer existing blocks and textures first; verify block IDs and texture availability before assigning sandstone, stone bricks, Nether bricks, chests, TNT, or portal blocks.
- Keep generation deterministic and bounded. Avoid dynamic allocations per block and avoid expensive pathfinding.

Recommended order: (1) confirm block IDs and map-generation lifecycle, (2) add deterministic structure-template primitives with bounds/overlap checks, (3) prototype a small desert temple and village, (4) extend terrain/biomes, (5) add outpost and stronghold templates, (6) implement a Nether dimension before fortress gameplay, then (7) add survival HUD/mechanics and specialized mobs.

## Current status

This is still planning and repository analysis, not a playable implementation. No structures have been added to generated worlds yet, and no `.nds` build has been produced or tested. The repository's legacy map loading/generation architecture must be addressed before these structures can appear reliably. Java 1.0-inspired survival is the baseline; selected later structures and expanded villages are constrained by DSi hardware.


## RAM rendering window and SD-backed world storage

The intended storage model for a large world is **streamed world data on SD, active render window in RAM**. Do not allocate a full 1024x1024x16 block array.

### Existing engine support

- `map_struct` stores a fixed `superCluster[32][32]` working window plus four 32-column transition strips.
- `loadTestMap()` reads the current window and transition columns through the selected `readClusterColumn` backend. The map header and file handle are opened through the existing filesystem layer.
- The renderer addresses world positions relative to `map.offset`; movement code shifts the window and reads new edge columns as the player approaches its boundary.
- The existing cluster-column readers include 2048-, 1024-, 512-byte and No$GBA variants. The file format/backend must match the world being opened; do not assume all formats are interchangeable.

### Required behavior for a 1024x1024 world

1. Keep only the current 32x32 cluster-column window and transition strips resident; load edge columns from SD as the player crosses a window boundary.
2. Save changed columns before reusing their memory, then read the incoming column. Never discard a changed column silently.
3. Treat SD reads/writes as slow operations: stream at boundary crossings, not per rendered block or every frame. If possible, show a short loading pause rather than blocking repeatedly during movement.
4. Use deterministic world generation keyed by world seed and column coordinates so a missing/uninitialized column can be generated once, persisted, and reproduced consistently. Do not regenerate a modified column on each visit.
5. Bound all world coordinates to the map header dimensions and validate the selected storage format before reading.
6. Profile the existing 32x32 window on real hardware. It represents roughly 128x128 horizontal blocks when each cluster column covers four blocks; this is a RAM window, not the entire world.

### Current limitation

The streaming-window structure already exists in the legacy engine, but this does **not** by itself create a generated 1024x1024 world. The current startup path opens an existing map via `loadTestMap()`, and the vertical dimension is still hard-coded to 16 in its `initMap()` call. A 1024x1024 world requires a compatible map file/header, tested edge-window updates and persistence, plus a generation or conversion path. No ROM build or runtime test has been performed for this design.


## Requested wood sets, Mushroom Island, and custom textures

This section records the user's requested feature scope. It is a target, not a claim that these features are already implemented.

### Five complete wood families
- Oak, spruce, birch, jungle, and acacia.
- Each family should have logs and planks, plus matching fences, slabs, stairs, and chests.
- Fences need their own connected/isolated shape behavior; slabs need half-height geometry and correct collision; stairs need step geometry and collision. These are not complete if implemented as ordinary full cubes.
- Chests need a placeable block, a stable saved state, and an inventory/container interface only if the memory and UI budget permits it.
- Reserve unique block/item IDs and texture tiles only after checking existing IDs, save compatibility, inventory capacity, and atlas dimensions. Do not reuse a texture coordinate for two different wood types as a final implementation.

### Mushroom Fields / Mushroom Island
- Add a deterministic Mushroom Fields biome with mycelium ground, red and brown mushrooms, giant mushroom cap/stem blocks, and mushroom-covered terrain features.
- Generate island-like terrain surrounded by ocean only after a seeded world-generation entry point exists; the current startup opens an existing map, so this cannot appear from documentation or block IDs alone.
- Add mooshrooms as a later mob variant if the existing mob model/texture and entity limits can support it.
- Use bounds checks and deterministic seed/region rules so streamed columns do not create duplicate mushrooms or mismatched biome edges.

### User-editable texture workflow
- Provide a numbered texture-atlas template that matches the engine's actual atlas dimensions, tile coordinates, color depth, and conversion pipeline.
- Include a tile map for existing blocks and reserved entries for the requested wood families and Mushroom Fields blocks.
- Keep source artwork editable (for example, PNG template plus a tile-index legend); convert it to the game's required runtime format during asset preparation/build.
- Do not claim arbitrary PNG files load directly unless the texture loader actually supports them.
- Use original or appropriately licensed artwork; the user should be able to replace the artwork without editing C source when IDs/layout remain unchanged.

### ROM build status
- The repository has a GitHub Actions workflow intended to compile an `.nds` and upload it as the `dscraft-nds` artifact.
- A successful CI compile would only verify that the current source builds; it would not mean the whole roadmap above is implemented or tested on a real DSi.
