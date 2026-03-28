#include <sarsa/assert.h>
#include <sarsa/crash_handler.h>
#include <sarsa/engine.h>
#include <sarsa/log.h>

int main() {
    sarsa::Log::init();
    sarsa::install_crash_handler();

    SR_LOG_ENGINE(info, "Sarsa Engine v{}.{}.{}",
                  sarsa::VERSION_MAJOR,
                  sarsa::VERSION_MINOR,
                  sarsa::VERSION_PATCH);
    SR_ENGINE_ASSERT(true, "This should not fire");

    // Deliberate crash to test crash handler — remove after testing
    SR_LOG_ENGINE(warn, "About to crash on purpose...");
    int* bad_ptr = nullptr;
    *bad_ptr = 42;

    return 0;
}
