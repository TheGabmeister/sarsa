#pragma once

#include <cstddef>
#include <cstdint>

namespace sarsa {

struct MemorySnapshot {
    std::int64_t allocation_count = 0;
    std::int64_t total_bytes = 0;
};

class MemoryDiagnostics {
public:
    static std::int64_t allocation_count();
    static std::int64_t total_bytes();
    static std::int64_t peak_bytes();

    static MemorySnapshot snapshot();
    static MemorySnapshot diff(const MemorySnapshot& baseline);

    // Logs a leak report relative to baseline. Returns true if leaks found.
    static bool check_leaks(const MemorySnapshot& baseline);

    // Called by global new/delete overrides. Do not call directly.
    static void record_alloc(std::size_t size);
    static void record_free(std::size_t size);
};

} // namespace sarsa
