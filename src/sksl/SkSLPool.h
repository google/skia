/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_POOL
#define SKSL_POOL

#include <cstddef>
#include <memory>

namespace SkSL {

class MemoryPool;

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
    // take ownership of the pool and its allocations. Before freeing any of the program's
    // allocations, make sure to reattach the pool by calling pool->attachToThread() again.
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
    static void FreeMemory(void* ptr);

    static bool IsAttached();

private:
    Pool();  // use Create to make a pool
    std::unique_ptr<SkSL::MemoryPool> fMemPool;
};

/**
 * If your class inherits from Poolable, its objects will be allocated from the pool.
 */
class Poolable {
public:
    // Override operator new and delete to allow us to use a memory pool.
    static void* operator new(const size_t size) {
        return Pool::AllocMemory(size);
    }

    static void operator delete(void* ptr) {
        Pool::FreeMemory(ptr);
    }
};

/**
 * Temporarily attaches a pool to the current thread within a scope.
 */
class AutoAttachPoolToThread {
public:
    AutoAttachPoolToThread(Pool* p) : fPool(p) {
        if (fPool) {
            fPool->attachToThread();
        }
    }
    ~AutoAttachPoolToThread() {
        if (fPool) {
            fPool->detachFromThread();
        }
    }

private:
    Pool* fPool = nullptr;
};


}  // namespace SkSL

#endif
