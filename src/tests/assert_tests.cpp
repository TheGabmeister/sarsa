#include "test_framework.h"

#include <sarsa/assert.h>
#include <sarsa/log.h>

#include <string>

namespace {

struct CapturedFailure {
    int count = 0;
    sarsa::AssertChannel channel = sarsa::AssertChannel::Game;
    std::string condition;
    std::string file;
    int line = 0;
    std::string message;
};

CapturedFailure* s_captured_failure = nullptr;

sarsa::AssertFailureAction capture_failure(const sarsa::AssertFailure& failure) {
    if (s_captured_failure != nullptr) {
        ++s_captured_failure->count;
        s_captured_failure->channel = failure.channel;
        s_captured_failure->condition = failure.condition;
        s_captured_failure->file = failure.file;
        s_captured_failure->line = failure.line;
        s_captured_failure->message = std::string(failure.message);
    }

    return sarsa::AssertFailureAction::Continue;
}

bool contains(const std::string& value, const std::string& needle) {
    return value.find(needle) != std::string::npos;
}

bool ends_with(const std::string& value, const std::string& suffix) {
    if (suffix.size() > value.size()) {
        return false;
    }

    return value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
}

} // namespace

SR_TEST(engine_assert_reports_metadata_without_breaking_test_process) {
    sarsa::Log::init();

    CapturedFailure captured_failure;
    s_captured_failure = &captured_failure;
    const auto previous_handler = sarsa::set_assert_failure_handler(&capture_failure);

    const int expected_line = __LINE__ + 1;
    SR_ASSERT_ENGINE(false, "engine assertion marker {}", 7);

    sarsa::set_assert_failure_handler(previous_handler);
    s_captured_failure = nullptr;

    SR_TEST_CHECK(captured_failure.count == 1);
    SR_TEST_CHECK(captured_failure.channel == sarsa::AssertChannel::Engine);
    SR_TEST_CHECK(captured_failure.condition == "false");
    SR_TEST_CHECK(ends_with(captured_failure.file, "assert_tests.cpp"));
    SR_TEST_CHECK(captured_failure.line == expected_line);
    SR_TEST_CHECK(captured_failure.message == "engine assertion marker 7");

    const auto messages = sarsa::Log::recent_messages();
    bool found_assert_log = false;

    for (const auto& message : messages) {
        found_assert_log = found_assert_log || contains(message, "engine assertion marker 7");
    }

    SR_TEST_CHECK(found_assert_log);
}
