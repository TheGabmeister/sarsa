#pragma once

#include <string>
#include <vector>

namespace sarsa::test {

struct Failure {
    std::string file;
    int line;
    std::string message;
};

class Context {
public:
    void check(bool condition, const char* expression, const char* file, int line);
    void fail(const char* file, int line, std::string message);

    [[nodiscard]] bool passed() const { return m_failures.empty(); }
    [[nodiscard]] const std::vector<Failure>& failures() const { return m_failures; }

private:
    std::vector<Failure> m_failures;
};

using TestFunction = void (*)(Context& context);

void register_test(const char* name, TestFunction function);
int run_all_tests();

} // namespace sarsa::test

#define SR_TEST(name)                                                          \
    static void name(::sarsa::test::Context& context);                         \
    namespace {                                                                \
    struct name##_registrar {                                                  \
        name##_registrar() { ::sarsa::test::register_test(#name, &name); }     \
    };                                                                         \
    static const name##_registrar name##_registrar_instance{};                 \
    }                                                                          \
    static void name(::sarsa::test::Context& context)

#define SR_TEST_CHECK(condition) context.check((condition), #condition, __FILE__, __LINE__)
#define SR_TEST_FAIL(message) context.fail(__FILE__, __LINE__, (message))
