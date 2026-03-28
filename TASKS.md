# Sarsa Game Engine

## What
- a modern 3D PBR game engine, object-oriented, with multiplayer
- libraries: glfw, glm, spdlog, enet, The Forge, assimp, nlohmann/json, Dear ImGui, Jolt Physics, raudio, stb_image, stb_truetype, FreeType

## Why
- for learning

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

## Tasks

### Phase 1: Project Setup

#### T1 - CMake Project Structure
- [ ] CMake project with engine, editor, and game module targets
- [ ] Compiler warning policy and code formatting rules

#### T2 - Logging and Error Handling
- [ ] spdlog integration with structured logging
- [ ] Assertions and crash handling

#### T3 - Memory Diagnostics and Profiling
- [ ] Debug leak checks and allocation counters
- [ ] Profiling hooks and basic telemetry

#### T4 - Window and Input
- [ ] GLFW window creation and basic input loop

#### T5 - Graphics Initialization
- [ ] The Forge initialization and validation layers
- [ ] Swapchain setup and clear screen rendering (proof of life)

#### T6 - Game Object Model
- [ ] Base game object class with component ownership
- [ ] Object lifecycle management (create, initialize, destroy)

#### T7 - Game Loop
- [ ] Fixed timestep update loop
- [ ] Simulation/render separation

#### T8 - Unit Testing
- [ ] Test framework setup
- [ ] First tests (serialization round-trips, game object basics)

### Phase 2: First Triangle

#### T9 - Shader Compilation
- [ ] HLSL compilation via DXC (SPIR-V for Vulkan, DXIL for D3D12)
- [ ] SPIRV-Cross for MSL (Metal) output

#### T10 - GPU Resource Management
- [ ] GPU upload path and staging buffers
- [ ] Resource lifetime management

#### T11 - Descriptor Binding
- [ ] Descriptor/resource binding model

#### T12 - Frame Synchronization
- [ ] Frame allocators and GPU synchronization
- [ ] Swapchain resize handling

#### T13 - Triangle Rendering
- [ ] Vertex and index buffer setup
- [ ] Triangle on screen

#### T14 - Offscreen Rendering
- [ ] Viewport rendering to an offscreen framebuffer

#### T15 - Multi-Backend Validation
- [ ] Backend capability detection and fallback table
- [ ] Triangle validated on Vulkan, D3D12, and Metal

### Phase 3: 3D Foundations

#### T22 - Transforms and Scene Graph
- [ ] Transform, mesh, camera, and light components
- [ ] Parent-child transform hierarchy

#### T23 - Camera System
- [ ] Perspective projection
- [ ] Free-look camera controls

#### T18 - Test Mesh Loading
- [ ] Hardcoded mesh loading (OBJ or glTF) for visual feedback

#### T19 - Frustum Culling
- [ ] Frustum culling implementation
- [ ] Opaque/transparent draw sorting

#### T20 - Forward Lighting
- [ ] Directional light support
- [ ] Point light support (no shadows yet)

#### T21 - Skybox
- [ ] Skybox rendering

### Phase 4: Runtime Core

#### T16 - Render Graph
- [ ] Basic render graph / pass dependency system

#### T17 - GPU Debug Tools
- [ ] RenderDoc/debug markers
- [ ] GPU profiling hooks

#### T24 - Input Abstraction
- [ ] Input action mapping above GLFW/platform layer
- [ ] Text input separation and input buffering

#### T25 - References and Lifecycle
- [ ] Stable object and asset references for saved scenes
- [ ] Runtime bootstrap/shutdown lifecycle for engine, editor, and game modules

#### T91 - Task System
- [ ] Thread pool for background work (asset IO, shader compilation)
- [ ] Job submission and completion API

### Phase 5: Editor Shell

#### T26 - ImGui Integration
- [ ] Dear ImGui (docking branch) setup and rendering

#### T27 - Panel Layout
- [ ] Dockable panel layout framework

#### T28 - Hierarchy Panel
- [ ] Game object tree display and interaction

#### T29 - Inspector Panel
- [ ] Component property editing (hardcoded initially, reflection-driven after T36)

#### T30 - Content Browser
- [ ] Basic asset browser

#### T31 - Selection and Picking
- [ ] Object selection and viewport picking

#### T32 - Editor Camera
- [ ] Editor-specific camera controls
- [ ] Layout persistence (save/restore panel arrangement)

#### T33 - Editor State Separation
- [ ] Clear separation between editor-only state and runtime/game state

### Phase 6: Gameplay Modules

#### T34 - Module Interface
- [ ] C++ gameplay module/plugin interface with stable boundary

#### T35 - Hot Reload
- [ ] Desktop hot reload for editor/dev builds

#### T36 - Reflection and Ownership
- [ ] Component registration/reflection for serialization and editor exposure
- [ ] Ownership rules and versioning strategy between engine, editor, and gameplay modules

### Phase 7: Asset Database

#### T37 - Asset Database
- [ ] Asset database with stable IDs
- [ ] Import settings and metadata files per asset

#### T38 - Asset Folder Structure
- [ ] Source-versus-cooked folder structure
- [ ] Importer versioning

#### T39 - Binary Serialization
- [ ] Binary format for mesh and cooked data (JSON reserved for project/metadata)

#### T40 - Dependency Tracking
- [ ] Cooked asset outputs and dependency tracking

#### T41 - Virtual Filesystem
- [ ] Path abstraction for editor and packaged builds

### Phase 8: Asset Import and Loading

#### T42 - 3D Model Import
- [ ] Assimp integration for static mesh loading

