# Sarsa Game Engine

## What
- a modern 3D PBR game engine, object-oriented, with multiplayer
- libraries: glfw, glm, spdlog, enet, Vulkan (+ VMA), assimp, nlohmann/json, Dear ImGui, Jolt Physics, raudio, stb_image, stb_truetype

## Why
- for learning

## Architecture

### Build & Architecture
- CMake build system
- Runtime, editor, and game modules as separate binaries/libraries
- Object-oriented architecture with game objects, component composition, and inheritance
- C++ gameplay modules with desktop hot reload in editor/dev builds via a stable module boundary (avoid raw C++ ABI coupling)
- Fixed timestep runtime with clear simulation/render separation
- Reproducible simulation on the same platform/build for debugging and networking validation
- Memory tracking, allocator hooks, and targeted frame/linear allocators where profiling justifies them
- Task system / thread pool for background asset IO, compilation, and non-gameplay parallel work

### Platform & Diagnostics
- Raw Vulkan initially; thin abstraction layer added later to support D3D12 and Metal
- VMA (Vulkan Memory Allocator) for GPU memory management
- GLFW for windowing and input
- GLM for math
- spdlog for structured logging
- Validation layers, assertions, crash handling, profiling hooks, and GPU markers
- Automated testing (unit tests, serialization round-trip tests, render comparison tests)

### Rendering
- GLSL shaders compiled to SPIR-V via glslc/shaderc (cross-compilation to other backends deferred to abstraction layer phase)
- Renderer resource system (buffers, textures, descriptors, transient frame allocators)
- Render graph / frame graph for pass dependency management
- Material and shader system with reflection/metadata
- Scene rendering fundamentals (frustum culling, draw sorting, instancing, depth pre-pass where beneficial)
- Basic forward lighting before full PBR (directional + point lights)
- Physically based rendering (PBR) pipeline
- Skybox and environment rendering
- LOD support and occlusion culling
- GPU particle / VFX system
- Hardware ray tracing (Vulkan RT initially) with capability checks, fallbacks, and denoising
- GPU-driven rendering direction decided before performance optimization phase

### Asset Pipeline
- 3D model import supporting common formats (Assimp)
- Texture import (stb_image for source import, platform-appropriate GPU-compressed cooked output)
- Scene serialization and project files (nlohmann/json)
- Binary cooked format for meshes/textures/shaders; JSON reserved for project and metadata
- Asset database, stable asset IDs, import settings, and dependency tracking
- Source-versus-cooked asset separation with importer versioning
- Async asset loading (background thread, non-blocking where practical)
- Asset reimport and hot reload support
- Virtual filesystem / path abstraction for engine, editor, and packaged builds

### Editor & UI
- Editor GUI with dockable panels (Dear ImGui - docking branch)
- Viewport, hierarchy, inspector, and content browser workflows
- Gizmos, selection/picking, undo/redo, and layout persistence
- Play-in-Editor (PIE) mode with explicit world snapshot/restore rules
- Font rasterization and in-engine text (stb_truetype)

### Runtime UI
- Gameplay UI system separate from the editor (menus, HUD, widgets)
- Explicit input focus ownership between gameplay, runtime UI, and editor UI

### Physics
- Rigid body simulation and collision detection (Jolt Physics)
- Collision layers/masks, raycasts, overlap queries, sweeps, and debug visualization
- Fixed timestep simulation with optional sub-stepping where required
- Defined transform ownership/sync rules between gameplay objects and physics

### Animation
- Static mesh import first, then skeletal animation, skinning, and animation playback
- Animation state machine and blend tree support
- Animation events/notifies for gameplay hooks

### Audio
- Sound playback and mixing (raudio)
- 2D/3D playback, buses/submixes, streaming music, and listener management

### Networking
- Multiplayer over reliable/unreliable UDP transport (ENet initially)
- Authority, replication, and prediction model defined before full multiplayer implementation
- Network compatibility informed by simulation design, but not dependent on cross-platform bit-identical determinism

### Packaging & Shipping
- Cooked build pipeline (asset archives, executable bundling)
- Platform packaging, save-data paths, and release configuration separation

---

# Technical Specification

Edge cases, tradeoffs, and hard problems for each major system.

---

## Table of Contents

