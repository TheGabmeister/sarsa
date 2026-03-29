#include <sarsa/memory_diagnostics.h>
#include <sarsa/log.h>

#include <atomic>
#include <cstdlib>
#include <new>

namespace sarsa {

namespace {

std::atomic<std::int64_t> s_allocation_count{0};
std::atomic<std::int64_t> s_total_bytes{0};
std::atomic<std::int64_t> s_peak_bytes{0};

} // namespace

void MemoryDiagnostics::record_alloc(std::size_t size) {
    s_allocation_count.fetch_add(1, std::memory_order_relaxed);
    auto current = s_total_bytes.fetch_add(
                       static_cast<std::int64_t>(size), std::memory_order_relaxed) +
                   static_cast<std::int64_t>(size);
    auto peak = s_peak_bytes.load(std::memory_order_relaxed);
    while (current > peak &&
           !s_peak_bytes.compare_exchange_weak(peak, current, std::memory_order_relaxed)) {
    }
}

void MemoryDiagnostics::record_free(std::size_t size) {
    s_allocation_count.fetch_sub(1, std::memory_order_relaxed);
    s_total_bytes.fetch_sub(static_cast<std::int64_t>(size), std::memory_order_relaxed);
}

std::int64_t MemoryDiagnostics::allocation_count() {
    return s_allocation_count.load(std::memory_order_relaxed);
}

std::int64_t MemoryDiagnostics::total_bytes() {
    return s_total_bytes.load(std::memory_order_relaxed);
}

std::int64_t MemoryDiagnostics::peak_bytes() {
    return s_peak_bytes.load(std::memory_order_relaxed);
}

MemorySnapshot MemoryDiagnostics::snapshot() {
    return {
        s_allocation_count.load(std::memory_order_relaxed),
        s_total_bytes.load(std::memory_order_relaxed),
    };
}

MemorySnapshot MemoryDiagnostics::diff(const MemorySnapshot& baseline) {
    auto current = snapshot();
    return {
        current.allocation_count - baseline.allocation_count,
        current.total_bytes - baseline.total_bytes,
    };
}

bool MemoryDiagnostics::check_leaks(const MemorySnapshot& baseline) {
    auto d = diff(baseline);
    if (d.allocation_count > 0 || d.total_bytes > 0) {
        SR_LOG_ENGINE(warn, "Memory leak detected: {} allocation(s), {} byte(s)",
                      d.allocation_count, d.total_bytes);
        return true;
    }
    SR_LOG_ENGINE(info, "No memory leaks detected (peak: {} bytes)", peak_bytes());
    return false;
}

} // namespace sarsa

// Global operator new/delete overrides for allocation tracking (debug only)
#ifndef NDEBUG

namespace {

static constexpr std::size_t ALLOC_HEADER_SIZE =
    (sizeof(std::size_t) + alignof(std::max_align_t) - 1) & ~(alignof(std::max_align_t) - 1);

void* tracked_alloc(std::size_t size) {
    void* ptr = std::malloc(ALLOC_HEADER_SIZE + size);
    if (!ptr) {
        std::abort();
    }
    *static_cast<std::size_t*>(ptr) = size;
    sarsa::MemoryDiagnostics::record_alloc(size);
    return static_cast<char*>(ptr) + ALLOC_HEADER_SIZE;
}

void* tracked_alloc_nothrow(std::size_t size) noexcept {
    void* ptr = std::malloc(ALLOC_HEADER_SIZE + size);
    if (!ptr) {
        return nullptr;
    }
    *static_cast<std::size_t*>(ptr) = size;
    sarsa::MemoryDiagnostics::record_alloc(size);
    return static_cast<char*>(ptr) + ALLOC_HEADER_SIZE;
}

void tracked_free(void* ptr) noexcept {
    if (!ptr) {
        return;
    }
    void* real = static_cast<char*>(ptr) - ALLOC_HEADER_SIZE;
    std::size_t size = *static_cast<std::size_t*>(real);
    sarsa::MemoryDiagnostics::record_free(size);
    std::free(real);
}

} // namespace

void* operator new(std::size_t size) { return tracked_alloc(size); }
void* operator new[](std::size_t size) { return tracked_alloc(size); }
void* operator new(std::size_t size, const std::nothrow_t&) noexcept {
    return tracked_alloc_nothrow(size);
}
void* operator new[](std::size_t size, const std::nothrow_t&) noexcept {
    return tracked_alloc_nothrow(size);
}

void operator delete(void* ptr) noexcept { tracked_free(ptr); }
void operator delete[](void* ptr) noexcept { tracked_free(ptr); }
void operator delete(void* ptr, std::size_t) noexcept { tracked_free(ptr); }
void operator delete[](void* ptr, std::size_t) noexcept { tracked_free(ptr); }
void operator delete(void* ptr, const std::nothrow_t&) noexcept { tracked_free(ptr); }
void operator delete[](void* ptr, const std::nothrow_t&) noexcept { tracked_free(ptr); }

#endif // NDEBUG
