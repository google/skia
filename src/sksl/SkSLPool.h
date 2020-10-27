/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_POOL
#define SKSL_POOL

#include <memory>

namespace SkSL {

class IRNode;
struct PoolData;

class Pool {
public:
    ~Pool();

    // Creates a pool to store IRNodes during program creation. Call attachToThread() to start using
    // the pool for IRNode allocations. When your program is complete, call pool->detachFromThread()
    // to take ownership of the pool and its nodes. Before destroying any of the program's nodes,
    // make sure to reattach the pool by calling pool->attachToThread() again.
    static std::unique_ptr<Pool> Create();

    // Gives up ownership of a pool; conceptually, this deletes it. In practice, on some platforms,
    // it is expensive to free and reallocate pools, so this gives us an opportunity to reuse the
    // allocation for future CreatePoolOnThread calls.
    static void Recycle(std::unique_ptr<Pool> pool);

    // Explicitly frees a previously recycled pool (if any), reclaiming the memory.
    static void FreeRecycledPool() { Recycle(nullptr); }

    // Attaches a pool to the current thread.
    // It is an error to call this while a pool is already attached.
    void attachToThread();

    // Once you are done creating or destroying IRNodes in the pool, detach it from the thread.
    // It is an error to call this while no pool is attached.
    void detachFromThread();

    // Retrieves a node from the thread pool. If the pool is exhausted, this will allocate a node.
    static void* AllocIRNode();

    // Releases a node that was created by AllocIRNode. This will return it to the pool, or free it,
    // as appropriate. Make sure to free all nodes, since some of them may be real allocations.
    static void FreeIRNode(void* node_v);

private:
    void checkForLeaks();

    Pool() = default;  // use CreatePoolOnThread to make a pool
    PoolData* fData = nullptr;
};

}  // namespace SkSL

#endif
