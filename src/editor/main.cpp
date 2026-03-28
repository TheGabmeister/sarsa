#include <sarsa/assert.h>
#include <sarsa/engine.h>
#include <sarsa/log.h>

int main() {
    sarsa::Log::init();

    SR_LOG_ENGINE(info, "Sarsa Engine v{}.{}.{}",
                  sarsa::VERSION_MAJOR,
                  sarsa::VERSION_MINOR,
                  sarsa::VERSION_PATCH);
    SR_ENGINE_ASSERT(true, "This should not fire");

    return 0;
}
