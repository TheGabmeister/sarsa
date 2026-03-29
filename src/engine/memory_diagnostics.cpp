#include <sarsa/memory_diagnostics.h>
#include <sarsa/log.h>

#include <atomic>
#include <cstdlib>
#include <new>

#ifdef _MSC_VER
    #include <malloc.h>
#endif

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

// Aligned allocation helpers (for over-aligned types via std::align_val_t)

std::size_t aligned_header_size(std::size_t alignment) {
    return (sizeof(std::size_t) + alignment - 1) & ~(alignment - 1);
}

void* platform_aligned_alloc(std::size_t size, std::size_t alignment) {
#ifdef _MSC_VER
    return _aligned_malloc(size, alignment);
#else
    std::size_t rounded = (size + alignment - 1) & ~(alignment - 1);
    return std::aligned_alloc(alignment, rounded);
#endif
}

void platform_aligned_free(void* ptr) {
#ifdef _MSC_VER
    _aligned_free(ptr);
#else
    std::free(ptr);
#endif
}

void* tracked_aligned_alloc(std::size_t size, std::align_val_t alignment) {
    auto align = static_cast<std::size_t>(alignment);
    auto header = aligned_header_size(align);
    void* ptr = platform_aligned_alloc(header + size, align);
    if (!ptr) {
        std::abort();
    }
    *static_cast<std::size_t*>(ptr) = size;
    sarsa::MemoryDiagnostics::record_alloc(size);
    return static_cast<char*>(ptr) + header;
}

void* tracked_aligned_alloc_nothrow(std::size_t size, std::align_val_t alignment) noexcept {
    auto align = static_cast<std::size_t>(alignment);
    auto header = aligned_header_size(align);
    void* ptr = platform_aligned_alloc(header + size, align);
    if (!ptr) {
        return nullptr;
    }
    *static_cast<std::size_t*>(ptr) = size;
    sarsa::MemoryDiagnostics::record_alloc(size);
    return static_cast<char*>(ptr) + header;
}

void tracked_aligned_free(void* ptr, std::align_val_t alignment) noexcept {
    if (!ptr) {
        return;
    }
    auto align = static_cast<std::size_t>(alignment);
    auto header = aligned_header_size(align);
    void* real = static_cast<char*>(ptr) - header;
    std::size_t size = *static_cast<std::size_t*>(real);
    sarsa::MemoryDiagnostics::record_free(size);
    platform_aligned_free(real);
}

} // namespace

// Regular new/delete
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

// Aligned new/delete (over-aligned types)
void* operator new(std::size_t size, std::align_val_t align) {
    return tracked_aligned_alloc(size, align);
}
void* operator new[](std::size_t size, std::align_val_t align) {
    return tracked_aligned_alloc(size, align);
}
void* operator new(std::size_t size, std::align_val_t align, const std::nothrow_t&) noexcept {
    return tracked_aligned_alloc_nothrow(size, align);
}
void* operator new[](std::size_t size, std::align_val_t align, const std::nothrow_t&) noexcept {
    return tracked_aligned_alloc_nothrow(size, align);
}

void operator delete(void* ptr, std::align_val_t align) noexcept {
    tracked_aligned_free(ptr, align);
}
void operator delete[](void* ptr, std::align_val_t align) noexcept {
    tracked_aligned_free(ptr, align);
}
void operator delete(void* ptr, std::size_t, std::align_val_t align) noexcept {
    tracked_aligned_free(ptr, align);
}
void operator delete[](void* ptr, std::size_t, std::align_val_t align) noexcept {
    tracked_aligned_free(ptr, align);
}
void operator delete(void* ptr, std::align_val_t align, const std::nothrow_t&) noexcept {
    tracked_aligned_free(ptr, align);
}
void operator delete[](void* ptr, std::align_val_t align, const std::nothrow_t&) noexcept {
    tracked_aligned_free(ptr, align);
}

#endif // NDEBUG
