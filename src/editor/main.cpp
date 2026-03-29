#include <sarsa/crash_handler.h>
#include <sarsa/engine.h>
#include <sarsa/log.h>
#include <sarsa/window.h>

int main() {
    sarsa::Log::init();
    sarsa::install_crash_handler();

    SR_LOG_ENGINE(info, "Sarsa Engine v{}.{}.{}",
                  sarsa::VERSION_MAJOR,
                  sarsa::VERSION_MINOR,
                  sarsa::VERSION_PATCH);

    auto window = sarsa::Window::create();
    if (!window) {
        SR_LOG_ENGINE(critical, "Failed to create window");
        return 1;
    }

    while (!window->should_close()) {
        window->poll_events();
    }

    SR_LOG_ENGINE(info, "Shutting down");
    return 0;
}
