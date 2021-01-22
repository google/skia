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
 * Efficiently allocates memory for IRNodes in an SkSL program. Optimized for allocate/release
 * performance over memory efficiency.
 *
 * All allocated IRNodes must be released back to the pool before it can be destroyed or recycled.
 */

class Pool {
public:
    ~Pool();

    // Creates a pool to store IRNodes during program creation. Call attachToThread() to start using
    // the pool for IRNode allocations. When your program is complete, call pool->detachFromThread()
    // to take ownership of the pool and its nodes. Before destroying any of the program's nodes,
    // make sure to reattach the pool by calling pool->attachToThread() again.
    static std::unique_ptr<Pool> Create();

    // Attaches a pool to the current thread.
    // It is an error to call this while a pool is already attached.
    void attachToThread();

    // Once you are done creating or destroying IRNodes in the pool, detach it from the thread.
    // It is an error to call this while no pool is attached.
    void detachFromThread();

    // Retrieves a node from the thread pool. If the pool is exhausted, or if the requested size
    // exceeds the size that we can deliver from a pool, this will just allocate memory.
    static void* AllocIRNode(size_t size);

    // Releases a node that was created by AllocIRNode. This will return it to the pool, or free it,
    // as appropriate. Make sure to free all nodes, since some of them may be real allocations.
    static void FreeIRNode(void* node_v);

private:
    void checkForLeaks();

    Pool() = default;  // use Create to make a pool
    std::unique_ptr<SkSL::MemoryPool> fMemPool;
};

}  // namespace SkSL

#endif
