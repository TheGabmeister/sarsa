# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Sarsa is a modern 3D PBR multiplayer game engine written in C++ using Vulkan, built as a learning project. It consists of three separate binaries/libraries: **engine** (runtime), **editor**, and **game module** (gameplay DLL). The full technical specification lives in SPEC.md and the implementation roadmap in TASKS.md (19 phases, 97 tasks).

## Build System

- **CMake** is the build system with three targets: engine, editor, game module
- All third-party libraries are **vendored in `vendor/`** (no FetchContent, no git submodules, manual updates only)
- Third-party headers use CMake `SYSTEM` keyword to suppress warnings
- Vulkan SDK must be installed on the host; CMake detects it
- Shaders: GLSL -> SPIR-V via glslc/shaderc with `-M` depfile flag for incremental builds (CMake alone can't track shader dependencies)
- Both Debug and Release configurations must build and pass CI

## Architecture

### Core Design Decisions

- **Fixed-timestep accumulator** (1/60s default): simulation runs at fixed rate, rendering interpolates with alpha. Cap wall-clock dt to ~250ms to prevent spiral of death. Use double for accumulator/clock, float for delta-time.
- **Generational handles (slot map)** for game object references instead of raw pointers. Each GameObject has a stable UUID (for serialization/networking) plus a runtime handle (for fast lookup).
- **Component composition** model with explicit update ordering. Deferred destruction: mark objects for deletion, sweep at frame boundaries.
- **No exceptions**: assertions for programmer errors, error codes/result types for runtime failures.
- **Hot reload**: gameplay code compiles as DLL/SO with a **C-style ABI boundary** (no STL types, no vtables, no allocations crossing boundary). Reload cycle: serialize state -> unload DLL -> copy to temp name -> load new copy -> deserialize.
- **Transform ownership**: dynamic bodies owned by physics engine, kinematic bodies owned by gameplay.
- **Serialization versioning**: version number in every format, prefer additive changes.

### Rendering

- Raw Vulkan initially; thin abstraction layer added later (Phase 17) for D3D12/Metal
- Multi-frame buffering (2-3 frames in flight) with manual barriers and resource transitions
- Render graph / frame graph for pass dependency management (simplified manual passes first in Phase 5, full render graph in Phase 10)
- PBR: GGX/Trowbridge-Reitz NDF, Smith-GGX geometry, Schlick Fresnel, IBL with pre-filtered environment maps and BRDF LUT
- Shadows: cascaded shadow maps (directional), shadow atlasing (point lights), PCF filtering

### Threading

- Main loop (simulation, gameplay, ImGui, render submission) is **single-threaded**
- Task system / thread pool for background work only (asset IO, shader compilation)
- Jolt Physics uses engine's task system via its `JobSystem` interface (avoids thread oversubscription)
- Avoid `std::async` (unbounded thread creation)

### Memory

- System allocator by default; custom allocators only where profiler justifies
- Frame-linear allocators (~16MB budget) for per-frame temporary work
- VMA for GPU memory tracking
- DLL hot reload: gameplay allocations freed on unload; tracking must not report these as leaks

## Key Libraries

| Library | Purpose |
|---------|---------|
| Vulkan + VMA | Graphics API + GPU memory |
| GLFW | Windowing and input |
| GLM | Math |
| spdlog | Structured logging |
| Dear ImGui (docking branch) | Editor GUI |
| Jolt Physics | Rigid body physics |
| Assimp | 3D model import |
| nlohmann/json | Scene serialization, project files |
| ENet | UDP networking (client/server) |
| raudio | Audio playback and mixing |
| stb_image, stb_truetype | Texture import, font rasterization |
| shaderc/glslc | GLSL to SPIR-V compilation |
| SPIRV-Reflect or SPIRV-Cross | Shader reflection |

## Important Constraints

- **Floating-point determinism**: consistent FPU mode (no fast-math), no uninitialized variables, deterministic iteration order. Cross-platform bit-identical results are NOT achievable.
- **Asset pipeline**: source vs. cooked asset separation. Cooked builds must not access source assets. Every load path must work in both editor and packaged contexts.
- **Editor undo/redo** is architecturally invasive: every state mutation must go through a command transaction system. Design it in from the start.
- **Play-in-Editor** requires full world serialization/deserialization for snapshot/restore.
- **Networking scope**: server-authoritative model, target 2-4 players in small arena. Networking is 40-60% of multiplayer engine complexity.
