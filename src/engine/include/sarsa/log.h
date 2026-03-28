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

// Internal helper — safe to call before Log::init (silently no-ops)
#define SR_LOG(logger, level, ...) \
    do { if (logger) (logger)->level(__VA_ARGS__); } while (false)

// Engine log macros
#define SR_TRACE(...)    SR_LOG(::sarsa::Log::engine_logger(), trace, __VA_ARGS__)
#define SR_DEBUG(...)    SR_LOG(::sarsa::Log::engine_logger(), debug, __VA_ARGS__)
#define SR_INFO(...)     SR_LOG(::sarsa::Log::engine_logger(), info, __VA_ARGS__)
#define SR_WARN(...)     SR_LOG(::sarsa::Log::engine_logger(), warn, __VA_ARGS__)
#define SR_ERROR(...)    SR_LOG(::sarsa::Log::engine_logger(), error, __VA_ARGS__)
#define SR_CRITICAL(...) SR_LOG(::sarsa::Log::engine_logger(), critical, __VA_ARGS__)

// Game log macros (used by gameplay DLL)
#define GAME_TRACE(...)    SR_LOG(::sarsa::Log::game_logger(), trace, __VA_ARGS__)
#define GAME_DEBUG(...)    SR_LOG(::sarsa::Log::game_logger(), debug, __VA_ARGS__)
#define GAME_INFO(...)     SR_LOG(::sarsa::Log::game_logger(), info, __VA_ARGS__)
#define GAME_WARN(...)     SR_LOG(::sarsa::Log::game_logger(), warn, __VA_ARGS__)
#define GAME_ERROR(...)    SR_LOG(::sarsa::Log::game_logger(), error, __VA_ARGS__)
#define GAME_CRITICAL(...) SR_LOG(::sarsa::Log::game_logger(), critical, __VA_ARGS__)
