## Tasks

### Phase 1: Minimal Setup

#### T1 - CMake Project Structure
- [x] CMake project with engine, editor, and game module targets
- [x] Compiler warning policy and code formatting rules

#### T2 - Logging and Error Handling
- [x] spdlog integration with structured logging
- [x] Assertions and crash handling

#### T2b - Crash Handling
- [x] Unhandled exception filter and structured crash handler
- [x] Minidump generation on crash
- [x] Ring buffer of recent log messages captured in crash reports

#### T3 - Unit Testing
- [x] Test framework setup
- [x] First tests (logging, assertions, game loop timing)

#### T4 - Memory Diagnostics
- [ ] Debug leak checks and allocation counters

#### T5 - Window and Input
- [ ] GLFW window creation and basic input loop

#### T6 - Graphics Initialization
- [ ] Vulkan instance, device, and queue setup with validation layers
- [ ] VMA initialization
- [ ] Swapchain setup and clear screen rendering (proof of life)

#### T7 - Game Loop
- [ ] Fixed timestep update loop
- [ ] Simulation/render separation

### Phase 2: First Triangle

#### T8 - Shader Compilation
- [ ] GLSL compilation to SPIR-V via glslc/shaderc

#### T9 - GPU Resource Management
- [ ] GPU upload path and staging buffers
- [ ] Resource lifetime management

#### T10 - Descriptor Binding
- [ ] Descriptor/resource binding model

#### T11 - Frame Synchronization
- [ ] Frame allocators and GPU synchronization
- [ ] Swapchain resize handling

#### T12 - Triangle Rendering
- [ ] Vertex and index buffer setup
- [ ] Triangle on screen

### Phase 3: 3D Foundations

#### T13 - Game Object Model
- [ ] Base game object class with component ownership
- [ ] Object lifecycle management (create, initialize, destroy)

#### T14 - Transforms and Scene Graph
- [ ] Transform, mesh, camera, and light components
- [ ] Parent-child transform hierarchy

#### T15 - Camera System
- [ ] Perspective projection
- [ ] Free-look camera controls

#### T16 - Test Mesh Loading
- [ ] Hardcoded mesh loading (OBJ or glTF) for visual feedback

#### T17 - Frustum Culling
- [ ] Frustum culling implementation
- [ ] Opaque/transparent draw sorting

#### T18 - Forward Lighting
- [ ] Directional light support
- [ ] Point light support (no shadows yet)

#### T19 - Skybox
- [ ] Skybox rendering

#### T20 - Vulkan Portability Check (Optional)
- [ ] Test on a second GPU vendor (AMD vs NVIDIA) to catch vendor-specific assumptions
- [ ] Note any portability issues for the abstraction layer phase

### Phase 4: Materials & Textures

#### T21 - 3D Model Import
- [ ] Assimp integration for static mesh loading

#### T22 - Texture Import
- [ ] Texture import and compression pipeline
- [ ] Material texture assignment

#### T23 - Material System
- [ ] Material asset system (textures, shader assignment, parameters)

#### T24 - GPU-Driven Rendering Decision
- [ ] Evaluate GPU-driven vs. CPU-driven draw submission for this engine's scope
- [ ] Document decision and implications for future rendering work

### Phase 5: Shadows & PBR

#### T25 - Render Pass Management
- [ ] Minimal pass/resource orchestration (render target creation, pass ordering, resource transitions)
- [ ] Enough structure to support shadow passes feeding into the main lighting pass

#### T26 - Shadow Mapping
- [ ] Shadow mapping (directional + point light shadows)

#### T27 - PBR Materials
- [ ] PBR material model (metallic-roughness workflow)

#### T28 - Image-Based Lighting
- [ ] IBL with environment maps

#### T29 - HDR Pipeline
- [ ] HDR rendering and tonemapping

### Phase 6: Physics

#### T30 - Physics Initialization
- [ ] Jolt Physics integration
- [ ] Rigidbody and collider components

#### T31 - Collision Configuration
- [ ] Collision layers and masks
- [ ] Physics <-> game object transform ownership/sync policy

#### T32 - Physics Timestep
- [ ] Fixed timestep physics integration with optional sub-stepping

#### T33 - Physics Queries
- [ ] Raycasts, sweeps, and overlap queries
- [ ] Gameplay query API

#### T34 - Physics Debug and Callbacks
- [ ] Physics debug visualization
- [ ] Collision callbacks/events to gameplay code

#### T35 - Physics Reproducibility
- [ ] Reproducibility tests for debugging and networking validation

### Phase 7: Scene & Serialization

#### T36 - References and Lifecycle
- [ ] Stable object and asset references for saved scenes
- [ ] Runtime bootstrap/shutdown lifecycle for engine, editor, and game modules

#### T37 - Scene Serialization
- [ ] Scene serialization to/from JSON

### Phase 8: Editor Shell

#### T38 - Input Abstraction
- [ ] Input action mapping above GLFW/platform layer
- [ ] Text input separation and input buffering

#### T39 - Offscreen Rendering
- [ ] Viewport rendering to an offscreen framebuffer

#### T40 - ImGui Integration
- [ ] Dear ImGui (docking branch) setup and rendering

#### T41 - Panel Layout
- [ ] Dockable panel layout framework
- [ ] Layout persistence (save/restore panel arrangement)

#### T42 - Hierarchy Panel
- [ ] Game object tree display and interaction

#### T43 - Inspector Panel
- [ ] Component property editing (hardcoded initially, reflection-driven after T68)

#### T44 - Content Browser
- [ ] Basic asset browser

#### T45 - Selection and Picking
- [ ] Object selection and viewport picking

