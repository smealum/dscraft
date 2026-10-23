# Survival mode: Java Edition 1.0-inspired

This fork is based on the original DScraft codebase, so the goal is a lightweight survival layer that fits the Nintendo DS/DSi hardware rather than a full PC feature port.

## Feature plan

1. **Survival state** — health and hunger values, initialized when a world/player is started. Keep the state small and use integer/fixed-point-friendly logic.
2. **Hunger loop** — drain hunger slowly while moving; sprinting drains it faster. At zero hunger, stop sprinting and apply a conservative survival penalty. Health regeneration should depend on hunger and be rate-limited.
3. **Sprint controls** — add a hold-to-sprint input that multiplies horizontal movement only. Preserve existing control schemes and avoid changing vertical velocity, collision, or water/ladder movement.
4. **Mobs** — extend the existing `SURVIVAL`-guarded mob code in small steps: stable wandering first, then player awareness and contact damage. Reuse existing collision and rendering rather than adding expensive pathfinding.
5. **HUD** — display health/hunger in the existing interface using low-cost DS-friendly icons or text. Avoid per-frame allocations.
6. **Build and test** — build with the project's devkitPro/devkitARM setup; test all three control schemes, water/ladder movement, world streaming, and performance on emulator and DSi hardware where available.

## Implementation notes

- The current player update already owns collision, gravity, water, and ladder state; sprint should affect horizontal input before collision resolution.
- The current mob implementation is guarded by `#ifdef SURVIVAL` and has basic wandering/physics. Keep new mob logic behind that guard so non-survival builds remain viable.
- The HUD lives in the existing interface code. Any new indicators should fit its current screen layout.
- Do not claim a playable `.nds` build until it compiles and has been smoke-tested with the required Nintendo DS toolchain.

## Current status

This document is a roadmap only. It does not claim that hunger, sprinting, combat, or new mob AI are implemented yet.