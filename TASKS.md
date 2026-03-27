Sarsa Game Engine

### Build & Architecture
- CMake build system
- Runtime, editor, and game modules as separate binaries/libraries
- Entity Component System architecture (EnTT)
- C++ gameplay modules with desktop hot reload in editor/dev builds via a stable module boundary (avoid raw C++ ABI coupling)
- Fixed timestep runtime with clear simulation/render separation
- Reproducible simulation on the same platform/build for debugging and networking validation
- Memory tracking, allocator hooks, and targeted frame/linear allocators where profiling justifies them
- Task system / thread pool for background asset IO, compilation, and non-gameplay parallel work

### Platform & Diagnostics
- The Forge for Vulkan, D3D12, and Metal
- GLFW for windowing and input, unless The Forge's platform layer fully replaces it
- GLM for math
- spdlog for structured logging
- Validation layers, assertions, crash handling, profiling hooks, and GPU markers
- Automated testing (unit tests, serialization round-trip tests, render comparison tests)
- Backend capability matrix and fallback policy for optional features

### Rendering
- HLSL shaders compiled via DXC (SPIR-V for Vulkan, DXIL for D3D12) and SPIRV-Cross (MSL for Metal)
- Renderer resource system (buffers, textures, descriptors, transient frame allocators)
- Render graph / frame graph for pass dependency management
- Material and shader system with reflection/metadata
- Scene rendering fundamentals (frustum culling, draw sorting, instancing, depth pre-pass where beneficial)
- Basic forward lighting before full PBR (directional + point lights)
- Physically based rendering (PBR) pipeline
- Skybox and environment rendering
- LOD support and occlusion culling
- GPU particle / VFX system
- Hardware ray tracing (Vulkan RT, DXR, Metal RT) with capability checks, fallbacks, and denoising
- GPU-driven rendering direction decided before deep renderer investment

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
- Font rasterization and in-engine text (stb_truetype, FreeType)

### Runtime UI
- Gameplay UI system separate from the editor (menus, HUD, widgets)
- Explicit input focus ownership between gameplay, runtime UI, and editor UI

### Physics
- Rigid body simulation and collision detection (Jolt Physics)
- Collision layers/masks, raycasts, overlap queries, sweeps, and debug visualization
- Fixed timestep simulation with optional sub-stepping where required
- Defined transform ownership/sync rules between gameplay, ECS, and physics

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

## Tasks

### T1 - Project Foundation
- [ ] CMake project structure with engine, editor, and game module targets
- [ ] Compiler warning policy, formatting, CI, and automated build validation
- [ ] Logging, assertions, crash handling, profiling hooks, and basic telemetry
- [ ] Basic memory diagnostics (debug leak checks, allocation counters, profiler integration hooks)
- [ ] GLFW window creation and input loop, unless replaced by The Forge platform layer
- [ ] The Forge initialization, validation layers, and swapchain setup
- [ ] Clear screen rendering (proof of life)
- [ ] EnTT integration (entities, components, systems)
- [ ] Game loop with fixed timestep update and simulation/render separation
- [ ] Unit test framework setup and first tests (serialization, ECS basics)

### T2 - Renderer Bring-Up
- [ ] HLSL shader compilation pipeline (DXC + SPIRV-Cross)
- [ ] GPU upload path and staging/resource lifetime management
- [ ] Descriptor/resource binding model
- [ ] Frame allocators, synchronization, and resize/swapchain recreation
- [ ] Optional background worker for non-blocking shader compilation or tools tasks
- [ ] Triangle rendering with vertex/index buffers
- [ ] Viewport rendering to an offscreen framebuffer
- [ ] Backend capability detection and feature fallback table
- [ ] Triangle rendering validated on Vulkan, D3D12, and Metal

### T3 - Renderer Architecture
- [ ] Basic render graph / pass dependency system
- [ ] RenderDoc/debug markers and GPU profiling hooks
- [ ] Hardcoded mesh loading (OBJ or glTF) for visual development feedback
- [ ] Frustum culling and opaque/transparent draw sorting
- [ ] Basic forward lighting (directional + point lights, no shadows)
- [ ] Skybox rendering

### T4 - Runtime Core
- [ ] Transform, mesh, camera, and light components
- [ ] Scene graph (parent-child transforms)
- [ ] Input abstraction above GLFW/platform layer (action mapping, text input separation, buffering)
- [ ] Stable entity and asset references for saved scenes
- [ ] Runtime bootstrap/shutdown lifecycle for engine, editor, and game modules
- [ ] Camera system (perspective projection, free-look controls)

### T5 - Editor Shell
- [ ] Dear ImGui (docking branch) integration
- [ ] Dockable panel layout (viewport, hierarchy, inspector, content browser)
- [ ] Entity selection and viewport picking
- [ ] Inspector panel (component property editing)
- [ ] Hierarchy panel
- [ ] Basic content browser / asset browser
- [ ] Layout persistence
- [ ] Editor camera controls
- [ ] Clear separation between editor-only state and runtime/game state

