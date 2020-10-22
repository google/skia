/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLPool.h"

#include "include/private/SkMutex.h"
#include "src/sksl/ir/SkSLIRNode.h"

#define VLOG(...) // printf(__VA_ARGS__)

namespace SkSL {

namespace { struct IRNodeData {
    union {
        uint8_t fBuffer[sizeof(IRNode)];
        IRNodeData* fFreeListNext;
    };
}; }

struct PoolData {
    // This holds the first free node in the pool. It will be null when the pool is exhausted.
    IRNodeData* fFreeListHead = fNodes;

    // This points to end of our pooled data, and implies the number of nodes.
    IRNodeData* fNodesEnd = nullptr;

    // Our pooled data lives here. (We allocate lots of nodes here, not just one.)
    IRNodeData fNodes[1];

    // Accessors.
    ptrdiff_t nodeCount() { return fNodesEnd - fNodes; }

    int nodeIndex(IRNodeData* node) {
        SkASSERT(node >= fNodes);
        SkASSERT(node < fNodesEnd);
        return SkToInt(node - fNodes);
    }
};

#if defined(SK_BUILD_FOR_IOS) && \
        (!defined(__IPHONE_9_0) || __IPHONE_OS_VERSION_MIN_REQUIRED < __IPHONE_9_0)

#include <pthread.h>

static pthread_key_t get_pthread_key() {
    static pthread_key_t sKey = []{
        pthread_key_t key;
        int result = pthread_key_create(&key, /*destructor=*/nullptr);
        if (result != 0) {
            SK_ABORT("pthread_key_create failure: %d", result);
        }
        return key;
    }();
    return sKey;
}

static PoolData* get_thread_local_pool_data() {
    return static_cast<PoolData*>(pthread_getspecific(get_pthread_key()));
}

static void set_thread_local_pool_data(PoolData* poolData) {
    pthread_setspecific(get_pthread_key(), poolData);
}

#else

static thread_local PoolData* sPoolData = nullptr;

static PoolData* get_thread_local_pool_data() {
    return sPoolData;
}

static void set_thread_local_pool_data(PoolData* poolData) {
    sPoolData = poolData;
}

#endif

static Pool* sRecycledPool; // GUARDED_BY recycled_pool_mutex
static SkMutex& recycled_pool_mutex() {
    static SkMutex* mutex = new SkMutex;
    return *mutex;
}

static PoolData* create_pool_data(int nodesInPool) {
    // Create a PoolData structure with extra space at the end for additional IRNode data.
    int numExtraIRNodes = nodesInPool - 1;
    PoolData* poolData = static_cast<PoolData*>(malloc(sizeof(PoolData) +
                                                       (sizeof(IRNodeData) * numExtraIRNodes)));

    // Initialize each pool node as a free node. The free nodes form a singly-linked list, each
    // pointing to the next free node in sequence.
    for (int index = 0; index < nodesInPool - 1; ++index) {
        poolData->fNodes[index].fFreeListNext = &poolData->fNodes[index + 1];
    }
    poolData->fNodes[nodesInPool - 1].fFreeListNext = nullptr;
    poolData->fNodesEnd = &poolData->fNodes[nodesInPool];

    return poolData;
}

Pool::~Pool() {
    if (get_thread_local_pool_data() == fData) {
        SkDEBUGFAIL("SkSL pool is being destroyed while it is still attached to the thread");
        set_thread_local_pool_data(nullptr);
    }

    this->checkForLeaks();

    VLOG("DELETE Pool:0x%016llX\n", (uint64_t)fData);
    free(fData);
}

std::unique_ptr<Pool> Pool::Create() {
    constexpr int kNodesInPool = 2000;

    SkAutoMutexExclusive lock(recycled_pool_mutex());
    std::unique_ptr<Pool> pool;
    if (sRecycledPool) {
        pool = std::unique_ptr<Pool>(sRecycledPool);
        sRecycledPool = nullptr;
        VLOG("REUSE  Pool:0x%016llX\n", (uint64_t)pool->fData);
    } else {
        pool = std::unique_ptr<Pool>(new Pool);
        pool->fData = create_pool_data(kNodesInPool);
        pool->fData->fFreeListHead = &pool->fData->fNodes[0];
        VLOG("CREATE Pool:0x%016llX\n", (uint64_t)pool->fData);
    }
    return pool;
}

void Pool::Recycle(std::unique_ptr<Pool> pool) {
    if (pool) {
        pool->checkForLeaks();
    }

    SkAutoMutexExclusive lock(recycled_pool_mutex());
    if (sRecycledPool) {
        delete sRecycledPool;
    }

    VLOG("STASH  Pool:0x%016llX\n", pool ? (uint64_t)pool->fData : 0ull);
    sRecycledPool = pool.release();
}

void Pool::attachToThread() {
    VLOG("ATTACH Pool:0x%016llX\n", (uint64_t)fData);
    SkASSERT(get_thread_local_pool_data() == nullptr);
    set_thread_local_pool_data(fData);
}

void Pool::detachFromThread() {
    VLOG("DETACH Pool:0x%016llX\n", (uint64_t)get_thread_local_pool_data());
    SkASSERT(get_thread_local_pool_data() != nullptr);
    set_thread_local_pool_data(nullptr);
}

void* Pool::AllocIRNode() {
    // Is a pool attached?
    PoolData* poolData = get_thread_local_pool_data();
    if (poolData) {
        // Does the pool contain a free node?
        IRNodeData* node = poolData->fFreeListHead;
        if (node) {
            // Yes. Take a node from the freelist.
            poolData->fFreeListHead = node->fFreeListNext;
            VLOG("ALLOC  Pool:0x%016llX Index:%04d         0x%016llX\n",
                 (uint64_t)poolData, poolData->nodeIndex(node), (uint64_t)node);
            return node->fBuffer;
        }
    }

    // The pool is detached or full; allocate nodes using malloc.
    void* ptr = ::operator new(sizeof(IRNode));
    VLOG("ALLOC  Pool:0x%016llX Index:____ malloc  0x%016llX\n",
         (uint64_t)poolData, (uint64_t)ptr);
    return ptr;
}

void Pool::FreeIRNode(void* node_v) {
    // Is a pool attached?
    PoolData* poolData = get_thread_local_pool_data();
    if (poolData) {
        // Did this node come from our pool?
        auto* node = static_cast<IRNodeData*>(node_v);
        if (node >= &poolData->fNodes[0] && node < poolData->fNodesEnd) {
            // Yes. Push it back onto the freelist.
            VLOG("FREE   Pool:0x%016llX Index:%04d         0x%016llX\n",
                 (uint64_t)poolData, poolData->nodeIndex(node), (uint64_t)node);
            node->fFreeListNext = poolData->fFreeListHead;
            poolData->fFreeListHead = node;
            return;
        }
    }

    // No pool is attached or the node was malloced; it must be freed.
    VLOG("FREE   Pool:0x%016llX Index:____ free    0x%016llX\n",
         (uint64_t)poolData, (uint64_t)node_v);
    ::operator delete(node_v);
}

void Pool::checkForLeaks() {
#ifdef SK_DEBUG
    ptrdiff_t nodeCount = fData->nodeCount();
    std::vector<bool> freed(nodeCount);
    for (IRNodeData* node = fData->fFreeListHead; node; node = node->fFreeListNext) {
        ptrdiff_t nodeIndex = fData->nodeIndex(node);
        freed[nodeIndex] = true;
    }
    bool foundLeaks = false;
    for (int index = 0; index < nodeCount; ++index) {
        if (!freed[index]) {
            IRNode* leak = reinterpret_cast<IRNode*>(fData->fNodes[index].fBuffer);
            SkDebugf("Node %d leaked: %s\n", index, leak->description().c_str());
            foundLeaks = true;
        }
    }
    if (foundLeaks) {
        SkDEBUGFAIL("leaking SkSL pool nodes; if they are later freed, this will likely be fatal");
    }
#endif
}

}  // namespace SkSL
