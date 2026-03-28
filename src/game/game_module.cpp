#include <sarsa/game_module.h>

static void on_load() {}
static void on_unload() {}
static void on_update(float /*dt*/) {}

extern "C" SARSA_GAME_API SarsaModuleInterface sarsa_create_module() {
    SarsaModuleInterface iface{};
    iface.on_load = on_load;
    iface.on_unload = on_unload;
    iface.on_update = on_update;
    return iface;
}