1. [Build System & Project Structure](#1-build-system--project-structure)
2. [Game Loop & Timestep](#2-game-loop--timestep)
3. [Game Object Model](#3-game-object-model)
4. [Transform Hierarchy](#4-transform-hierarchy)
5. [Rendering Foundation (Vulkan)](#5-rendering-foundation-vulkan)
6. [Shader Pipeline](#6-shader-pipeline)
7. [Render Graph](#7-render-graph)
8. [Materials & PBR](#8-materials--pbr)
9. [Shadows](#9-shadows)
10. [Ray Tracing](#10-ray-tracing)
11. [GPU-Driven Rendering](#11-gpu-driven-rendering)
12. [GPU Particles & VFX](#12-gpu-particles--vfx)
13. [Physics (Jolt)](#13-physics-jolt)
14. [Animation](#14-animation)
15. [Audio](#15-audio)
16. [Asset Pipeline](#16-asset-pipeline)
17. [Gameplay Modules & Hot Reload](#17-gameplay-modules--hot-reload)
18. [Editor](#18-editor)
19. [Runtime UI](#19-runtime-ui)
20. [Networking](#20-networking)
21. [Memory & Allocation](#21-memory--allocation)
22. [Threading & Task System](#22-threading--task-system)
23. [Platform & Windowing](#23-platform--windowing)
24. [Diagnostics & Profiling](#24-diagnostics--profiling)
25. [Packaging & Shipping](#25-packaging--shipping)
26. [Cross-Cutting Concerns](#26-cross-cutting-concerns)

---

## 1. Build System & Project Structure

### Hard Parts

- **Vulkan SDK dependency.** The Vulkan SDK must be installed on the build machine. CMake's `find_package(Vulkan)` handles detection, but CI machines need the SDK installed or the Vulkan headers/loader vendored. VMA is a single-header library — vendor it directly.
- **Compiler warning policy across platforms.** MSVC, Clang, and GCC have different warning sets. `-Wall -Wextra` on GCC/Clang does not map 1:1 to `/W4` on MSVC. Third-party headers will produce warnings under strict policies — you need to suppress warnings for external includes (CMake `SYSTEM` keyword or pragma push/pop).
- **Debug vs. Release configuration drift.** It is easy for Debug and Release builds to diverge silently. Debug builds with assertions enabled and Release builds with optimizations can behave differently (uninitialized variables, UB that optimizers exploit). CI must build and test both configurations.

### Edge Cases

- Incremental builds with shader compilation: CMake has no native shader dependency tracking. You need custom commands that track `#include` dependencies in GLSL files (via glslc's `-M` depfile flag), or you get stale SPIR-V after header changes.
- Module target output directories must be predictable for hot reload to find the DLL. If CMake puts them in `Debug/` vs `Release/` subdirectories (MSVC multi-config generators do this), hot reload breaks unless you account for it.

### Library Setup

All third-party libraries are vendored in a `vendor/` directory and integrated via CMake `add_subdirectory`. No FetchContent, no submodules, no system-installed dependencies. Each library is downloaded manually and committed to the repo. Updates are manual — download the new version, replace the folder, verify it builds.

---

## 2. Game Loop & Timestep

### Implementation Detail

The accumulator model:

```
accumulator = 0
while running:
    dt = clock.elapsed()           // wall-clock delta, capped
    accumulator += dt

    while accumulator >= FIXED_DT:
        simulate(FIXED_DT)         // physics, gameplay logic
        accumulator -= FIXED_DT

    alpha = accumulator / FIXED_DT
    render(alpha)                  // interpolated state for smooth display
```

`FIXED_DT` is a global engine constant — changing it mid-run invalidates physics tuning, animation timing, and network assumptions. The renderer interpolates between the previous and current simulation state using `alpha`, meaning the render is always at most one tick behind.

### Hard Parts

- **The spiral of death.** If simulation takes longer than `FIXED_DT`, the accumulator grows faster than it drains. Uncapped, this causes the engine to simulate dozens of frames in a single update, freezing the renderer. You must cap the number of simulation steps per frame (e.g., max 4). This means the simulation falls behind real time — gameplay slows down. There is no good answer: you either slow down the game, skip simulation steps (breaks physics determinism), or accept frame drops.
- **Interpolation requires storing two states.** Every rendered object needs both its current and previous transform. This doubles the transform storage or requires a parallel "render state" buffer. If any system writes transforms outside the simulation step (e.g., animation, particles), it must also update the interpolation source, or objects will jitter.
- **Accumulator precision.** Over hours of runtime, floating-point accumulator drift can cause timing anomalies. Use `double` for the accumulator and the running clock. Use `float` only when passing delta time into simulation functions.
- **Variable monitor refresh rates.** With VSync on a 144Hz monitor, the render loop runs at 144fps but simulation at 60Hz. With VSync on a 30fps display, multiple simulation steps execute per render frame. Both cases work with the accumulator model, but both have distinct feel — test on a range of refresh rates.

### Edge Cases

- Alt-tabbing or debugger breakpoints cause a massive `dt` spike. Cap `dt` to something reasonable (e.g., 250ms). Without this, the accumulator overflows and the spiral of death triggers immediately.
- If the simulation and render run on separate threads (not planned, but a future possibility), the interpolation alpha must be communicated thread-safely, and the previous/current state buffers become a double-buffer synchronization problem.

### Tradeoffs

| Fixed DT | Pro | Con |
|----------|-----|-----|
| 1/60 (16.67ms) | Common, well-tested | Coarse for fast physics (small objects tunnel) |
| 1/120 (8.33ms) | Better collision for fast objects | Double the simulation cost, network tick rate implications |
| 1/60 with sub-stepping | Best of both | Complexity, physics sub-steps are not free |

---

## 3. Game Object Model

### Implementation Detail

Object identity: each GameObject has a stable UUID (for serialization, networking, editor references) and a runtime handle/index (for fast lookup).

### Hard Parts

- **Object lifecycle and mid-frame destruction.** If gameplay code calls `Destroy(obj)` during a simulation tick, you cannot delete the object immediately — other systems may still hold references to it (physics callbacks, render lists, pending events). You need deferred destruction: mark objects for deletion, then sweep them at a defined point in the frame (end of simulation tick, before render). This means `IsValid()` / `IsPendingDestroy()` checks must exist, and all code that holds object pointers must be aware that the object may be dead.
- **Dangling pointers.** Raw `GameObject*` pointers become dangling after deferred destruction. Options:
  - **Weak handles:** An index + generation counter into a slot map. When the generation doesn't match, the handle is stale. This is the standard solution. Cost: one indirection per access.
  - **`std::shared_ptr`:** Reference counting. Cost: atomic refcount overhead, shared ownership semantics make lifecycle reasoning harder, circular references possible.
  - **Raw pointers with discipline:** Fastest, but a single missed invalidation is a crash or memory corruption.

  **Recommendation:** Generational handles (slot map). They are cheap, safe, and make serialization natural (the handle is just two integers).

- **Component ordering.** If ComponentA depends on ComponentB (e.g., a CharacterController that reads from RigidBody), the update order matters. Options: explicit dependency declaration with topological sort, fixed phase ordering (all physics components update before all gameplay components), or manual ordering by component type priority.

- **Inheritance vs. composition tension.** An "object-oriented with component composition" model means you have both base class virtual methods AND component-based behavior. This creates ambiguity: does a `PlayerCharacter : GameObject` override `Update()` directly, or does it delegate to a `PlayerControllerComponent`? You need a clear rule: either GameObjects have no virtual behavior methods (all logic lives in components), or there is a defined call order between base class methods and component updates.

### Edge Cases

- Creating objects inside `Destroy` callbacks (e.g., "when this enemy dies, spawn an explosion"). The newly created object must not be swept by the same destruction pass.
- Reparenting an object during iteration over the scene hierarchy. Use deferred reparenting or iteration-safe containers.
- Serializing an object graph with circular references (Object A's component references Object B, which references Object A). The serializer must handle reference cycles without infinite recursion — typically by serializing references as IDs and resolving them in a second pass.

### Tradeoffs

| Model | Pro | Con |
|-------|-----|-----|
| Pure ECS (data-oriented) | Cache-friendly, parallelizable | Harder to learn, poor fit for OOP goal, complex queries |
| GameObject + Components (Unity-style) | Intuitive, well-understood | Pointer chasing, harder to parallelize, virtual call overhead |
| Actor model (Unreal-style) | Rich inheritance hierarchy | Complex, deep vtables, hard to serialize |

Do not try to hybrid it with a data-oriented ECS underneath; that creates two systems to maintain.

---

## 4. Transform Hierarchy

### Hard Parts

- **Dirty flag propagation cost.** Local transforms are relative to parent; world transforms are computed by concatenating up the chain. When a local transform changes, mark this node and all descendants dirty. Moving a root with 1000 descendants marks 1000 flags. Eager batch update (one pass over dirty nodes) is simpler than lazy recomputation (risks redundant work when multiple children accessed per frame).
- **Scale in hierarchies.** Non-uniform scale (different X, Y, Z scale) combined with rotation produces shear in child transforms. Most engines either:
  - Disallow non-uniform scale in the hierarchy (simplest, most predictable).
  - Allow it but document that physics shapes will not match visual appearance (Jolt uses unscaled shapes).
  - Decompose the world matrix back into TRS and accept the lossy decomposition.

  **Recommendation:** Support non-uniform scale on leaf nodes only. Warn in the editor when non-uniform scale is applied to a node with children.

- **Physics transform sync.** Physics bodies in a hierarchy complicate transform ownership (see [Section 13](#13-physics-jolt) for the full ownership model). The specific transform concern: if a physics body is a child of another object, its world transform from physics must be decomposed into a local transform relative to its parent. This decomposition is expensive and lossy if the parent has non-uniform scale.

  **Rule:** Physics bodies should be root-level or children of static/non-scaled parents. Enforce this at author time in the editor.

- **Rotation representation.** Store rotations as quaternions internally. Expose Euler angles in the editor for user-friendliness, but convert to/from quaternions at the boundary. Euler angles have gimbal lock and order-of-rotation ambiguity (XYZ vs. ZYX). Pick one convention (e.g., intrinsic XYZ) and document it. Never store Euler angles as the source of truth.

### Edge Cases

- Cycle detection: reparenting A under B when B is already a descendant of A creates a cycle. The reparenting code must check for this.
- Moving an object between parents while a physics simulation step is in flight. The physics body's position is now relative to a different parent. Defer reparenting until after physics sync.
- Precision loss far from the origin. At world positions like (100000, 0, 100000), 32-bit float loses sub-millimeter precision. This causes jittering in rendering and physics. Solutions: camera-relative rendering (subtract camera position before building the view matrix), origin rebasing (shift the whole world periodically), or 64-bit world positions with 32-bit local offsets. For a learning engine, camera-relative rendering is the minimal fix.

---

## 5. Rendering Foundation (Vulkan)

### Hard Parts

- **Vulkan's verbosity.** Vulkan requires explicit management of everything. Instance creation, physical/logical device selection, queue families, command pools, command buffers, synchronization primitives, memory allocation, descriptor sets, pipeline layouts, render passes, framebuffers — all must be configured manually. Expect ~1500 lines before a triangle appears. Every frame, you must:
  1. Acquire a swap chain image (handle `VK_SUBOPTIMAL_KHR` and `VK_ERROR_OUT_OF_DATE_KHR`).
  2. Wait on the fence for the frame-in-flight that previously used this image.
  3. Reset and begin recording command buffers.
  4. Insert pipeline barriers to transition resources to correct states.
  5. Begin render pass, bind pipeline/descriptors/vertex buffers, issue draw calls, end render pass.
  6. Transition the swap chain image for presentation.
  7. Submit the command buffer, signal a fence and a semaphore.
  8. Present (passing the semaphore for the GPU to wait on).

  Missing any synchronization step causes GPU crashes, device lost errors, or visual corruption — validation layers catch some of these, but not all.

- **Pipeline barriers and resource transitions.** Vulkan requires explicit `vkCmdPipelineBarrier` calls with correct stage masks, access masks, and image layout transitions. Getting this wrong produces validation errors at best and silent corruption at worst. The render graph (Phase 10) should eventually automate this, but you'll be managing it manually for the first 5+ phases. Common mistakes: wrong `srcStageMask`/`dstStageMask`, forgetting the `oldLayout`→`newLayout` transition on first use, and missing barriers between compute and graphics passes.

- **Swap chain resize.** When the window resizes, `vkAcquireNextImageKHR` returns `VK_ERROR_OUT_OF_DATE_KHR`. You must destroy and recreate the swap chain, image views, depth buffer, and framebuffers. This must happen while no GPU work is in flight (`vkDeviceWaitIdle` or fence-based drain). Minimizing the window produces a 0x0 extent — detect and skip rendering entirely.

- **Multi-frame buffering.** Buffer 2-3 frames in flight. Each frame needs its own command buffer, fence, semaphores, and dynamic uniform buffers. Per-frame resources must be indexed by frame index (not swap chain image index — Vulkan can return swap chain images out of order). Getting the indexing wrong causes data races between CPU and GPU.

- **GPU memory management.** Use VMA (Vulkan Memory Allocator) from the start. Raw `vkAllocateMemory` has a per-device allocation limit (often 4096 allocations) that you will hit quickly. VMA handles sub-allocation, memory type selection, and defragmentation. GPU memory is finite — on integrated GPUs, it's shared with system RAM. Handle allocation failures gracefully.

- **Descriptor set management.** Vulkan descriptors are verbose and error-prone. You need descriptor pool creation, descriptor set layouts, descriptor set allocation, and descriptor writes — and you must not update a descriptor set while the GPU is using it. Options: per-frame descriptor pools that reset each frame, or persistent descriptor sets with double-buffering. Starting simple (recreate per frame) is fine; optimize only when the profiler says to.

### Edge Cases

- Device lost (`VK_ERROR_DEVICE_LOST`): requires full device and resource recreation. Test this early — trigger it by removing a GPU driver or running a TDR (timeout detection and recovery).
- Multiple monitors with different DPI. The swap chain extent must use the correct DPI-adjusted size, or rendering is blurry or cropped.
- HDR monitors: swap chain format selection (`VK_FORMAT_A2B10G10R10_UNORM_PACK32`, `VK_FORMAT_R16G16B16A16_SFLOAT`) and color space (`VK_COLOR_SPACE_HDR10_ST2084_EXT`, `VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT`) require `VK_EXT_swapchain_colorspace`.
- Vulkan driver differences: NVIDIA, AMD, and Intel drivers have subtly different behavior around validation, memory types, and format support. Test on at least two vendors.

### Concerns

- **Future abstraction layer.** Write Vulkan code cleanly, but don't over-abstract prematurely. When the abstraction layer phase arrives, you'll extract an interface from working Vulkan code — this produces a better abstraction than designing one in a vacuum. Keep Vulkan-specific types (VkBuffer, VkImage, etc.) confined to the rendering module so the rest of the engine doesn't touch them directly.

---

## 6. Shader Pipeline

### Hard Parts

- **GLSL to SPIR-V compilation.** Use glslc (from the Vulkan SDK / shaderc) to compile GLSL to SPIR-V. This is straightforward for Vulkan-only, but keep in mind that when the abstraction layer phase arrives you'll need a cross-compilation strategy (HLSL via DXC, SPIRV-Cross to MSL, etc.). Writing clean GLSL now with clear binding conventions will make that transition easier.

- **Include handling.** glslc supports `#include` via the `GL_GOOGLE_include_directive` extension. You need dependency tracking for incremental compilation — parse `#include` directives or use glslc's `-M` flag for depfile output. CMake has no native shader dependency tracking, so this requires custom build commands.

- **Shader reflection.** After compilation, extract binding metadata (uniform buffers, textures, samplers, push constants) to set up Vulkan descriptor set layouts and pipeline layouts. Use SPIRV-Reflect or SPIRV-Cross for this. Alternatively, define bindings by convention (set 0 = per-frame, set 1 = per-material, set 2 = per-object) and skip runtime reflection for now.

- **Shader variants / permutations.** A PBR material might need variants: with/without normal map, with/without skinning, with/without alpha test. The combinatorial explosion of `#define` permutations can produce thousands of shader variants. You need a permutation management strategy:
  - Compile all permutations offline (slow compile, fast runtime).
  - Compile on demand and cache (fast iteration, potential runtime hitches).
  - Use Vulkan specialization constants (cheapest at runtime, avoids recompilation).

### Edge Cases

- SPIR-V validation (via spirv-val) should be part of the shader build pipeline. Invalid SPIR-V can crash specific GPU drivers while working on others.
- glslc optimization levels can affect precision. A shader that works at `-O0` may produce slightly different results at `-O`. Test both.
- Vulkan validation layers catch many descriptor mismatches at runtime — but only if you're testing the code path that uses that descriptor set.

Ship compiled SPIR-V bytecode, not source GLSL.

---

## 7. Render Graph

### Hard Parts

- **This is one of the most complex rendering subsystems.** Do not build it until you have several working passes (shadow, main, post-process) that you understand well enough to formalize their dependencies. The task list correctly places this in Phase 10 — but even then, start simple.

- **Graph compilation performance.** The graph is rebuilt every frame (because passes can be conditionally enabled/disabled). Compilation must be fast — microseconds, not milliseconds. This means: no heap allocations during graph building (use frame-linear allocators), no complex graph algorithms (topological sort on a small graph is fine, but avoid NP-hard scheduling optimizations).

- **Transient resource aliasing.** The graph compiler can determine that two textures are never alive at the same time and alias them to the same GPU memory. This saves VRAM but is tricky to implement correctly:
  - Aliasing barriers are needed on some APIs (D3D12 aliasing barriers, Vulkan memory aliasing).
  - If a resource is used in an async compute pass that overlaps with a graphics pass, aliasing is invalid.
  - Incorrect aliasing causes GPU memory corruption that is nearly impossible to debug.

- **Async compute integration.** A render graph can schedule work on the async compute queue (parallel to graphics). This is an advanced optimization that adds complexity: you need cross-queue synchronization (semaphores/fences), and not all passes are safe to run on the compute queue. Defer this until late in the project if at all.

- **Import/export of persistent resources.** Not all resources are transient (created and destroyed within a frame). The swap chain image, shadow map atlases, and temporal history buffers persist across frames. The graph must handle "imported" resources that it doesn't manage the lifetime of.

### Concerns

- Building the render graph too early will slow down iteration on rendering features. Build passes manually first (Phase 5's "minimal pass orchestration" is the right level), then extract the graph once patterns stabilize.
- Building it too late means retrofitting all passes to declare their dependencies, which is a large refactor.

### Tradeoffs

| Approach | Pro | Con |
|----------|-----|-----|
| Full render graph (Frostbite-style) | Automatic barriers, aliasing, scheduling | High complexity, long implementation time |
| Simplified pass system (ordered list with manual dependencies) | Easy to build, easy to debug | No automatic aliasing, manual barrier management |
| No abstraction (hardcoded pass order) | Simplest, fastest to build | Doesn't scale, barrier management scattered everywhere |

**Recommendation:** Start with the simplified pass system. Upgrade to a full render graph only if you need transient aliasing or async compute.

---

## 8. Materials & PBR

### Hard Parts

- **Energy conservation.** The split-sum approximation for IBL must be consistent with the direct lighting BRDF. If you use different NDF (Normal Distribution Function), geometry, or Fresnel terms for direct vs. indirect lighting, metals will be too bright or too dark. Stick to the same terms everywhere (GGX/Trowbridge-Reitz NDF, Smith-GGX geometry, Schlick Fresnel).

- **Pre-filtering the environment map.** The specular pre-filter convolves the environment cubemap at multiple roughness levels, storing the results in the mip chain. This is an offline or load-time GPU operation that is easy to get wrong:
  - Under-sampling produces sparkle artifacts on rough surfaces.
  - Over-sampling is slow.
  - The convolution must use importance sampling aligned to the GGX NDF.
  - Seams at cubemap edges are visible on rough materials. Use seamless cubemap filtering or octahedral mapping.

- **BRDF LUT generation.** A 2D lookup texture parameterized by (NdotV, roughness). Generated once, used for all IBL lookups. Must be generated with the same BRDF as the shader, or indirect specular will be wrong.

- **Linear vs. sRGB color space confusion.** Albedo textures are sRGB; normal maps, roughness, metallic are linear. If you sample a linear texture as sRGB (or vice versa), materials look wrong in subtle ways that are hard to diagnose. The texture import pipeline must tag textures with their color space, and the material system must create the correct texture views (sRGB vs. UNORM).

- **Tonemapping and exposure.** HDR rendering produces pixel values > 1.0. Tonemapping maps these to displayable range. The choice of tonemapping operator (Reinhard, ACES, AgX, Khronos PBR Neutral) dramatically affects the look of the game. Exposure must be settable (manual or auto-exposure via luminance histogram). Without proper exposure control, scenes are either washed out or too dark.

### Edge Cases

- Materials with missing textures: fall back to default values (white albedo, 0.5 roughness, 0.0 metallic, flat normal). Never crash or produce black surfaces due to missing assets.
- Materials with alpha transparency: transparent objects must render in a separate pass, sorted back-to-front. Alpha-tested (cutout) materials can render in the opaque pass with a discard but break early-Z optimization.
- Emissive materials with HDR values: emission can blow out with bloom post-processing. Emissive intensity must be artist-controllable and independent of bloom threshold.

---

## 9. Shadows

### Hard Parts

- **Cascaded shadow maps are a system, not a single technique.** CSM requires:
  - Splitting the view frustum into cascades (logarithmic, practical, or manual split distances).
  - Computing a tight orthographic projection per cascade.
  - Stabilizing the shadow map as the camera moves (texel snapping) to prevent shadow shimmer.
  - Blending between cascades at their boundaries to hide transitions.
  - Fitting cascade distances to the scene — too few cascades or bad splits waste resolution.

- **Shadow acne and peter-panning.** Shadow acne (Moiré-like self-shadowing artifacts) is caused by insufficient depth bias. Adding more bias causes peter-panning (shadows detach from objects). The correct bias is view-dependent and scale-dependent. Slope-scaled bias helps but doesn't eliminate the problem. Normal offset bias (pushing the shadow receiver along its normal) is the most robust fix, but adds complexity.

- **Shadow map resolution vs. quality.** A 2048x2048 shadow map for a directional light gives approximately 1 texel per ~5cm at 50m range. For large outdoor scenes, this is insufficient. Higher resolution maps cost GPU memory and fill rate. Percentage-closer filtering (PCF), Variance Shadow Maps (VSM), or Moment Shadow Maps improve quality at the cost of performance. PCF is the simplest and most predictable.

- **Point light shadows are expensive.** Rendering 6 faces of a cubemap per point light per frame is costly. With 10 point lights, that's 60 shadow passes. Solutions:
  - Shadow atlas: pack all shadow maps into a single large texture.
  - Caching: only re-render shadow maps for lights whose casters moved.
  - Budget: limit the number of shadowed lights (e.g., 4 shadowed point lights max, rest are unshadowed).

### Edge Cases

- Lights near or inside geometry produce degenerate shadow projections.
- Transparent/alpha-tested objects in shadow maps: do you render them? Alpha-tested shadows are expensive (no early-Z). Transparent shadows require either colored shadow maps or opacity dithering.
- Moving lights cause full shadow map invalidation every frame. For mostly-static scenes, caching is a big win.

---

## 10. Ray Tracing

### Hard Parts

- **BLAS/TLAS management with dynamic scenes.** BLAS builds are expensive. For static meshes, build once. For skinned/deforming meshes, you must rebuild or refit BLAS every frame. Refitting is cheaper but produces lower-quality acceleration structures over time.
  - TLAS must be rebuilt every frame if any object moves. TLAS builds are cheaper than BLAS but still not free with thousands of instances.
  - Budget: measure BLAS/TLAS build time and set limits on how many dynamic objects can participate in RT.

- **Denoising.** Hardware RT at 1 sample per pixel produces extremely noisy results. You need a denoiser:
  - Temporal accumulation (reproject previous frame's results using motion vectors) reduces noise over time but ghosts on disocclusion.
  - Spatial filters (edge-aware blur) clean up remaining noise but lose detail.
  - A proper denoiser (SVGF, ReLAX, or similar) requires: motion vectors, depth, normals, and mesh ID buffers. This is a significant rendering infrastructure requirement just to make RT usable.

- **Hybrid pipeline complexity.** The hybrid pipeline uses rasterization for primary visibility and RT for secondary effects (shadows, reflections). This means every pixel needs data from both pipelines. Resource management, synchronization, and pass ordering become more complex. The fallback path (no RT) must produce acceptable results on its own — you're maintaining two lighting pipelines.

- **Ray tracing shader compilation.** RT shaders (ray generation, closest hit, any hit, miss) use a different compilation model. glslc can compile GLSL RT shaders to SPIR-V, but testing is harder — you can't step through RT shaders in most debuggers. Validation layers help but are slower.

### Concerns

- RT is not available on all hardware. Any feature using RT must have a complete non-RT fallback. This means designing features "raster-first" and adding RT as an enhancement, not building features that only work with RT.
- Vulkan RT extension support varies by driver version and hardware generation. Check `VK_KHR_ray_tracing_pipeline` and `VK_KHR_acceleration_structure` availability before committing to advanced RT features.

---

## 11. GPU-Driven Rendering

### Hard Parts

- **This is a very large architectural decision.** GPU-driven rendering fundamentally changes how you structure your renderer:
  - All geometry must be in persistent GPU buffers (bindless or large SSBOs).
  - Materials must be indexable by ID rather than bound per draw call.
  - Culling happens on the GPU — your CPU-side frustum/occlusion culling becomes redundant or supplementary.
  - Draw calls are generated by compute shaders and issued via `DrawIndirect` / `DrawIndexedIndirect`.

- **Bindless resource model.** GPU-driven rendering typically requires bindless textures (all textures in a single descriptor array, indexed in the shader). Vulkan supports this via `VK_EXT_descriptor_indexing` (core in Vulkan 1.2), but not all hardware supports the full feature set (check `descriptorBindingPartiallyBound` and `runtimeDescriptorArray`).

- **Debugging is harder.** You can't printf from a GPU culling shader. Visual artifacts from GPU-driven rendering are hard to diagnose — is it the culling compute shader? The indirect argument buffer? A wrong instance ID? GPU debugging tools help but have limitations.

### Tradeoffs

| Approach | Pro | Con |
|----------|-----|-----|
| CPU-driven (traditional) | Simple, debuggable, well-understood | CPU bottleneck with many draw calls |
| GPU-driven (full) | Scales to millions of objects, minimal CPU overhead | Major complexity, bindless requirements, hard to debug |
| Hybrid | GPU culling, CPU dispatch | Moderate complexity, good performance |

**Recommendation:** Start CPU-driven. Profile. If CPU draw call submission is the bottleneck (unlikely for a learning engine with moderate scene complexity), consider GPU culling as a targeted optimization. Do not build a GPU-driven renderer from the start.

---

## 12. GPU Particles & VFX

### Hard Parts

- **Dead particle management.** Particles die at different times. You need a free-list or compaction pass to reuse dead particle slots. Without compaction, the buffer becomes sparse, wasting GPU threads on dead particles. Compaction itself is a non-trivial GPU operation (parallel prefix sum / stream compaction).
- **Sorting transparent particles.** Transparent particles must render back-to-front. GPU sorting (bitonic sort, radix sort) on the distance to camera. Sorting thousands of particles per frame on the GPU is doable but adds significant compute work.
- **Collision with the scene.** Particles that collide with geometry need either:
  - Depth buffer collision (simple, but only works for on-screen geometry, one layer deep).
  - SDF (Signed Distance Field) collision (accurate, but requires maintaining an SDF of the scene).
  - No collision (simplest, often acceptable for effects like sparks and dust).

### Edge Cases

- Emitting particles from a moving object: the emission position must be interpolated across the frame to avoid clumping at high emission rates.
- Particle count overflow: if more particles are spawned than the buffer can hold, silently drop new particles rather than crashing.

---

## 13. Physics (Jolt)

### Hard Parts

- **Transform ownership between physics and gameplay.** This is the single most important design decision for the physics integration. There are two approaches:

  1. **Physics owns the transform.** After each physics step, copy Jolt body transforms to GameObjects. Gameplay code moves objects by applying forces/velocities, never by setting position directly. Problem: teleportation (setting an object's position for gameplay reasons) requires calling into Jolt to reposition the body, which can cause tunneling if the body overlaps geometry at the new position.

  2. **Gameplay owns the transform.** Gameplay sets positions freely, and physics reads them as kinematic targets. Problem: physics simulation doesn't work — dynamic bodies need to be driven by the physics engine, not by gameplay.

  In practice, you need both: **dynamic bodies are owned by physics**, **kinematic bodies are owned by gameplay**. The component must clearly expose which mode it's in and prevent invalid operations (e.g., setting position directly on a dynamic body logs a warning).

- **Fixed timestep alignment.** Jolt must step at the same fixed timestep as the rest of the simulation. If you sub-step physics (multiple physics steps per simulation tick), Jolt supports this natively, but your collision callbacks fire multiple times per frame and gameplay must handle that.

- **Collision callback complexity.** Jolt provides callbacks during simulation (contact added, persisted, removed). These callbacks fire on Jolt's worker threads if you use Jolt's built-in job system. Your callbacks must be thread-safe, which means no modifying game state directly in callbacks — buffer events and process them on the main thread after the step.

- **Shape cooking and runtime cost.** Complex collision shapes (convex hulls, mesh shapes) must be cooked (preprocessed). Cooking is slow — do it at import time, not runtime. Store cooked shapes in the asset pipeline. Mesh collision shapes should only be used for static geometry. Dynamic bodies should use simple shapes (box, sphere, capsule, convex hull with low vertex count).

- **Determinism.** Jolt is deterministic on the same platform/build given the same input — sufficient for debugging. See [Cross-Cutting Concerns](#26-cross-cutting-concerns) for engine-wide determinism requirements.

### Edge Cases

- Stacking stability: Jolt handles this well, but extreme stacking (100+ bodies) can exhibit jitter. Increase solver iterations for stacking-heavy scenes.
- Bodies falling asleep at inconvenient times: a body resting on a moving platform may sleep and stop responding. Jolt has sleep thresholds; you may need to wake bodies manually in some gameplay scenarios.
- Destruction: removing a body mid-simulation step crashes. Always defer body removal to before or after the Jolt step.
- Scale changes: Jolt shapes don't support runtime scale changes on most shape types. Changing an object's scale requires recreating the shape at the new scale. This is expensive and should be uncommon.

---

## 14. Animation

### Hard Parts

- **Bone limits and GPU skinning.** Vertex shader skinning typically supports 4 bone influences per vertex. The maximum number of bones per draw call is limited by the uniform/constant buffer size. With 256 bones at 64 bytes each (mat4), that's 16KB — within typical limits. But characters with more bones or multiple skinned meshes drawn in one call may exceed this. Compute shader skinning removes this limit but adds a pre-pass.

- **Animation compression.** Raw animation data is large: N bones × M keyframes × (position + rotation + scale) × 32-bit floats. A 60-second clip at 30fps with 100 bones is ~14MB uncompressed. You need compression:
  - Quantize rotations (smallest-three, 48-bit quaternions).
  - Quantize positions (16-bit fixed point relative to bind pose).
  - Remove redundant keyframes (curve simplification).
  - This is a significant offline pipeline task.

- **Root motion.** The root bone's movement can be extracted and applied as gameplay movement (root motion) instead of animating the visual mesh. This affects physics: the character controller must advance by the root motion delta each frame. Problems:
  - Root motion and physics input conflict: if the animation says "move forward 2m" but there's a wall, who wins? The physics engine must constrain root motion. This means extracting root motion, feeding it to the character controller as a desired velocity, and letting physics resolve the actual movement.
  - Blended animations produce blended root motion, which can cause sliding.

- **Animation state machine.** A blend tree with states and transitions is conceptually simple but implementation gets complex with:
  - Transition interruptions (transitioning from A to B, but a trigger interrupts to C — do you blend A→C or (A→B blend)→C?).
  - Layered animations (upper body attack while lower body runs) with mask-based blending.
  - Additive animations (breathing overlay on any base pose).
  - Synchronized animations (two characters grappling, animations must stay in sync).

- **Inverse Kinematics (IK).** Not in the task list, but almost always needed for:
  - Foot placement (adjusting feet to uneven terrain).
  - Look-at / aim-at (pointing a weapon at a target).
  - Hand IK for interacting with objects.

  Two-bone IK (for limbs) is straightforward. Full-body IK (FABRIK, CCD) is significantly more complex. Consider adding basic two-bone IK as a stretch goal.

### Edge Cases

- Animation clips with different numbers of bones than the skeleton. The import pipeline must validate bone compatibility.
- Extremely short blend times (< 1 frame) cause pops. Clamp blend duration to at least one simulation tick.
- Animations authored at different frame rates than the engine's tick rate. Clips must be sampled at arbitrary time values, not just at keyframe boundaries.

---

## 15. Audio

### Hard Parts

- **Audio threading.** raudio processes audio on a separate thread (its internal callback thread). This means:
  - Audio state modifications from gameplay must be thread-safe. If gameplay calls `PlaySound()`, the actual playback starts on the audio thread — the gameplay thread must not touch audio buffer pointers.
  - Latency: there is inherent latency between "gameplay requests sound" and "sound is heard" (typically 10-50ms depending on buffer size). For music, this is fine. For gameplay feedback (gunshot, footstep), even 50ms can feel laggy.

- **3D audio positioning.** Basic 3D audio uses distance attenuation and left-right panning. More realistic spatial audio uses HRTF (Head-Related Transfer Function), which simulates how sound wraps around the head. raudio does not provide HRTF out of the box. For a learning engine, simple distance + panning is sufficient. Note that the listener's position and orientation must be synced every frame from the camera or player object.

- **Streaming vs. loaded.** Short sound effects should be fully loaded into memory. Music and ambient tracks should be streamed from disk. The audio system must handle both, and the boundary should be configurable (e.g., sounds > 5 seconds are streamed). Streaming requires async disk IO that stays ahead of playback.

- **Audio buses and mixing.** A bus hierarchy (master -> music, SFX, voice, ambient) with per-bus volume control. Each bus can have effects (reverb, compression). This is essential for the player to have separate volume sliders. raudio's built-in mixing is basic — you may need to layer your own submix system on top.

### Edge Cases

- Playing many sounds simultaneously: with 50+ concurrent sounds, mixing becomes expensive and the output gets muddy. Implement a priority system: quiet/distant sounds are culled or virtualized (tracked but not mixed).
- Alt-tabbing: should audio mute when the window loses focus? This is a user preference. Default to muting, but make it configurable.
- Audio during PIE mode: the editor should mute or attenuate game audio when not in PIE mode. Entering PIE starts the listener; exiting PIE stops it.

---

## 16. Asset Pipeline

### Hard Parts

- **The .meta file model is critical and hard to get right.** Every design decision here has serialization consequences:
  - `.meta` files must be committed to version control alongside source assets. If a `.meta` file is lost, the asset gets a new UUID, breaking all references to it. Educate users (yourself) about this early.
  - Import settings in the `.meta` file determine how the asset is cooked. Changing import settings must trigger re-cook of the asset AND all assets that depend on it.
  - `.meta` file format changes between engine versions must be handled by migration code, or old projects break on engine update.

- **Dependency tracking.** A material asset references texture assets. If a texture is re-imported (changed import settings or modified source file), the material must be re-cooked. This requires a dependency graph:
  - Forward dependencies: "this material depends on these textures."
  - Reverse dependencies: "these materials depend on this texture" (needed for invalidation).
  - Circular dependencies are possible (shader includes, material references) and must be detected and broken.

- **Hot reload of in-use assets.** When an asset is re-imported while the engine is running:
  - Textures bound to descriptors cannot be destroyed while the GPU is using them. You must wait for in-flight frames to complete, then swap the resource and update descriptor sets.
  - Meshes currently being rendered need their vertex/index buffers swapped. Any cached draw calls referencing old buffers must be invalidated.
  - Materials that changed need their pipeline state objects (PSOs) recreated if shader bindings changed.
  - This is a major source of bugs. Each system must support "resource replaced" notifications, and the replacement must be atomic from the renderer's perspective.

- **Async import in the editor.** Large assets (high-poly meshes, 4K textures) take time to import. Blocking the editor UI is unacceptable. The import must happen on a background thread, with progress reported to the UI. The asset database must handle concurrent access: the background thread is writing cooked data while the main thread reads the database for rendering. You need either locking or an append-only/MVCC design for the database.

- **File watching on Windows.** To detect source file changes for hot reload, you need filesystem notifications. On Windows, `ReadDirectoryChangesW` works but has quirks: it can miss events if the buffer overflows, and it reports changes at the directory level, not individual files. You must debounce events (editors like Photoshop write files in multiple steps) and reconcile with the asset database.

### Edge Cases

- Renaming or moving source files: the `.meta` file must follow the source file. If the user renames a file outside the editor (in Explorer/Finder), the `.meta` file is orphaned and a new one is created, breaking references. The editor should provide rename/move operations that handle this correctly.
- Deleting a source file that is referenced by other assets: the engine must not crash. References to missing assets should resolve to a placeholder (pink texture, missing mesh indicator).
- Importing the same source file twice under different names: each import gets a unique UUID, resulting in duplicate cooked data. Not harmful but wasteful. Detection is possible via content hashing.
- Asset paths with unicode characters, spaces, or very long paths (Windows 260-char limit). Use the `\\?\` prefix on Windows for long paths and test with unicode paths early.

---

## 17. Gameplay Modules & Hot Reload

### Implementation Detail

The hot reload cycle: serialize all game state → unload old DLL → copy and load new DLL (can't overwrite a loaded DLL on Windows) → deserialize into new code.

### Hard Parts

- **The module boundary is the single most constrained API in the engine.** Everything that crosses the DLL boundary must be ABI-stable:
  - No passing `std::string`, `std::vector`, or any STL type across the boundary (layout differs between MSVC CRT versions, Debug vs Release).
  - No passing objects with vtables across the boundary (vtable layout can change on recompilation).
  - No returning or accepting `new`/`delete`-allocated memory across the boundary (each DLL has its own heap on Windows).
  - Safe types: primitives, C-style structs with no virtual functions, opaque handles, function pointers.
  - Typically implemented as a C-style interface: the DLL exports `CreateModule()` returning a struct of function pointers, and the engine calls those function pointers.

- **State serialization across reload.** When you unload the DLL, all gameplay objects in that DLL's memory are invalidated. You must:
  - Serialize all gameplay state before unload.
  - Deserialize after loading the new DLL.
  - Gameplay components must implement serialize/deserialize, and these functions must handle schema changes gracefully (new fields get defaults, removed fields are ignored).
  - Object references (handles) must survive serialization — this is another reason generational handles are valuable (they're just integers).

- **Static state in the DLL.** Global/static variables in the gameplay DLL are lost on reload. If gameplay code uses singletons, static caches, or file-scoped state, that state is gone after hot reload. This is a constant source of bugs. Rule: no static mutable state in gameplay modules.

- **The DLL copy dance on Windows.** You cannot overwrite a DLL that is currently loaded. The standard workaround: copy the DLL to a temp name, load the copy. On next reload, unload the copy, delete it, copy the new DLL to a new temp name, load it. File locking issues and stale temp files accumulate. Clean up temp files on editor startup.

- **Debugger interaction.** When the DLL is unloaded and a new one is loaded, breakpoints in the old DLL are invalidated. The debugger may need to re-attach or re-set breakpoints. This makes debugging hot-reloaded code painful. Some developers just restart the editor for debug sessions. Consider a "disable hot reload when debugger attached" mode.

### Edge Cases

- Hot reloading while PIE mode is active. Game state is live and being simulated. The serialize-reload-deserialize cycle must pause simulation. If the new code has a bug that crashes during deserialization, the editor must catch the crash and offer to revert to the old DLL.
- Adding a new component type in a hot reload: the engine needs to register the new type. Removing a component type: existing instances in the scene become orphans.
- Template/inline functions in shared headers: if the engine and the DLL both instantiate a template, the two copies may have different behavior after recompilation. Avoid templates in the module boundary headers.

---

## 18. Editor

### Hard Parts

- **Undo/redo is architecturally invasive.** Every editor operation that modifies state (move object, change property, delete component, reparent, etc.) must be recorded as a reversible command. This means:
  - A command object with `Execute()`, `Undo()`, and optionally `Redo()`.
  - Every state-modifying code path must go through the command system. Direct state mutation bypasses undo and is a bug.
  - Compound operations (e.g., "duplicate object" = create object + copy components + copy children) must be a single transaction so undo reverses the whole operation.
  - Commands must capture enough state to reverse themselves. For property changes, store the old and new value. For object creation, store enough to recreate or destroy the object. For hierarchy changes, store the old parent.
  - Memory: the undo stack can grow unboundedly. Cap it (e.g., 100 operations) and release old commands.
  - Systems that are hard to undo: physics state (Jolt body positions), GPU resources (descriptor allocations). These must be regenerated from the game state after undo, not undone directly.

- **Play-in-Editor (PIE) state management.** Entering play mode requires:
  1. Snapshot the entire editor world state (serialize to memory).
  2. Start simulation.
  3. On exit: destroy the simulated world, deserialize the snapshot.

  Hard parts:
  - GPU resources created during play (textures loaded, buffers allocated) must be cleaned up on exit.
  - Objects spawned during play must not exist after exit.
  - Objects destroyed during play must be restored.
  - If the editor crashes during play mode, the snapshot is lost. Save it to disk as a recovery mechanism.
  - The undo history should be cleared or frozen during play mode (you can't undo gameplay).

- **Viewport picking.** Two approaches:
  1. **GPU picking:** Render the scene with object IDs encoded in a render target. Read back the pixel under the mouse to get the selected object ID. Pros: pixel-perfect, handles all geometry. Cons: requires a readback from GPU to CPU (latency — at least one frame), requires a dedicated render pass.
  2. **CPU ray casting:** Cast a ray from the mouse position into the scene and intersect with object bounding boxes or collision shapes. Pros: immediate result, no GPU readback. Cons: doesn't match visual geometry exactly (bounding boxes are approximations), doesn't handle transparency or custom shaders.

  **Recommendation:** GPU picking for accuracy. Buffer the readback (read the result from 1-2 frames ago) to avoid stalling the GPU pipeline.

- **Gizmo interaction.** Translation/rotation/scale gizmos in the viewport:
  - Gizmos must render on top of the scene (depth test disabled or special depth handling).
  - Gizmo handles must be pickable (use the same picking system or separate ray-intersection for gizmo geometry).
  - Dragging a gizmo must generate undo-able commands.
  - Gizmo transform space: local vs. world vs. parent. The user must be able to switch.
  - Snapping (position to grid, rotation to degrees, scale to increments).
  - Multi-selection: moving multiple objects at once. The gizmo is at the selection centroid, and each object moves by the same delta.

- **ImGui limitations and workarounds:**
  - ImGui's docking branch is perpetually "beta." It works well in practice but has known bugs with certain docking configurations and multi-viewport setups.
  - ImGui is immediate-mode: the UI is rebuilt every frame. This is great for simplicity but means you can't easily do animations, complex layouts, or rich text. For a game engine editor, this is usually fine.
  - Styling is limited. ImGui can be themed but will never look like a native application. Accept this tradeoff.
  - ImGui does not support accessibility (screen readers, keyboard navigation). This is a limitation of the library.
  - Text input in ImGui interacts with the engine's input system. When an ImGui text field is focused, keyboard input must NOT go to the game/editor viewport. Check `ImGui::GetIO().WantCaptureKeyboard` and `WantCaptureMouse` every frame.

### Edge Cases

- Selecting an object through a transparent surface (glass window). GPU picking returns the front-most rendered pixel, which may be the transparent surface. The user may want to select what's behind it. Consider a "click through transparent" option or a hierarchy-based selection alternative.
- Undoing an operation that referenced an object that was subsequently deleted: the undo command holds a stale reference. Commands must validate their targets before executing undo.
- Layout persistence: saving and restoring ImGui docking layout across editor sessions. ImGui provides `imgui.ini` for this, but it can get corrupted. Handle the case where the layout file is invalid (fall back to a default layout).
- Extremely deep scene hierarchies (1000+ levels) in the hierarchy panel. ImGui tree nodes at this depth are slow to render and unusable to navigate. Impose a practical depth limit or provide a search/filter mechanism.

---

## 19. Runtime UI

### Hard Parts

- **Building a UI framework is a project in itself.** Even a minimal one needs:
  - A layout system (anchoring, alignment, sizing relative to parent or screen).
  - Hit testing for mouse/touch input.
  - Text rendering with word wrap and alignment (stb_truetype provides glyph rasterization, but not layout — you must implement line breaking, text shaping, and glyph positioning yourself).
  - A widget set (button, label, image, slider, scroll area, list).
  - Event propagation (which widget gets the click? Topmost? Focus-based?).
  - Screen resolution independence (UI authored at one resolution, scaled to others). DPI awareness.

- **Input focus ownership.** Three consumers of input: gameplay, runtime UI, and editor UI (in editor builds). Only one should receive input at a time. A focus stack or priority system determines who gets input:
  - If a UI menu is open, it captures input. Gameplay doesn't receive it.
  - If the console is open, it captures keyboard. Mouse may still go to gameplay.
  - In editor: ImGui captures input when hovering its windows. Viewport input goes to editor camera or game camera depending on PIE mode.
  - This is a state machine and it must be explicit. Implicit "who has focus?" is a source of input bugs (WASD moves the character while typing in a chat box).

### Tradeoffs

| Approach | Pro | Con |
|----------|-----|-----|
| Custom immediate-mode (like ImGui for gameplay) | Simple, fast to iterate | Limited layouts, no animation, rebuild every frame |
| Custom retained-mode (like a mini HTML/CSS) | Rich layouts, animation, state | Significant implementation effort |
| Third-party (RmlUI, Yoga for layout) | Feature-rich, maintained | External dependency, integration effort |

**Recommendation:** Start with the absolute minimum: render textured quads and text. Build specific widgets as needed for your game demo. Do not build a general-purpose UI framework — that's a multi-month project on its own.

---

## 20. Networking

### Hard Parts

- **This is the hardest system in the engine.** Networking touches every other system: game objects (replication), physics (prediction/reconciliation), serialization (delta compression), the game loop (tick alignment), animation (lag compensation). It is not a feature you "add" — it is an architectural concern that shapes the entire engine.

- **Replication model.** What gets replicated and how:
  - Each replicated object needs a network ID (unique across the network, different from its local UUID).
  - The server assigns network IDs. Clients map network IDs to local objects.
  - Properties must be marked as replicated. Only replicated properties are serialized and sent.
  - Replication frequency: not everything updates every tick. A priority system sends the most important/changed objects first, within a bandwidth budget.
  - Large worlds: relevancy sets determine what each client receives. An object 1km away doesn't need position updates at 60Hz.

- **Client-side prediction and reconciliation.** The client predicts its own movement locally (so input feels responsive) and receives corrections from the server:
  1. Client sends input + tick number to server.
  2. Client immediately applies input locally (prediction).
  3. Server receives input, simulates, sends authoritative state + tick number.
  4. Client receives server state for tick N. Compares predicted state at tick N with server state.
  5. If they match: prediction was correct. If not: snap to server state and re-simulate from tick N to current tick using stored inputs (reconciliation / rollback).

  Hard parts:
  - Storing input history: you must buffer the last N inputs for reconciliation.
  - Re-simulation: when correcting, you run the simulation forward from the server state, applying buffered inputs. This means simulation must be deterministic given the same inputs (within a platform/build). It must also be fast enough to re-simulate multiple ticks in one frame.
  - Physics reconciliation: Jolt does not natively support rollback. Reconciling physics state means either:
    - Rolling back the entire physics world (save/restore full state — expensive, complex).
    - Only predicting non-physics gameplay state and accepting server-authoritative physics with interpolation.
    - Limiting prediction to simple kinematics and letting physics be server-only.

  **Recommendation:** Only predict character movement (simple kinematic movement, not full physics). Accept server-authoritative physics for everything else. This avoids the physics rollback problem entirely while giving responsive player movement.

- **Interpolation and jitter.** Non-local entities (other players, NPCs) are displayed with interpolation between received server states. This requires buffering 2+ server snapshots and interpolating between them. The interpolation delay (typically 2× server tick interval) adds to perceived latency. A jitter buffer absorbs network timing variance but adds more delay. The tradeoff is smoothness vs. latency — there is no perfect setting.

- **Delta compression.** Sending full state every tick wastes bandwidth. Delta compression sends only what changed since the last acknowledged state. This requires:
  - Tracking what each client has acknowledged receiving.
  - Computing diffs against the last acknowledged state.
  - Handling lost packets: if the baseline state is lost, the diff is meaningless. Use reliable delivery for baselines and unreliable for deltas, with periodic full-state fallback.

- **Object spawning across the network.** When the server creates an object:
  1. Server assigns a network ID and sends a "spawn" message with initial state.
  2. Client receives the spawn message and creates a local object.
  3. Until the client receives the spawn message, it doesn't know the object exists. Other messages referencing that object's network ID arrive before the spawn message — buffer or drop them?
  4. When the server destroys an object, the client must also destroy it. But the client may have references to it (e.g., "I'm targeting that enemy"). Use the same deferred destruction as local objects.

### Edge Cases

- Network partitions: a client stops receiving updates but the connection isn't closed. Implement a heartbeat and timeout mechanism. ENet provides this, but the timeout parameters need tuning.
- Clock synchronization: the server and client clocks drift. You need a clock sync protocol (similar to NTP) to estimate one-way latency and clock offset. Without this, tick numbers are meaningless.
- Cheating: in a server-authoritative model, the server validates inputs. But it can't validate everything (e.g., aim direction). For a learning engine, don't build anti-cheat — just build the authority model correctly.
- Joining mid-game: the new client needs the full world state. This "initial replication" is a large message that must be broken into chunks and sent reliably. The client shows a loading screen during this.
- Host migration (if you ever go peer-to-peer): not worth implementing. Server-authoritative with a dedicated server process is simpler and more robust.

### Concerns

- **Do not underestimate the scope.** Networking is often 40-60% of a multiplayer engine's complexity. For a learning project, consider implementing networking as the very last major feature, and scope it to a simple use case (2-4 players in a small arena).
- **Test with simulated latency and packet loss from day one.** ENet or OS-level tools can simulate bad network conditions. Networking that works on localhost always works — the bugs only appear with 50-200ms latency and 1-5% packet loss.

---

## 21. Memory & Allocation

### Hard Parts

- **Custom allocators add complexity for uncertain benefit.** Profile first. The default system allocator on modern platforms (Windows heap, glibc malloc, jemalloc) is fast and well-tested. Custom allocators are justified only when:
  - The profiler shows allocation/deallocation as a hotspot (unlikely for most game code).
  - Fragmentation is causing out-of-memory in long-running sessions (unlikely with modern allocators).
  - You need deterministic allocation patterns (e.g., for frame timing guarantees — relevant for VR, not for a learning engine).

  Start with the system allocator and debug tracking. Add custom allocators only where profiling shows a need.

- **Frame allocator sizing.** The frame allocator has a fixed budget (e.g., 16MB). If a frame exceeds this budget (e.g., a spike in particle count or a debug visualization that draws 100K lines), the allocator overflows. Options: assert and crash (easiest to diagnose), fall back to the system allocator (hides the problem), grow the buffer (wastes memory most frames). **Recommendation:** Assert in debug, fallback in release.

- **GPU memory is a separate concern.** VMA handles suballocation and memory type selection, but you must still track total GPU memory usage and warn when approaching the budget. On discrete GPUs, VRAM is limited (4-12GB typically). On integrated GPUs, VRAM shares system RAM. Release resources when they're no longer needed (reference counting or explicit lifecycle).

### Edge Cases

- Leak detection with DLL hot reload: when the gameplay DLL is unloaded, allocations made by the DLL (with a DLL-local heap) are freed. If you track allocations globally, these look like double-frees or leaks. The tracking system must be aware of DLL boundaries.
- Thread safety: if the frame allocator is per-thread, no synchronization needed. If shared, every allocation is a contention point. Use per-thread allocators for parallel work.

---

## 22. Threading & Task System

### Hard Parts

- **Most game engine code is fundamentally single-threaded.** The main game loop, gameplay logic, ImGui rendering, and render submission are all single-threaded by default. The task system's value is in offloading background work, not in parallelizing the frame. Over-parallelizing the frame adds synchronization complexity for minimal gain in a learning engine.

- **Jolt's threading model.** Jolt has its own job system interface. You can implement it to use your task system, or let Jolt use its default thread pool. If Jolt uses its own threads, you have two thread pools competing for CPU cores. **Recommendation:** Implement Jolt's `JobSystem` interface to use your task system. This is straightforward (Jolt provides an example implementation) and avoids thread oversubscription.

- **Work stealing vs. simple queue.** A simple shared task queue with a mutex works fine for light loads. Work stealing (each thread has a local queue, steals from others when empty) improves throughput under heavy load. For this project, a simple queue is sufficient — work stealing adds complexity for marginal gain.

- **Main thread completion callbacks.** Background tasks (asset import) often need to deliver results to the main thread. Options:
  - A completion queue polled each frame on the main thread.
  - A future/promise pattern where the main thread checks `isDone()`.
  - Callback functions executed on the main thread.

  All are valid. The completion queue is simplest and most predictable.

### Concerns

- Thread sanitizer (TSan) should be part of CI for debug builds. Data races in multithreaded engine code are subtle and may only manifest as rare crashes on specific hardware.
- Avoid `std::async` — its thread management behavior is implementation-defined and can create unbounded threads.

---

## 23. Platform & Windowing

### Hard Parts

- **GLFW and Vulkan surface creation.** GLFW provides `glfwCreateWindowSurface()` which handles platform-specific surface creation (Win32, Xlib/Wayland, Cocoa). This is straightforward, but on macOS you'll eventually need a `CAMetalLayer` for the Metal backend when the abstraction layer is added. For now, GLFW + Vulkan surface works cleanly on Windows and Linux.

- **DPI awareness on Windows.** Windows has three DPI awareness modes: unaware, system, and per-monitor. GLFW supports per-monitor DPI. But the window content scale (1.0 at 96dpi, 1.5 at 144dpi, 2.0 at 192dpi) affects:
  - ImGui font rendering (fonts must be loaded at scaled sizes).
  - Swap chain extent (physical pixels ≠ logical pixels).
  - Mouse coordinates (GLFW reports in screen coordinates, which may differ from pixel coordinates).
  - Moving a window between monitors with different DPI triggers a rescale event. Handle it by resizing the swap chain and reloading scaled fonts.

- **Input handling nuances.**
  - Key repeat: GLFW fires repeated key events when a key is held. You usually want repeat for text input but not for gameplay (use key state polling for gameplay, events for text).
  - Gamepad support: GLFW has basic gamepad support (via `glfwGetGamepadState`). It's functional but limited. For a learning engine, keyboard+mouse is sufficient initially.
  - Mouse capture: in FPS mode, the mouse should be locked and hidden (`glfwSetInputMode(GLFW_CURSOR_DISABLED)`). Escape should release the cursor. Handling the transition between captured and free cursor is a frequent source of bugs (cursor jumps, raw input deltas spike on capture/release).

### Edge Cases

- Minimized window: the swap chain extent may be 0x0. Skip rendering entirely when minimized.
- Alt+Enter for fullscreen toggle: implement as a borderless fullscreen window (change window size and position to cover the monitor) rather than exclusive fullscreen (which causes mode switches and is increasingly deprecated).
- Multi-monitor: the window can span two monitors. The swap chain presents to one monitor's refresh rate. VSync behavior is undefined when spanning monitors. Avoid this scenario or document it as unsupported.

---

## 24. Diagnostics & Profiling

### Hard Parts

- **Validation layers slow rendering significantly.** Vulkan validation layers can reduce framerate by 5-10×. They should be enabled in debug builds and CI, but you must have a way to run debug builds without validation for performance iteration. A command-line flag (`--no-validation`) or build option is sufficient.

- **GPU profiling is asynchronous.** GPU timestamp queries return results 1-3 frames later. You must read results from previous frames, not the current one. The profiling display in the editor shows data that is a few frames old. This is expected and unavoidable.

- **Crash handling.** When the engine crashes:
  - On Windows: set an unhandled exception filter (`SetUnhandledExceptionFilter`) that writes a minidump (`.dmp` file). Include the call stack if symbols are available.
  - Log the last N log messages before the crash (ring buffer). Often the log messages leading up to a crash are more useful than the crash stack itself.
  - GPU crashes (device lost) don't trigger CPU crash handlers. Detect device lost from API return codes and log as much GPU state as possible before exiting.

- **Render comparison tests.** Screenshot-based tests are useful for regression detection but are fragile:
  - Different GPUs produce slightly different results (floating point rounding in shaders). Pixel-exact comparison fails across hardware. Use perceptual comparison (SSIM, pixel threshold) instead.
  - The test must run in a deterministic, repeatable scenario (fixed camera, fixed time, fixed random seed).
  - CI must have a GPU. Headless rendering (via `VK_EXT_headless_surface` or software rasterizers) is possible but produces different results than real hardware.

---

## 25. Packaging & Shipping

### Hard Parts

- **Cooked builds must not access source assets.** The virtual filesystem must resolve paths differently in editor (source assets available) vs. packaged builds (only cooked assets). Every asset load path must work in both modes. Test the packaged build early and often — it is common for features to work in the editor but break in packaged builds because they accidentally reference source paths.

- **Shader compilation in cooked builds.** All shader variants must be pre-compiled during cooking. If the game encounters a shader variant at runtime that wasn't compiled, it fails. The cooking step must enumerate all possible shader variants (material permutations × pass combinations × platform). This is where the permutation explosion from Section 6 becomes a concrete problem.

- **Save data location.** On Windows, game saves should go in `%APPDATA%/Sarsa/` or `%LOCALAPPDATA%/Sarsa/`, not in the game's install directory (which may be read-only). The virtual filesystem must provide a writable save path that works in both editor and packaged builds.

### Edge Cases

- Anti-virus software on Windows may quarantine the game executable if it's unsigned. Users will see a SmartScreen warning. Code signing mitigates this but requires a certificate.
- The game executable's working directory may not be the same as its install directory if launched from a shortcut or command line. Use the executable's path, not the working directory, to resolve asset archives.
- Steam/Epic store integration (if ever considered): these platforms have their own packaging, DRM, and overlay requirements. Design the packaging system to be store-agnostic.

---

## 26. Cross-Cutting Concerns

### Serialization Versioning

Every serialized format (scenes, assets, materials, cooked data) will change as the engine evolves. Without versioning, old files become unreadable. Include a version number in every serialized file. When loading, check the version and either:
- Migrate (convert old format to new) — most user-friendly but requires writing migration code for every format change.
- Reject (error on old versions) — simplest but forces re-export, frustrating for users.

For JSON formats, prefer additive changes (new fields with defaults) over destructive changes (renamed/removed fields). This provides implicit forward compatibility.

### Error Handling Strategy

- **Assertions for programming errors** (invariant violations, null pointers, invalid state). Enabled in debug, disabled in release. Assertions are documentation that the condition should never happen.
- **Error codes or exceptions for runtime errors** (file not found, network timeout, GPU allocation failure). These are expected failure modes.
- **The project should pick one: error codes or exceptions.** Mixing is worse than either alone. For a game engine, error codes (or `Result<T, Error>` types) are more common because they have no hidden control flow and zero overhead when not failing. Exceptions are viable but must be used consistently, and all exception-unsafe code (which is most C++ code that wasn't written with exceptions in mind) must be audited.

**Recommendation:** No exceptions. Use return values (error codes, optional, or result types). Reserve `assert` for programmer errors.

### Floating Point Determinism

The project wants "reproducible simulation on the same platform/build." Achieving this requires:
- Consistent floating-point mode (no fast-math compiler flags that reorder operations).
- No uninitialized variables (optimizer can use whatever value is in the register).
- No data races (concurrent access to shared floats).
- Deterministic iteration order over containers (unordered_map iterates in hash order, which may vary between runs if using ASLR-based hash seeds).
- Identical compiler, optimization level, and target architecture.

This is achievable but requires discipline. Cross-platform determinism (same result on Windows/macOS/Linux or AMD/Intel) is NOT achievable with standard IEEE 754 hardware — different FMA (fused multiply-add) availability alone changes results.

### Scope Management

This is a learning project with the scope of a commercial engine. Realistic assessment:

| System | Estimated Complexity | Risk |
|--------|---------------------|------|
| Rendering (basic PBR) | High | Raw Vulkan is verbose but well-documented |
| Physics | Medium | Jolt does the heavy lifting |
| Editor (basic) | High | ImGui helps, but undo/redo/PIE are complex |
| Asset Pipeline | High | Often underestimated; touches everything |
| Hot Reload | High | ABI boundary is the constraint |
| Networking | Very High | Touches every system, hard to test |
| Ray Tracing | High | Optional, can defer |
| GPU Particles | Medium | Self-contained system |
| Animation | Medium-High | State machines and root motion are the hard parts |
| Runtime UI | Medium | Scope carefully |

The engine will teach the most if you build each system to a working state before moving to the next. Resist the urge to build everything to production quality — "working and understood" is the goal for a learning project.
