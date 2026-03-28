#pragma once

#include <spdlog/spdlog.h>

#include <memory>

namespace sarsa {

class Log {
public:
    static void init();

    static std::shared_ptr<spdlog::logger>& engine_logger() { return s_engine_logger; }
    static std::shared_ptr<spdlog::logger>& game_logger() { return s_game_logger; }

private:
    static std::shared_ptr<spdlog::logger> s_engine_logger;
    static std::shared_ptr<spdlog::logger> s_game_logger;
};

} // namespace sarsa

// Log category accessors — maps category name to logger
#define SR_LOG_CAT_Engine ::sarsa::Log::engine_logger()
#define SR_LOG_CAT_Game   ::sarsa::Log::game_logger()

// Single logging macro: SR_LOG(Category, level, format, ...)
// Usage: SR_LOG(Engine, info, "started v{}", version);
//        SR_LOG(Game, warn, "player {} disconnected", id);
#define SR_LOG(category, level, ...) \
    do { if (SR_LOG_CAT_##category) (SR_LOG_CAT_##category)->level(__VA_ARGS__); } while (false)
