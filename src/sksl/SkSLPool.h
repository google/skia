/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_POOL
#define SKSL_POOL

#include <memory>

#include "src/sksl/SkSLMemoryPool.h"

namespace SkSL {

/**
 * Efficiently allocates memory in an SkSL program. Optimized for allocate/release performance over
 * memory efficiency.
 *
 * All allocated memory must be released back to the pool before it can be destroyed or recycled.
 */

class Pool {
public:
    ~Pool();

    // Creates a pool to store objects during program creation. Call attachToThread() to start using
    // the pool for its allocations. When your program is complete, call pool->detachFromThread() to
    // take ownership of the pool and its nodes. Before destroying any of the program's nodes, make
    // sure to reattach the pool by calling pool->attachToThread() again.
    static std::unique_ptr<Pool> Create();

    // Attaches a pool to the current thread.
    // It is an error to call this while a pool is already attached.
    void attachToThread();

    // Once you are done creating or destroying objects in the pool, detach it from the thread.
    // It is an error to call this while no pool is attached.
    void detachFromThread();

    // Allocates memory from the thread pool. If the pool is exhausted, an additional block of pool
    // storage will be created to hold the data.
    static void* AllocMemory(size_t size);

    // Releases memory that was created by AllocMemory. All objects in the pool must be freed before
    // the pool can be destroyed.
    static void FreeMemory(void* node_v);

private:
    void checkForLeaks();

    Pool() = default;  // use Create to make a pool
    std::unique_ptr<SkSL::MemoryPool> fMemPool;
};

/**
 * PoolAllocator<T> can be used as an allocator for standard-library containers.
 */

template <class T>
struct PoolAllocator {
    using value_type = T;
    T* allocate(std::size_t n) {
        return static_cast<T*>(Pool::AllocMemory(n * sizeof(T)));
    }
    void deallocate(T* p, std::size_t n) {
        return Pool::FreeMemory(p);
    }
};

template <class T, class U>
constexpr bool operator==(const PoolAllocator<T>& a, const PoolAllocator<U>& b) {
    // Returns true if the allocators can free each other's memory. Pool::FreeMemory should always
    // be okay with this.
    return true;
}

template <class T, class U>
constexpr bool operator!=(const PoolAllocator<T>& a, const PoolAllocator<U>& b) {
    return !(a == b);
}

}  // namespace SkSL

#endif
