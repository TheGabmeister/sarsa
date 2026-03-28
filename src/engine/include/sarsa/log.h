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

// Engine log macros
#define SR_TRACE(...)    ::sarsa::Log::engine_logger()->trace(__VA_ARGS__)
#define SR_DEBUG(...)    ::sarsa::Log::engine_logger()->debug(__VA_ARGS__)
#define SR_INFO(...)     ::sarsa::Log::engine_logger()->info(__VA_ARGS__)
#define SR_WARN(...)     ::sarsa::Log::engine_logger()->warn(__VA_ARGS__)
#define SR_ERROR(...)    ::sarsa::Log::engine_logger()->error(__VA_ARGS__)
#define SR_CRITICAL(...) ::sarsa::Log::engine_logger()->critical(__VA_ARGS__)

// Game log macros (used by gameplay DLL)
#define GAME_TRACE(...)    ::sarsa::Log::game_logger()->trace(__VA_ARGS__)
#define GAME_DEBUG(...)    ::sarsa::Log::game_logger()->debug(__VA_ARGS__)
#define GAME_INFO(...)     ::sarsa::Log::game_logger()->info(__VA_ARGS__)
#define GAME_WARN(...)     ::sarsa::Log::game_logger()->warn(__VA_ARGS__)
#define GAME_ERROR(...)    ::sarsa::Log::game_logger()->error(__VA_ARGS__)
#define GAME_CRITICAL(...) ::sarsa::Log::game_logger()->critical(__VA_ARGS__)
