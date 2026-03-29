#include "test_framework.h"

#include <sarsa/memory_diagnostics.h>

// These tests rely on global new/delete overrides which are debug-only
#ifndef NDEBUG

SR_TEST(memory_diagnostics_tracks_allocation_count) {
    auto baseline = sarsa::MemoryDiagnostics::snapshot();

    auto* p = new int(42);
    auto d = sarsa::MemoryDiagnostics::diff(baseline);
    SR_TEST_CHECK(d.allocation_count >= 1);

    delete p;
    d = sarsa::MemoryDiagnostics::diff(baseline);
    SR_TEST_CHECK(d.allocation_count == 0);
}

SR_TEST(memory_diagnostics_tracks_bytes) {
    auto baseline = sarsa::MemoryDiagnostics::snapshot();

    auto* p = new int(42);
    auto d = sarsa::MemoryDiagnostics::diff(baseline);
    SR_TEST_CHECK(d.total_bytes == static_cast<std::int64_t>(sizeof(int)));

    delete p;
    d = sarsa::MemoryDiagnostics::diff(baseline);
    SR_TEST_CHECK(d.total_bytes == 0);
}

SR_TEST(memory_diagnostics_tracks_array_allocations) {
    auto baseline = sarsa::MemoryDiagnostics::snapshot();

    auto* arr = new int[10];
    auto d = sarsa::MemoryDiagnostics::diff(baseline);
    SR_TEST_CHECK(d.allocation_count >= 1);
    SR_TEST_CHECK(d.total_bytes >= static_cast<std::int64_t>(10 * sizeof(int)));

    delete[] arr;
    d = sarsa::MemoryDiagnostics::diff(baseline);
    SR_TEST_CHECK(d.allocation_count == 0);
    SR_TEST_CHECK(d.total_bytes == 0);
}

SR_TEST(memory_diagnostics_check_leaks_detects_leak) {
    auto baseline = sarsa::MemoryDiagnostics::snapshot();
    SR_TEST_CHECK(!sarsa::MemoryDiagnostics::check_leaks(baseline));

    auto* p = new int(42);
    SR_TEST_CHECK(sarsa::MemoryDiagnostics::check_leaks(baseline));

    delete p;
    SR_TEST_CHECK(!sarsa::MemoryDiagnostics::check_leaks(baseline));
}

SR_TEST(memory_diagnostics_peak_bytes_tracks_maximum) {
    auto before = sarsa::MemoryDiagnostics::snapshot();

    constexpr std::size_t BLOCK_SIZE = 1024 * 1024;
    auto* p = new char[BLOCK_SIZE];
    auto peak_during = sarsa::MemoryDiagnostics::peak_bytes();

    delete[] p;
    auto peak_after = sarsa::MemoryDiagnostics::peak_bytes();

    // Peak must have reached at least the pre-allocation total + the new block
    SR_TEST_CHECK(peak_during >= before.total_bytes + static_cast<std::int64_t>(BLOCK_SIZE));
    // Peak never decreases on free
    SR_TEST_CHECK(peak_after == peak_during);
}

#endif // NDEBUG
