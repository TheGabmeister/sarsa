#pragma once

// C-style ABI boundary for hot reload.
// No STL types, no vtables, no allocations crossing this boundary.

#ifdef _WIN32
    #define SARSA_GAME_API __declspec(dllexport)
#else
    #define SARSA_GAME_API __attribute__((visibility("default")))
#endif

extern "C" {

struct SarsaModuleInterface {
    void (*on_load)();
    void (*on_unload)();
    void (*on_update)(float dt);
};

SARSA_GAME_API SarsaModuleInterface sarsa_create_module();

} // extern "C"
