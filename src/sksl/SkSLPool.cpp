/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLPool.h"

#include "src/sksl/ir/SkSLIRNode.h"

#define VLOG(...) // printf(__VA_ARGS__)

namespace SkSL {

#if defined(SK_BUILD_FOR_IOS) && \
        (!defined(__IPHONE_9_0) || __IPHONE_OS_VERSION_MIN_REQUIRED < __IPHONE_9_0)

// iOS did not support for C++11 `thread_local` variables until iOS 9.
// Pooling is not supported here; we allocate all nodes directly.
struct PoolData {};

Pool::~Pool() {}
std::unique_ptr<Pool> Pool::CreatePoolOnThread(int nodesInPool) {
    auto pool = std::unique_ptr<Pool>(new Pool);
    pool->fData = nullptr;
    return pool;
}
void Pool::detachFromThread() {}
void Pool::attachToThread() {}
void* Pool::AllocIRNode() { return ::operator new(sizeof(IRNode)); }
void Pool::FreeIRNode(void* node) { ::operator delete(node); }

#else  // !defined(SK_BUILD_FOR_IOS)...

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

    ptrdiff_t nodeIndex(IRNodeData* node) {
        SkASSERT(node >= fNodes);
        SkASSERT(node < fNodesEnd);
        return node - fNodes;
    }
};

static thread_local PoolData* sPoolData = nullptr;

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
    if (sPoolData == fData) {
        SkDEBUGFAIL("SkSL pool is being destroyed while it is still attached to the thread");
        sPoolData = nullptr;
    }

    // In debug mode, report any leaked nodes.
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

    VLOG("DELETE Pool:0x%016llX\n", (uint64_t)fData);
    free(fData);
}

std::unique_ptr<Pool> Pool::CreatePoolOnThread(int nodesInPool) {
    auto pool = std::unique_ptr<Pool>(new Pool);
    pool->fData = create_pool_data(nodesInPool);
    pool->fData->fFreeListHead = &pool->fData->fNodes[0];
    VLOG("CREATE Pool:0x%016llX\n", (uint64_t)pool->fData);
    pool->attachToThread();
    return pool;
}

void Pool::detachFromThread() {
    VLOG("DETACH Pool:0x%016llX\n", (uint64_t)sPoolData);
    SkASSERT(sPoolData != nullptr);
    sPoolData = nullptr;
}

void Pool::attachToThread() {
    VLOG("ATTACH Pool:0x%016llX\n", (uint64_t)fData);
    SkASSERT(sPoolData == nullptr);
    sPoolData = fData;
}

void* Pool::AllocIRNode() {
    // Is a pool attached?
    if (sPoolData) {
        // Does the pool contain a free node?
        IRNodeData* node = sPoolData->fFreeListHead;
        if (node) {
            // Yes. Take a node from the freelist.
            sPoolData->fFreeListHead = node->fFreeListNext;
            VLOG("ALLOC  Pool:0x%016llX Index:%04d         0x%016llX\n",
                 (uint64_t)sPoolData, (int)(node - &sPoolData->fNodes[0]), (uint64_t)node);
            return node->fBuffer;
        }
    }

    // The pool is detached or full; allocate nodes using malloc.
    void* ptr = ::operator new(sizeof(IRNode));
    VLOG("ALLOC  Pool:0x%016llX Index:____ malloc  0x%016llX\n",
         (uint64_t)sPoolData, (uint64_t)ptr);
    return ptr;
}

void Pool::FreeIRNode(void* node_v) {
    // Is a pool attached?
    if (sPoolData) {
        // Did this node come from our pool?
        auto* node = static_cast<IRNodeData*>(node_v);
        if (node >= &sPoolData->fNodes[0] && node < sPoolData->fNodesEnd) {
            // Yes. Push it back onto the freelist.
            VLOG("FREE   Pool:0x%016llX Index:%04d         0x%016llX\n",
                 (uint64_t)sPoolData, (int)(node - &sPoolData->fNodes[0]), (uint64_t)node);
            node->fFreeListNext = sPoolData->fFreeListHead;
            sPoolData->fFreeListHead = node;
            return;
        }
    }

    // No pool is attached or the node was malloced; it must be freed.
    VLOG("FREE   Pool:0x%016llX Index:____ free    0x%016llX\n",
         (uint64_t)sPoolData, (uint64_t)node_v);
    ::operator delete(node_v);
}

#endif  // !defined(SK_BUILD_FOR_IOS)...

}  // namespace SkSL
