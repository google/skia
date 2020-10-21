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

    // Creates a pool to store newly-created IRNodes during program creation and attaches it to the
    // current thread. When your program is complete, call pool->detachFromThread() to transfer
    // ownership of those nodes. Before destroying any of the program's nodes, reattach the pool via
    // pool->attachToThread(). It is an error to call CreatePoolOnThread if a pool is already
    // attached to the current thread.
    static std::unique_ptr<Pool> CreatePoolOnThread(int nodesInPool);

    // Once a pool has been created and the ephemeral work has completed, detach it from its thread.
    // It is an error to call this while no pool is attached.
    void detachFromThread();

    // Reattaches a pool to the current thread. It is an error to call this while a pool is already
    // attached.
    void attachToThread();

    // Retrieves a node from the thread pool. If the pool is exhausted, this will allocate a node.
    static void* AllocIRNode();

    // Releases a node that was created by AllocIRNode. This will return it to the pool, or free it,
    // as appropriate. Make sure to free all nodes, since some of them may be real allocations.
    static void FreeIRNode(void* node_v);

private:
    Pool() = default;  // use CreatePoolOnThread to make a pool
    PoolData* fData = nullptr;
};

}  // namespace SkSL

#endif
