# AGENTS.md

Guidance for coding agents working in this repository.

## Project Overview

Sarsa is a modern 3D PBR multiplayer game engine written in C++ as a learning project. The intended top-level deliverables are:

- `engine`: runtime
- `editor`: tooling and authoring environment
- `game module`: gameplay code loaded through a stable module boundary

This repository is currently specification-first. The most important context lives in the docs:

- `SPEC.md`: technical architecture, constraints, tradeoffs, and recommended decisions
- `TASKS.md`: phased implementation roadmap
- `README.md`: minimal project label
- `CLAUDE.md`: existing agent-oriented project summary

## Source Of Truth

When the docs disagree, use this order:

1. `SPEC.md`
2. `TASKS.md`
3. `AGENTS.md`
4. `README.md`

Do not invent architecture that conflicts with `SPEC.md`. If a requested change would materially alter a documented design decision, update the docs in the same change or call out the mismatch clearly.

## Current Repository State

At the time this file was written, the repository is mostly planning and documentation. Do not assume an existing engine layout, build tree, or module structure beyond what the docs define. Prefer incremental scaffolding that matches the roadmap instead of speculative large-scale boilerplate.

## Working Style

- Read `SPEC.md` and the relevant section of `TASKS.md` before making non-trivial changes.
- Keep changes aligned with the current phase/task if the user is working from the roadmap.
- Prefer small, verifiable steps over broad framework generation.
- Preserve user changes. Do not revert unrelated work.
- If you introduce a new architectural convention, document it.
- If you add code that depends on a design decision not yet documented, add a short note to the relevant doc.

## Architectural Guardrails

These are already established and should be treated as default constraints unless the user explicitly wants to revisit them:

- CMake is the build system.
- Third-party dependencies are vendored under `vendor/`.
- No FetchContent, no git submodules, no reliance on system-installed libraries beyond required platform SDKs such as Vulkan.
- The engine uses a fixed timestep simulation loop with render interpolation.
- Game object references should use stable IDs plus generational runtime handles, not raw unmanaged pointer ownership as the primary reference model.
- Prefer component composition with explicit lifecycle management and deferred destruction.
- Use assertions for programmer errors and error codes/result-style handling for runtime failures.
- Avoid exceptions as a core error-handling model.
- Gameplay hot reload must cross a stable C-style module boundary.
- Simulation reproducibility on the same platform/build matters; avoid fast-math assumptions and nondeterministic container iteration in critical paths.
- Editor/runtime/cooked-build separation is important from the start, especially for assets and filesystem access.

## Implementation Priorities

When filling in missing code, follow the roadmap order unless the user asks otherwise:

1. Minimal setup and diagnostics
2. Vulkan proof-of-life
3. Core scene/object foundations
4. Materials, textures, and rendering progression
5. Physics, serialization, and editor shell
6. Asset pipeline, gameplay modules, animation, audio, UI, networking, and advanced rendering

Do not jump to advanced systems like networking, ray tracing, or multi-backend abstraction unless the necessary prerequisites are in place or the user explicitly wants exploratory work.

## Build And Dependency Rules

- Prefer modern CMake with clearly separated targets for engine, editor, and game module.
- Keep third-party warnings isolated using CMake `SYSTEM` includes or equivalent suppression boundaries.
- Assume Vulkan SDK discovery happens through CMake.
- For shaders, preserve dependency-aware compilation behavior described in `SPEC.md`.
- Ensure Debug and Release builds remain considered first-class.

## Coding Guidance

- Favor clear, conventional C++ over clever abstractions.
- Keep ownership, lifetime, and threading boundaries explicit.
- Avoid introducing hidden global state.
- Design for debuggability: logging, assertions, validation hooks, and predictable control flow are preferred.
- Keep single-threaded gameplay/update assumptions unless a task explicitly concerns background work.
- For serialization, asset, and networking work, prefer stable identifiers and versioned formats.

## Documentation Expectations

Update docs when changes affect:

- architecture
- project structure
- build steps
- dependency strategy
- task ordering or task scope
- serialization/networking/runtime guarantees

If a code change intentionally diverges from `SPEC.md`, either:

- update `SPEC.md`, or
- leave a clear note explaining why the implementation is temporarily different

## What To Avoid

- Generating large amounts of placeholder code with no path to verification
- Adding systems that contradict the documented phased plan
- Mixing incompatible ownership models
- Building editor-only assumptions into runtime code
- Accessing source assets from packaged/cooked-only paths
- Treating networking as a bolt-on feature disconnected from the object model and simulation design

## Good First Read For Any Agent

Before substantial work, review:

1. `SPEC.md`
2. `TASKS.md`
3. `CLAUDE.md`

Then inspect the current repository contents and work from what actually exists.