#### T43 - Texture Import
- [ ] Texture import and compression pipeline
- [ ] Material texture assignment

#### T44 - Material System
- [ ] Material asset system (textures, shader assignment, parameters)

#### T45 - Async Asset Loading
- [ ] Background thread loading with controlled main-thread handoff

#### T46 - Scene Serialization
- [ ] Scene serialization to/from JSON

#### T47 - Asset Hot Reload
- [ ] Asset reimport and hot reload

### Phase 9: Editor Authoring

#### T48 - Scene Save/Load
- [ ] Scene save/load from editor

#### T49 - Gizmos
- [ ] Translate, rotate, scale gizmos

#### T50 - Undo/Redo
- [ ] Undo/redo transaction model for editor operations

#### T51 - Object Workflows
- [ ] Selection, rename, duplicate, and delete with consistent IDs/references

#### T52 - Drag-and-Drop and Asset Import
- [ ] Drag-and-drop assignment for assets/materials
- [ ] Asset import/reimport from editor UI

### Phase 10: Prefabs and Play Mode

#### T53 - Prefabs
- [ ] Prefab/prototype workflow for reusable object setups

#### T54 - Play-in-Editor
- [ ] Enter/exit play mode with world snapshot/restore
- [ ] Runtime-spawned object handling and state restoration after PIE

### Phase 11: Physics

#### T55 - Physics Initialization
- [ ] Jolt Physics integration
- [ ] Rigidbody and collider components

#### T56 - Collision Configuration
- [ ] Collision layers and masks
- [ ] Physics <-> game object transform ownership/sync policy

#### T57 - Physics Timestep
- [ ] Fixed timestep physics integration with optional sub-stepping

#### T58 - Physics Queries
- [ ] Raycasts, sweeps, and overlap queries
- [ ] Gameplay query API

#### T59 - Physics Debug and Callbacks
- [ ] Physics debug visualization
- [ ] Collision callbacks/events to gameplay code

#### T60 - Physics Reproducibility
- [ ] Reproducibility tests for debugging and networking validation

### Phase 12: Networking Design

#### T61 - Authority Model
- [ ] Define client/server authority model
- [ ] Define replicated object identity and ownership rules

#### T62 - Replication Design
- [ ] Define serialization rules for replication
- [ ] Choose prediction/reconciliation scope (full movement, selected actors, or hybrid)

#### T63 - Network Protocol and Documentation
- [ ] Validate object model, physics, and gameplay assumptions against representative scenarios
- [ ] Lock reliable/unreliable channel usage and message categories
- [ ] Define divergence detection and debug tooling requirements
- [ ] Document simulation guarantees networking depends on

### Phase 13: Text, Audio, and Runtime UI

#### T64 - Font Rendering
- [ ] Font rasterization (stb_truetype / FreeType)
- [ ] In-engine text display (debug overlay, UI labels)

#### T65 - Audio Playback
- [ ] raudio integration (load, play, stream sounds)

#### T66 - Audio Components
- [ ] Spatial audio, buses/submixes, and listener management

#### T67 - Runtime UI
- [ ] Gameplay UI system (menus, HUD, widgets)
- [ ] UI input focus and integration with gameplay input

### Phase 14: Animation

#### T68 - Skeletal Mesh
- [ ] Skeletal mesh import and skinning

#### T69 - Animation Playback
- [ ] Animation clip playback

#### T70 - Animation Systems
- [ ] Animation state machine and blend tree
- [ ] Animation events/notifies for gameplay hooks
- [ ] Root motion policy and gameplay ownership rules

### Phase 15: PBR and Lighting

#### T71 - PBR Materials
- [ ] PBR material model (metallic-roughness workflow)

#### T72 - Image-Based Lighting
- [ ] IBL with environment maps

#### T73 - Shadow Mapping
- [ ] Shadow mapping (directional + point light shadows)

#### T74 - HDR Pipeline
- [ ] HDR rendering and tonemapping

### Phase 16: Rendering Performance

#### T75 - Draw Call Optimization
- [ ] Batching and instancing

#### T76 - LOD
- [ ] LOD support

#### T77 - Post-Processing
- [ ] Basic post-processing stack

#### T78 - Occlusion Culling
- [ ] Occlusion culling
- [ ] Re-evaluate GPU-driven rendering direction based on profiling

### Phase 17: Multiplayer

#### T79 - Network Transport
- [ ] ENet integration (client/server architecture)

#### T80 - Replication
- [ ] Replicated object spawn/despawn and state synchronization

#### T81 - Prediction and Interpolation
- [ ] Client-side prediction, interpolation, and reconciliation

#### T82 - Network Debugging
- [ ] Divergence detection, correction paths, and replay/debug tooling

#### T83 - Session Management
- [ ] Session/lobby flow and replication diagnostics

### Phase 18: Ray Tracing

#### T84 - Acceleration Structures
- [ ] BLAS/TLAS building

#### T85 - RT Shadows
- [ ] Ray-traced shadows or ambient occlusion

#### T86 - RT Reflections
- [ ] Ray-traced reflections
- [ ] Temporal denoiser for low-sample RT output

#### T87 - Hybrid Pipeline
- [ ] Hybrid raster + RT pipeline
- [ ] Fallback path for unsupported hardware/backends

### Phase 19: VFX and Shipping

#### T88 - GPU Particles
- [ ] GPU particle system (emission, simulation, rendering)

#### T89 - Build Pipeline
- [ ] Cooked build pipeline (asset archives, executable bundling)

#### T90 - Packaging
- [ ] Game distribution packaging
- [ ] Release config, save-data paths, and platform packaging checks
