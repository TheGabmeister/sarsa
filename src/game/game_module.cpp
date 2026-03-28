#include <sarsa/game_module.h>

static void on_load() {}
static void on_unload() {}
static void on_update(float /*dt*/) {}

extern "C" SARSA_GAME_API SarsaModuleInterface sarsa_create_module() {
    SarsaModuleInterface module_interface{};
    module_interface.on_load = on_load;
    module_interface.on_unload = on_unload;
    module_interface.on_update = on_update;
    return module_interface;
}