#### T46 - Editor Camera
- [ ] Editor-specific camera controls

#### T47 - Editor State Separation
- [ ] Clear separation between editor-only state and runtime/game state

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

#### T53 - Prefabs
- [ ] Prefab/prototype workflow for reusable object setups

#### T54 - Play-in-Editor
- [ ] Enter/exit play mode with world snapshot/restore
- [ ] Runtime-spawned object handling and state restoration after PIE

### Phase 10: Runtime Core

#### T55 - Render Graph
- [ ] Basic render graph / pass dependency system

#### T56 - GPU Debug Tools
- [ ] RenderDoc/debug markers
- [ ] GPU profiling hooks

#### T57 - Profiling
- [ ] Profiling hooks and basic telemetry

#### T58 - Task System
- [ ] Thread pool for background work (asset IO, shader compilation)
- [ ] Job submission and completion API

#### T58b - Precompiled Headers
- [ ] Precompiled header setup for engine, editor, and game targets
- [ ] Common heavy headers (spdlog, STL containers, GLM, Vulkan) in PCH

### Phase 11: Asset Pipeline

#### T59 - Virtual Filesystem
- [ ] Path abstraction for editor and packaged builds

#### T60 - Asset Database
- [ ] Asset database with stable IDs
- [ ] Import settings and metadata files per asset

#### T61 - Asset Folder Structure
- [ ] Source-versus-cooked folder structure
- [ ] Importer versioning

#### T62 - Binary Serialization
- [ ] Binary format for mesh and cooked data (JSON reserved for project/metadata)

#### T63 - Dependency Tracking
- [ ] Cooked asset outputs and dependency tracking

#### T64 - Async Asset Loading
- [ ] Background thread loading with controlled main-thread handoff

#### T65 - Asset Hot Reload
- [ ] Asset reimport and hot reload

### Phase 12: Gameplay Modules

#### T66 - Module Interface
- [ ] C++ gameplay module/plugin interface with stable boundary

#### T67 - Hot Reload
- [ ] Desktop hot reload for editor/dev builds

#### T68 - Reflection and Ownership
- [ ] Component registration/reflection for serialization and editor exposure
- [ ] Ownership rules and versioning strategy between engine, editor, and gameplay modules

### Phase 13: Animation

#### T69 - Skeletal Mesh
- [ ] Skeletal mesh import and skinning

#### T70 - Animation Playback
- [ ] Animation clip playback

#### T71 - Animation Systems
- [ ] Animation state machine and blend tree
- [ ] Animation events/notifies for gameplay hooks
- [ ] Root motion policy and gameplay ownership rules

### Phase 14: Audio & Runtime UI

#### T72 - Font Rendering
- [ ] Font rasterization (stb_truetype)
- [ ] In-engine text display (debug overlay, UI labels)

#### T73 - Audio Playback
- [ ] raudio integration (load, play, stream sounds)

#### T74 - Audio Components
- [ ] Spatial audio, buses/submixes, and listener management

#### T75 - Runtime UI
- [ ] Gameplay UI system (menus, HUD, widgets)
- [ ] UI input focus and integration with gameplay input

### Phase 15: Networking

#### T76 - Authority Model
- [ ] Define client/server authority model
- [ ] Define replicated object identity and ownership rules

#### T77 - Replication Design
- [ ] Define serialization rules for replication
- [ ] Choose prediction/reconciliation scope (full movement, selected actors, or hybrid)

#### T78 - Network Protocol and Documentation
- [ ] Validate object model, physics, and gameplay assumptions against representative scenarios
- [ ] Lock reliable/unreliable channel usage and message categories
- [ ] Define divergence detection and debug tooling requirements
- [ ] Document simulation guarantees networking depends on

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

### Phase 16: Rendering Performance

#### T84 - Draw Call Optimization
- [ ] Batching and instancing

#### T85 - LOD
- [ ] LOD support

#### T86 - Post-Processing
- [ ] Basic post-processing stack

#### T87 - Occlusion Culling
- [ ] Occlusion culling
- [ ] Re-evaluate GPU-driven rendering direction based on profiling

### Phase 17: Abstraction Layer & Multi-Backend

#### T88 - Graphics Abstraction Layer
- [ ] Extract a thin renderer interface from the working Vulkan code
- [ ] Define backend-agnostic resource types (buffers, textures, pipelines, render passes)
- [ ] Vulkan backend implements the interface (refactor, not rewrite)

#### T89 - D3D12 Backend
- [ ] D3D12 backend implementation
- [ ] HLSL compilation via DXC (DXIL output) or cross-compile GLSL via DXC/SPIRV-Cross
- [ ] Rendering validated on D3D12

#### T90 - Metal Backend (Optional)
- [ ] Metal backend implementation
- [ ] SPIRV-Cross for MSL output
- [ ] Rendering validated on Metal

### Phase 18: Ray Tracing

#### T91 - Acceleration Structures
- [ ] BLAS/TLAS building (Vulkan RT)

#### T92 - RT Shadows
- [ ] Ray-traced shadows or ambient occlusion

#### T93 - RT Reflections
- [ ] Ray-traced reflections
- [ ] Temporal denoiser for low-sample RT output

#### T94 - Hybrid Pipeline
- [ ] Hybrid raster + RT pipeline
- [ ] Fallback path for unsupported hardware

### Phase 19: VFX & Shipping

#### T95 - GPU Particles
- [ ] GPU particle system (emission, simulation, rendering)

#### T96 - Build Pipeline
- [ ] Cooked build pipeline (asset archives, executable bundling)

#### T97 - Packaging
- [ ] Game distribution packaging
- [ ] Release config, save-data paths, and platform packaging checks
