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

### 4. Biomes
Build a small set of visually distinct, deterministic biomes using the existing world-generation and block systems:
- **Plains** — open grassy terrain, scattered trees, village-friendly.
- **Forest** — denser trees and green ground.
- **Desert** — sand, sparse vegetation, potential desert structures.
- **Taiga** — conifer-like tree patterns and cooler palette where supported.
- **Swamp** — low, wet terrain and sparse trees if generation budget permits.
- **Snow** — snow-covered surfaces and sparse conifers, subject to existing block/texture support.

Generation should use seeded, low-cost rules and smooth transitions between regions. Biomes must not require large extra lookup tables or expensive per-block calculations.

### 5. Structures and world landmarks
- Small houses, huts, wells, ruins, and simple towers assembled from existing blocks.
- Structure placement should check terrain height, world bounds, and overlap before writing blocks.
- Use deterministic placement based on the world seed so structures are reproducible.
- Keep templates compact and limit how many are generated per region.

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

## Implementation notes
- The player update already owns collision, gravity, water, and ladder state; sprint should affect horizontal input before collision resolution.
- Existing mobs are guarded by `#ifdef SURVIVAL`; preserve compatibility with non-survival builds.
- The existing map/world generation must be extended rather than replaced wholesale. Keep generation deterministic and bounded to fit handheld memory and frame-time budgets.
- Prefer existing blocks and textures first; new assets can be added only when the asset pipeline and memory budget are understood.

## Current status
This document is a roadmap only. It does not claim that hunger, sprinting, new mob AI, biomes, structures, or villages are implemented yet. Java 1.0-inspired survival is the baseline; modern-style village variety is an expanded goal, constrained by DSi hardware.
