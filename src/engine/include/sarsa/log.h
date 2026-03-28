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

// Engine log macros (safe to call before Log::init — silently no-ops)
#define SR_TRACE(...)    do { if (::sarsa::Log::engine_logger()) ::sarsa::Log::engine_logger()->trace(__VA_ARGS__); } while (false)
#define SR_DEBUG(...)    do { if (::sarsa::Log::engine_logger()) ::sarsa::Log::engine_logger()->debug(__VA_ARGS__); } while (false)
#define SR_INFO(...)     do { if (::sarsa::Log::engine_logger()) ::sarsa::Log::engine_logger()->info(__VA_ARGS__); } while (false)
#define SR_WARN(...)     do { if (::sarsa::Log::engine_logger()) ::sarsa::Log::engine_logger()->warn(__VA_ARGS__); } while (false)
#define SR_ERROR(...)    do { if (::sarsa::Log::engine_logger()) ::sarsa::Log::engine_logger()->error(__VA_ARGS__); } while (false)
#define SR_CRITICAL(...) do { if (::sarsa::Log::engine_logger()) ::sarsa::Log::engine_logger()->critical(__VA_ARGS__); } while (false)

// Game log macros (used by gameplay DLL)
#define GAME_TRACE(...)    do { if (::sarsa::Log::game_logger()) ::sarsa::Log::game_logger()->trace(__VA_ARGS__); } while (false)
#define GAME_DEBUG(...)    do { if (::sarsa::Log::game_logger()) ::sarsa::Log::game_logger()->debug(__VA_ARGS__); } while (false)
#define GAME_INFO(...)     do { if (::sarsa::Log::game_logger()) ::sarsa::Log::game_logger()->info(__VA_ARGS__); } while (false)
#define GAME_WARN(...)     do { if (::sarsa::Log::game_logger()) ::sarsa::Log::game_logger()->warn(__VA_ARGS__); } while (false)
#define GAME_ERROR(...)    do { if (::sarsa::Log::game_logger()) ::sarsa::Log::game_logger()->error(__VA_ARGS__); } while (false)
#define GAME_CRITICAL(...) do { if (::sarsa::Log::game_logger()) ::sarsa::Log::game_logger()->critical(__VA_ARGS__); } while (false)
