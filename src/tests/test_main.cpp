#include "test_framework.h"

#include <algorithm>
#include <cstdio>
#include <utility>
#include <vector>

namespace sarsa::test {

namespace {

struct TestCase {
    std::string name;
    TestFunction function;
};

std::vector<TestCase>& registry() {
    static std::vector<TestCase> test_cases;
    return test_cases;
}

} // namespace

void Context::check(bool condition, const char* expression, const char* file, int line) {
    if (!condition) {
        fail(file, line, std::string("Check failed: ") + expression);
    }
}

void Context::fail(const char* file, int line, std::string message) {
    m_failures.push_back(Failure{
        .file = file,
        .line = line,
        .message = std::move(message),
    });
}

void register_test(const char* name, TestFunction function) {
    registry().push_back(TestCase{
        .name = name,
        .function = function,
    });
}

int run_all_tests() {
    auto& tests = registry();
    std::sort(
        tests.begin(),
        tests.end(),
        [](const TestCase& left, const TestCase& right) {
            return left.name < right.name;
        });

    int failed_test_count = 0;

    for (const auto& test_case : tests) {
        Context context;
        test_case.function(context);

        if (context.passed()) {
            std::printf("[PASS] %s\n", test_case.name.c_str());
            continue;
        }

        ++failed_test_count;
        std::printf("[FAIL] %s\n", test_case.name.c_str());
        for (const auto& failure : context.failures()) {
            std::printf("  %s:%d: %s\n",
                        failure.file.c_str(),
                        failure.line,
                        failure.message.c_str());
        }
    }

    std::printf("\n%d test(s), %d failure(s)\n",
                static_cast<int>(tests.size()),
                failed_test_count);

    return failed_test_count == 0 ? 0 : 1;
}

} // namespace sarsa::test

int main() {
    return sarsa::test::run_all_tests();
}
