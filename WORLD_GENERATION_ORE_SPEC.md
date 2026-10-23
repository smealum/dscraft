# World depth, sea level, caves, and ore generation

This is the implementation target for the `survival-features` branch. It is a specification, not a claim that the generator is already active in-game.

## Vertical dimensions
- Set sea level to world Y = 60.
- Provide 60 blocks of terrain above sea level (nominal surface ceiling Y = 120).
- Provide 60 blocks below sea level for caves and underground resources (bedrock/deepest layer near Y = 0).
- Treat the playable vertical span as approximately Y = 0 through Y = 120, with water filled to sea level where terrain generation calls for oceans/lakes.
- The current map loader initializes a vertical dimension of 16; update map header/initialization and cluster allocation together rather than changing only the constant. Validate memory use on Nintendo DS hardware.

## Cave generation
- Generate caves deterministically from world seed and chunk/region coordinates.
- Use a low-memory tunnel/worm or connected-carving approach; never allocate a whole 1024 x 1024 x 120 block world in RAM.
- Preserve the surface and avoid carving through the bedrock floor.
- Cave carving must run before final geometry, lighting, and cluster mesh construction.
- Ensure carved blocks and generated ores are persisted with the affected cluster/column when streamed out to SD.

## Ore generation
Generate ore veins after cave carving and before final mesh/light generation. Use deterministic pseudorandom values derived from the world seed and chunk coordinates, so regenerating a streamed region reproduces the same ore placement.

Initial ore targets (Minecraft-inspired, tuned conservatively for DS performance):
- Coal: common, broad underground distribution.
- Iron: common-ish, concentrated underground.
- Gold: uncommon, deeper underground.
- Redstone: uncommon, deepest underground.
- Diamond: rare, deepest underground.
- Optional lapis lazuli: uncommon, mid-to-deep underground.

Rules:
- Replace stone only; never overwrite air, water, bedrock, logs, or player-placed blocks.
- Keep veins compact (small clusters) and cap attempts per chunk to control generation time.
- Use explicit depth bands relative to sea level Y=60; keep diamonds/redstone deepest, gold below mid-depth, iron/coal more broadly distributed.
- Validate each ore's block ID and texture mapping before enabling it. If a needed ore block ID does not exist, add it to the block/texture tables and inventory/loot systems first.
- Cave exposure is allowed: ores may be visible on cave walls, but avoid generating ore in open air.
- Do not run generation again on saved, player-modified chunks. Store a generated/modified flag or persist the generated result so ore veins do not reroll.

## Nintendo DS constraints and acceptance checks
- Generate one region at a time, with fixed upper bounds on attempts and scratch memory.
- No per-block heap allocations.
- Confirm a newly generated region has the requested vertical span, sea level, caves, and ore veins.
- Confirm adjacent regions are seam-consistent and that save/reload preserves ore and player edits.
- Build and test the ROM before claiming this feature is complete.
