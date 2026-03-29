#include "test_framework.h"

#include <sarsa/log.h>

#include <string>

namespace {

bool contains(const std::string& value, const std::string& needle) {
    return value.find(needle) != std::string::npos;
}

} // namespace

SR_TEST(log_writes_recent_engine_and_game_messages) {
    sarsa::Log::init();

    const std::string engine_message = "log test engine marker";
    const std::string game_message = "log test game marker";

    SR_LOG_ENGINE(info, "{}", engine_message);
    SR_LOG_GAME(warn, "{}", game_message);

    const auto messages = sarsa::Log::recent_messages();

    bool found_engine = false;
    bool found_game = false;

    for (const auto& message : messages) {
        found_engine = found_engine ||
                       (contains(message, "SARSA") && contains(message, engine_message));
        found_game = found_game ||
                     (contains(message, "GAME") && contains(message, game_message));
    }

    SR_TEST_CHECK(found_engine);
    SR_TEST_CHECK(found_game);
}
