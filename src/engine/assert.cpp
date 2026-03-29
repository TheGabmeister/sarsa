#include <sarsa/assert.h>

namespace sarsa {

namespace {

AssertFailureHandler s_assert_failure_handler = nullptr;

} // namespace

AssertFailureHandler set_assert_failure_handler(AssertFailureHandler handler) {
    AssertFailureHandler previous_handler = s_assert_failure_handler;
    s_assert_failure_handler = handler;
    return previous_handler;
}

namespace detail {

void report_assert_failure(
    AssertChannel channel,
    const char* condition,
    const char* file,
    int line,
    const std::string& message) {
    if (channel == AssertChannel::Engine) {
        if (Log::engine_logger()) {
            Log::engine_logger()->critical(
                "Assertion failed: {} ({}:{})",
                condition,
                file,
                line);
            Log::engine_logger()->critical("{}", message);
        }
    } else if (Log::game_logger()) {
        Log::game_logger()->critical(
            "Assertion failed: {} ({}:{})",
            condition,
            file,
            line);
        Log::game_logger()->critical("{}", message);
    }

    const AssertFailure failure{
        .channel = channel,
        .condition = condition,
        .file = file,
        .line = line,
        .message = message,
    };

    const AssertFailureAction action =
        s_assert_failure_handler ? s_assert_failure_handler(failure)
                                 : AssertFailureAction::Break;

    if (action == AssertFailureAction::Break) {
        SR_DEBUG_BREAK();
    }
}

} // namespace detail

} // namespace sarsa
