#include <sarsa/log.h>

#include <spdlog/sinks/ringbuffer_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace sarsa {

static constexpr size_t RING_BUFFER_SIZE = 128;

static std::shared_ptr<spdlog::sinks::ringbuffer_sink_mt> s_ring_sink;

std::shared_ptr<spdlog::logger> Log::s_engine_logger;
std::shared_ptr<spdlog::logger> Log::s_game_logger;

void Log::init() {
    if (s_engine_logger) {
        return;
    }

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    s_ring_sink = std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(RING_BUFFER_SIZE);

    auto pattern = "%^[%T] [%n] %v%$";
    console_sink->set_pattern(pattern);
    s_ring_sink->set_pattern("[%T] [%n] [%l] %v");

    s_engine_logger = std::make_shared<spdlog::logger>(
        "SARSA", spdlog::sinks_init_list{console_sink, s_ring_sink});
    s_engine_logger->set_level(spdlog::level::trace);
    spdlog::register_logger(s_engine_logger);

    s_game_logger = std::make_shared<spdlog::logger>(
        "GAME", spdlog::sinks_init_list{console_sink, s_ring_sink});
    s_game_logger->set_level(spdlog::level::trace);
    spdlog::register_logger(s_game_logger);
}

std::vector<std::string> Log::recent_messages() {
    if (!s_ring_sink) {
        return {};
    }
    return s_ring_sink->last_formatted();
}

} // namespace sarsa
