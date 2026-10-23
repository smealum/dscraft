# Structure generation implementation specification

This document is the implementation contract for the requested structures on the `survival-features` branch. It intentionally does not claim that these structures are already generated in-game.

## Why the generation hook comes first

The current game startup calls `loadTestMap(&map)`. The old `generateTestMap` function in `source/game/map.c` is marked obsolete/invalid. The map uses cluster-local storage and a render/quad cache, so writing raw block bytes at an arbitrary time can leave geometry and lighting inconsistent. The current loading path also initializes a 16-block vertical map. A structure generator must be integrated into a real map-generation/save lifecycle before templates can safely become gameplay features.

## Proposed API shape

Keep the structure layer small and separate from mob AI:

- `structures.h`: structure type enum and public generation entry point.
- `structures.c`: deterministic selection, terrain checks, compact templates and block palette mapping.
- A future world-generation hook calls the structure layer once for each region before final geometry/light generation.
- Structure placement returns a result (placed, unsuitable terrain, out of bounds, overlap) so generation can fail safely rather than partially writing a template.
- The caller supplies a stable world seed and region coordinates. Do not use unseeded global `rand()` for placement decisions.
- Check the full footprint and vertical extent before writing any block. Validate all required block IDs against the actual block/texture tables first.
- Avoid heap allocation for individual blocks. Template data should be static and small.

## Template scope

### Desert temple
Start with a small sandstone pyramid silhouette and a central chamber. Add a hidden loot chamber only after chest/container support is confirmed. A pressure plate, TNT, and redstone trap must not be simulated by arbitrary block IDs; if those mechanics do not exist, use a non-functional visual trap or defer it.

### Village
Start with 2–4 compact houses and a path around a well on flat terrain. Add farms only if farmland/crop blocks are supported. Keep the village anchor deterministic and large enough to prevent duplicates from adjacent regions. Static buildings come before villagers, schedules, professions or trading.

### Pillager outpost
Start with a compact wooden tower and small camp. Pillager mobs and ranged combat are a separate dependency; do not spawn the existing creeper-textured mob and label it a pillager.

### Stronghold
Use small underground room/corridor templates, but first increase or redesign vertical map support. Portal-room geometry is only a landmark until End/portal mechanics exist. Do not promise Eye-of-Ender location behavior without a corresponding item/search system.

### Nether fortress
Create bridge and corridor templates only after a Nether dimension or an explicitly scoped Nether prototype exists. Nether-specific mobs and Blaze Rod progression are separate dependencies. A fortress placed in the overworld should not be presented as a complete Nether feature.

## Acceptance checklist

- Same seed and region produce the same candidate structure and layout.
- Re-loading or streaming a region does not duplicate the structure.
- No placement occurs outside map bounds or in unsuitable terrain.
- No structure overwrites bedrock, the spawn/player collision volume, or an existing structure.
- Failed placement leaves the map unchanged.
- Geometry and lighting are rebuilt using the engine's supported pipeline.
- Memory and frame time remain within the DS budget.
- Both normal and `SURVIVAL` builds compile.
- The resulting ROM is tested in an emulator; hardware testing is recorded separately.

## Status

Not implemented. This is a design specification to guide the first safe code change. The next code task is to identify and formalize the world-generation hook and block palette, then wire one tiny test template end-to-end before adding the full requested set.
