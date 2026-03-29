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
    SR_ASSERT_ENGINE(true, "This should not fire");

    return 0;
}
