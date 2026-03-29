#pragma once

#include <sarsa/log.h>

#include <spdlog/fmt/fmt.h>

#include <string>
#include <string_view>
#include <utility>

#ifdef _MSC_VER
    #define SR_DEBUG_BREAK() __debugbreak()
#else
    #include <csignal>
    #define SR_DEBUG_BREAK() raise(SIGTRAP)
#endif

namespace sarsa {

enum class AssertChannel {
    Engine,
    Game,
};

struct AssertFailure {
    AssertChannel channel;
    const char* condition;
    const char* file;
    int line;
    std::string_view message;
};

enum class AssertFailureAction {
    Break,
    Continue,
};

using AssertFailureHandler = AssertFailureAction (*)(const AssertFailure& failure);

AssertFailureHandler set_assert_failure_handler(AssertFailureHandler handler);

namespace detail {

void report_assert_failure(
    AssertChannel channel,
    const char* condition,
    const char* file,
    int line,
    const std::string& message);

template <typename... Args>
void report_assert_failure(
    AssertChannel channel,
    const char* condition,
    const char* file,
    int line,
    fmt::format_string<Args...> format,
    Args&&... args) {
    report_assert_failure(
        channel,
        condition,
        file,
        line,
        fmt::format(format, std::forward<Args>(args)...));
}

} // namespace detail

} // namespace sarsa

// Assertions - enabled in Debug, disabled in Release
#ifdef NDEBUG
    #define SR_ASSERT(condition, ...)
    #define SR_ASSERT_ENGINE(condition, ...)
#else
    #define SR_ASSERT(condition, ...)                                            \
        do {                                                                    \
            if (!(condition)) {                                                 \
                ::sarsa::detail::report_assert_failure(                         \
                    ::sarsa::AssertChannel::Game,                               \
                    #condition,                                                 \
                    __FILE__,                                                   \
                    __LINE__,                                                   \
                    __VA_ARGS__);                                               \
            }                                                                   \
        } while (false)

    #define SR_ASSERT_ENGINE(condition, ...)                                     \
        do {                                                                    \
            if (!(condition)) {                                                 \
                ::sarsa::detail::report_assert_failure(                         \
                    ::sarsa::AssertChannel::Engine,                             \
                    #condition,                                                 \
                    __FILE__,                                                   \
                    __LINE__,                                                   \
                    __VA_ARGS__);                                               \
            }                                                                   \
        } while (false)
#endif
