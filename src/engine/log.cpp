#include <sarsa/log.h>

#include <spdlog/sinks/stdout_color_sinks.h>

namespace sarsa {

std::shared_ptr<spdlog::logger> Log::s_engine_logger;
std::shared_ptr<spdlog::logger> Log::s_game_logger;

void Log::init() {
    spdlog::set_pattern("%^[%T] [%n] %v%$");

    s_engine_logger = spdlog::stdout_color_mt("SARSA");
    s_engine_logger->set_level(spdlog::level::trace);

    s_game_logger = spdlog::stdout_color_mt("GAME");
    s_game_logger->set_level(spdlog::level::trace);
}

} // namespace sarsa
