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

// SR_LOG_ENGINE(level, format, ...)
// SR_LOG_GAME(level, format, ...)
#define SR_LOG_ENGINE(level, ...) \
    do { if (::sarsa::Log::engine_logger()) ::sarsa::Log::engine_logger()->level(__VA_ARGS__); } while (false)
#define SR_LOG_GAME(level, ...) \
    do { if (::sarsa::Log::game_logger()) ::sarsa::Log::game_logger()->level(__VA_ARGS__); } while (false)
