# Survival, biomes, villages, and structures roadmap

This fork is based on the original DScraft codebase. The goal is a lightweight, Java Edition 1.0-inspired survival game with selected expanded/modern village and world-generation ideas, designed for Nintendo DS/DSi hardware rather than a full PC feature port.

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
