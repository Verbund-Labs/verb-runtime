#pragma once

#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <functional>
#include <iostream>

namespace verbrt {

// RAII Scope Guard for 'defer' support
class ScopeGuard {
public:
    explicit ScopeGuard(std::function<void()> fn) : fn_(std::move(fn)), active_(true) {}
    ~ScopeGuard() {
        if (active_ && fn_) {
            fn_();
        }
    }
    void dismiss() { active_ = false; }
    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;
    ScopeGuard(ScopeGuard&& other) noexcept : fn_(std::move(other.fn_)), active_(other.active_) {
        other.active_ = false;
    }

private:
    std::function<void()> fn_;
    bool active_;
};

#define VERB_CONCAT_INTERNAL(a, b) a##b
#define VERB_CONCAT(a, b) VERB_CONCAT_INTERNAL(a, b)
#define VERB_DEFER(code) ::verbrt::ScopeGuard VERB_CONCAT(_defer_guard_, __LINE__)([&]() { code; })

// Ultra-fast bump / arena allocator
class ArenaAllocator {
public:
    explicit ArenaAllocator(size_t default_chunk_size = 65536)
        : default_chunk_size_(default_chunk_size), current_chunk_(nullptr), current_offset_(0) {
        allocate_chunk(default_chunk_size_);
    }

    ~ArenaAllocator() {
        reset();
        for (void* chunk : chunks_) {
            std::free(chunk);
        }
    }

    void* allocate(size_t bytes, size_t alignment = alignof(std::max_align_t)) {
        size_t current_addr = reinterpret_cast<size_t>(static_cast<char*>(current_chunk_) + current_offset_);
        size_t aligned_addr = (current_addr + alignment - 1) & ~(alignment - 1);
        size_t padding = aligned_addr - current_addr;

        if (current_offset_ + padding + bytes <= chunk_capacity_) {
            current_offset_ += padding;
            void* ptr = static_cast<char*>(current_chunk_) + current_offset_;
            current_offset_ += bytes;
            total_allocated_ += bytes;
            return ptr;
        }

        size_t next_size = std::max(default_chunk_size_, bytes + alignment);
        allocate_chunk(next_size);
        return allocate(bytes, alignment);
    }

    void reset() {
        current_offset_ = 0;
        total_allocated_ = 0;
        if (!chunks_.empty()) {
            current_chunk_ = chunks_[0];
        }
    }

    size_t total_bytes_allocated() const { return total_allocated_; }

private:
    void allocate_chunk(size_t size) {
        void* mem = std::malloc(size);
        chunks_.push_back(mem);
        current_chunk_ = mem;
        chunk_capacity_ = size;
        current_offset_ = 0;
    }

    size_t default_chunk_size_;
    std::vector<void*> chunks_;
    void* current_chunk_;
    size_t current_offset_;
    size_t chunk_capacity_ = 0;
    size_t total_allocated_ = 0;
};

// C++ Raw Memory Manipulation APIs exposed to VerbLang
inline void* raw_alloc(int64_t bytes) {
    if (bytes <= 0) return nullptr;
    return std::malloc(static_cast<size_t>(bytes));
}

inline void raw_free(void* ptr) {
    if (ptr) {
        std::free(ptr);
    }
}

inline void raw_poke64(void* ptr, int64_t value) {
    if (ptr) {
        *reinterpret_cast<int64_t*>(ptr) = value;
    }
}

inline int64_t raw_peek64(const void* ptr) {
    if (!ptr) return 0;
    return *reinterpret_cast<const int64_t*>(ptr);
}

inline void raw_poke8(void* ptr, uint8_t value) {
    if (ptr) {
        *reinterpret_cast<uint8_t*>(ptr) = value;
    }
}

inline uint8_t raw_peek8(const void* ptr) {
    if (!ptr) return 0;
    return *reinterpret_cast<const uint8_t*>(ptr);
}

inline void raw_memcpy(void* dest, const void* src, int64_t bytes) {
    if (dest && src && bytes > 0) {
        std::memcpy(dest, src, static_cast<size_t>(bytes));
    }
}

} // namespace verbrt