### T6 - Gameplay Module Boundary
- [ ] C++ gameplay module/plugin interface with stable boundary and desktop hot reload for editor/dev builds
- [ ] Component registration/reflection metadata for serialization and editor exposure
- [ ] Clear ownership rules between engine, editor, and gameplay modules
- [ ] Versioning strategy for engine-to-game module compatibility

### T7 - Asset Database and Cooking
- [ ] Asset database with stable IDs
- [ ] Import settings and metadata files per asset
- [ ] Source-versus-cooked asset folder structure and importer versioning
- [ ] Binary serialization format for mesh and cooked data (JSON for project/metadata only)
- [ ] Cooked asset outputs and dependency tracking
- [ ] Virtual filesystem/path abstraction for editor and packaged builds

### T8 - Asset Import and Runtime Loading
- [ ] 3D model loading via Assimp (static meshes first)
- [ ] Texture import, compression pipeline, and material texture assignment
- [ ] Material asset system (textures, shader assignment, parameters)
- [ ] Async asset loading (background thread, controlled main-thread handoff)
- [ ] Scene serialization to/from JSON
- [ ] Asset reimport and hot reload

### T9 - Editor Authoring Workflows
- [ ] Scene save/load from editor
- [ ] Gizmos (translate, rotate, scale)
- [ ] Undo/redo transaction model for editor operations
- [ ] Selection, rename, duplicate, and delete workflows with consistent IDs/references
- [ ] Drag-and-drop assignment for assets/materials
- [ ] Asset import/reimport from editor

### T10 - Prefabs and Play-In-Editor
- [ ] Prefab/prototype workflow for reusable entity setups
- [ ] Play-in-Editor (PIE): enter/exit play mode with world snapshot/restore
- [ ] Explicit handling for runtime-spawned objects and editor state restoration after PIE

### T11 - Physics Integration
- [ ] Jolt Physics integration
- [ ] Rigidbody and collider components
- [ ] Collision layers and masks
- [ ] Physics <-> ECS transform ownership/sync policy
- [ ] Fixed timestep physics integration with optional sub-stepping
- [ ] Raycasts, sweeps, overlap queries, and gameplay query API
- [ ] Physics debug visualization
- [ ] Collision callbacks/events to gameplay code
- [ ] Reproducibility tests on the same platform/build for debugging and networking validation

### T12 - Networking Architecture Spike
- [ ] Define client/server authority model
- [ ] Define replicated entity identity and ownership rules
- [ ] Define component serialization rules for replication
- [ ] Choose prediction/reconciliation scope (full movement, selected actors, or hybrid)
- [ ] Validate ECS, physics, and gameplay assumptions against representative scenarios
- [ ] Lock reliable/unreliable channel usage and message categories
- [ ] Define divergence detection and debug tooling requirements
- [ ] Document what simulation guarantees networking depends on, and what it does not

### T13 - Text, Audio, and Runtime UI
- [ ] Font rendering (stb_truetype / FreeType)
- [ ] In-engine text display (debug overlay, UI labels)
- [ ] miniaudio integration (load, play, and stream sounds)
- [ ] Audio components (spatial audio, buses/submixes, listener management)
- [ ] Runtime gameplay UI system (menus, HUD, widgets)
- [ ] Runtime UI input focus and integration with gameplay input

### T14 - Animation
- [ ] Skeletal mesh import and skinning
- [ ] Animation clip playback
- [ ] Animation state machine and blend tree support
- [ ] Animation events/notifies for gameplay hooks
- [ ] Root motion policy and gameplay ownership rules

### T15 - PBR and Lighting
- [ ] PBR material model (metallic-roughness workflow)
- [ ] Image-based lighting (IBL) with environment maps
- [ ] Shadow mapping (directional + point light shadows)
- [ ] HDR rendering and tonemapping

### T16 - Rendering Performance and Post
- [ ] Draw call batching and instancing optimizations
- [ ] LOD support
- [ ] Basic post-processing stack
- [ ] Occlusion culling
- [ ] Re-evaluate GPU-driven rendering direction based on actual bottlenecks

### T17 - Multiplayer Implementation
- [ ] ENet integration (client/server architecture)
- [ ] Replicated entity spawn/despawn and state synchronization
- [ ] Client-side prediction, interpolation, and reconciliation
- [ ] Divergence detection, correction paths, and replay/debug tooling
- [ ] Session/lobby flow
- [ ] Multiplayer debugging and replication diagnostics

### T18 - Ray Tracing
- [ ] Acceleration structure building (BLAS/TLAS)
- [ ] Ray-traced shadows or ambient occlusion
- [ ] Ray-traced reflections
- [ ] Temporal denoiser for low-sample RT output
- [ ] Hybrid raster + RT pipeline
- [ ] Fallback path for unsupported hardware/backends

### T19 - VFX and Shipping
- [ ] GPU particle system (emission, simulation, rendering)
- [ ] Cooked build pipeline (asset archives, executable bundling)
- [ ] Game distribution packaging
- [ ] Release configuration, save-data paths, and platform-specific packaging checks
