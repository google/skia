/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLPool.h"

#include <bitset>

#include "include/private/SkMutex.h"
#include "src/sksl/ir/SkSLIRNode.h"

#define VLOG(...) // printf(__VA_ARGS__)

namespace SkSL {

namespace {

template <int kNodeSize, int kNumNodes>
class Subpool {
public:
    Subpool() {
        // Initializes each node in the pool as a free node. The free nodes form a singly-linked
        // list, each pointing to the next free node in sequence.
        for (int index = 0; index < kNumNodes - 1; ++index) {
            fNodes[index].fFreeListNext = &fNodes[index + 1];
        }
        fNodes[kNumNodes - 1].fFreeListNext = nullptr;
    }

    void* poolBegin() {
        return &fNodes[0];
    }

    void* poolEnd() {
        return &fNodes[kNumNodes];
    }

    void* alloc() {
        // Does the pool contain a free node?
        if (!fFreeListHead) {
            return nullptr;
        }
        // Yes. Take a node from the freelist.
        auto* node = fFreeListHead;
        fFreeListHead = node->fFreeListNext;
        return node->fBuffer;
    }

    void free(void* node_v) {
        SkASSERT(this->isValidNodePtrInPool(node_v));

        // Push a node back onto the freelist.
        auto* node = static_cast<Subpool::Node*>(node_v);
        node->fFreeListNext = fFreeListHead;
        fFreeListHead = node;
    }

    bool isValidNodePtrInPool(void* node_v) {
        // Verify that the pointer exists in our subpool at all.
        if (node_v < this->poolBegin()) {
            return false;
        }
        if (node_v >= this->poolEnd()) {
            return false;
        }
        // Verify that the pointer points to the start of a node, not the middle.
        intptr_t offsetInPool = (intptr_t)node_v - (intptr_t)this->poolBegin();
        return (offsetInPool % kNodeSize) == 0;
    }

    void checkForLeaks() {
    #ifdef SK_DEBUG
        // Walk the free list and mark each node. We should encounter every item in the pool.
        std::bitset<kNumNodes> freed;
        for (Node* node = fFreeListHead; node; node = node->fFreeListNext) {
            ptrdiff_t nodeIndex = this->nodeIndex(node);
            freed[nodeIndex] = true;
        }
        // Look for any bit left unset above, and report it as a leak.
        bool foundLeaks = false;
        for (int index = 0; index < kNumNodes; ++index) {
            if (!freed[index]) {
                SkDebugf("Node %d leaked: ", index);
                IRNode* leak = reinterpret_cast<IRNode*>(fNodes[index].fBuffer);
                SkDebugf("%s\n", leak->description().c_str());
                foundLeaks = true;
            }
        }
        if (foundLeaks) {
            SkDEBUGFAIL("leaking SkSL pool nodes; if they are later freed, this will "
                        "likely be fatal");
        }
    #endif
    }

    // Accessors.
    constexpr int nodeCount() { return kNumNodes; }

    int nodeIndex(void* node_v) {
        SkASSERT(this->isValidNodePtrInPool(node_v));

        auto* node = static_cast<Subpool::Node*>(node_v);
        return SkToInt(node - fNodes);
    }

private:
    struct Node {
        union {
            uint8_t fBuffer[kNodeSize];
            Node* fFreeListNext;
        };
    };

    // This holds the first free node in the pool. It will be null when the pool is exhausted.
    Node* fFreeListHead = fNodes;

    // Our pooled data lives here.
    Node fNodes[kNumNodes];
};

static constexpr int kSmallNodeSize = 120;
static constexpr int kNumSmallNodes = 480;
using SmallSubpool = Subpool<kSmallNodeSize, kNumSmallNodes>;

static constexpr int kLargeNodeSize = 240;
static constexpr int kNumLargeNodes = 20;
using LargeSubpool = Subpool<kLargeNodeSize, kNumLargeNodes>;

}  // namespace

struct PoolData {
    SmallSubpool fSmall;
    LargeSubpool fLarge;
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

Pool::~Pool() {
    if (get_thread_local_pool_data() == fData) {
        SkDEBUGFAIL("SkSL pool is being destroyed while it is still attached to the thread");
        set_thread_local_pool_data(nullptr);
    }

    fData->fSmall.checkForLeaks();
    fData->fLarge.checkForLeaks();

    VLOG("DELETE Pool:0x%016llX\n", (uint64_t)fData);
    delete fData;
}

std::unique_ptr<Pool> Pool::Create() {
    SkAutoMutexExclusive lock(recycled_pool_mutex());
    std::unique_ptr<Pool> pool;
    if (sRecycledPool) {
        pool = std::unique_ptr<Pool>(sRecycledPool);
        sRecycledPool = nullptr;
        VLOG("REUSE  Pool:0x%016llX\n", (uint64_t)pool->fData);
    } else {
        pool = std::unique_ptr<Pool>(new Pool);
        pool->fData = new PoolData;
        VLOG("CREATE Pool:0x%016llX\n", (uint64_t)pool->fData);
    }
    return pool;
}

void Pool::Recycle(std::unique_ptr<Pool> pool) {
    if (pool) {
        pool->fData->fSmall.checkForLeaks();
        pool->fData->fLarge.checkForLeaks();
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

void* Pool::AllocIRNode(size_t size) {
    // Is a pool attached?
    PoolData* poolData = get_thread_local_pool_data();
    if (poolData) {
        if (size <= kSmallNodeSize) {
            // The node will fit in the small pool.
            auto* node = poolData->fSmall.alloc();
            if (node) {
                VLOG("ALLOC  Pool:0x%016llX Index:S%03d         0x%016llX\n",
                     (uint64_t)poolData, poolData->fSmall.nodeIndex(node), (uint64_t)node);
                return node;
            }
        } else if (size <= kLargeNodeSize) {
            // Try to allocate a large node.
            auto* node = poolData->fLarge.alloc();
            if (node) {
                VLOG("ALLOC  Pool:0x%016llX Index:L%03d         0x%016llX\n",
                     (uint64_t)poolData, poolData->fLarge.nodeIndex(node), (uint64_t)node);
                return node;
            }
        }
    }

    // The pool can't be used for this allocation. Allocate nodes using the system allocator.
    void* ptr = ::operator new(size);
    VLOG("ALLOC  Pool:0x%016llX Index:____ malloc  0x%016llX\n",
         (uint64_t)poolData, (uint64_t)ptr);
    return ptr;
}

void Pool::FreeIRNode(void* node) {
    // Is a pool attached?
    PoolData* poolData = get_thread_local_pool_data();
    if (poolData) {
        // Did this node come from either of our pools?
        if (node >= poolData->fSmall.poolBegin()) {
            if (node < poolData->fSmall.poolEnd()) {
                poolData->fSmall.free(node);
                VLOG("FREE   Pool:0x%016llX Index:S%03d         0x%016llX\n",
                     (uint64_t)poolData, poolData->fSmall.nodeIndex(node), (uint64_t)node);
                return;
            } else if (node < poolData->fLarge.poolEnd()) {
                poolData->fLarge.free(node);
                VLOG("FREE   Pool:0x%016llX Index:L%03d         0x%016llX\n",
                     (uint64_t)poolData, poolData->fLarge.nodeIndex(node), (uint64_t)node);
                return;
            }
        }
    }

    // We couldn't associate this node with our pool. Free it using the system allocator.
    VLOG("FREE   Pool:0x%016llX Index:____ free    0x%016llX\n",
         (uint64_t)poolData, (uint64_t)node);
    ::operator delete(node);
}


}  // namespace SkSL
