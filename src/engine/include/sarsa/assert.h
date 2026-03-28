#pragma once

#include <sarsa/log.h>

#ifdef _MSC_VER
    #define SR_DEBUG_BREAK() __debugbreak()
#else
    #include <csignal>
    #define SR_DEBUG_BREAK() raise(SIGTRAP)
#endif

// Assertions - enabled in Debug, disabled in Release
#ifdef NDEBUG
    #define SR_ASSERT(condition, ...)
    #define SR_ENGINE_ASSERT(condition, ...)
#else
    #define SR_ASSERT(condition, ...)                                        \
        do {                                                                \
            if (!(condition)) {                                             \
                SR_LOG_GAME(critical, "Assertion failed: {} ({}:{})",      \
                       #condition, __FILE__, __LINE__);                     \
                SR_LOG_GAME(critical, __VA_ARGS__);                        \
                SR_DEBUG_BREAK();                                           \
            }                                                               \
        } while (false)

    #define SR_ENGINE_ASSERT(condition, ...)                                 \
        do {                                                                \
            if (!(condition)) {                                             \
                SR_LOG_ENGINE(critical, "Assertion failed: {} ({}:{})",    \
                       #condition, __FILE__, __LINE__);                     \
                SR_LOG_ENGINE(critical, __VA_ARGS__);                      \
                SR_DEBUG_BREAK();                                           \
            }                                                               \
        } while (false)
#endif
