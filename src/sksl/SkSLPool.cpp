/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLPool.h"

#include "src/sksl/ir/SkSLIRNode.h"

namespace SkSL {

#if defined(SK_BUILD_FOR_IOS) && \
        (!defined(__IPHONE_9_0) || __IPHONE_OS_VERSION_MIN_REQUIRED < __IPHONE_9_0)

// iOS did not support for C++11 `thread_local` variables until iOS 9.
// Pooling is not supported here; we allocate all nodes directly.
void Pool::Enable() {}
void Pool::Disable() {}
void* Pool::AllocIRNode() { return ::operator new(sizeof(IRNode)); }
void Pool::FreeIRNode(void* node) { ::operator delete(node); }

#else

#define VLOG(...) // printf(__VA_ARGS__)

namespace {

static constexpr int kPoolSize = 2000;

struct PoolData {
    struct IRNodeData {
        union {
            uint8_t fBuffer[sizeof(IRNode)];
            IRNodeData* fFreeListNext;
        };
    };

    // This holds the first free node in the pool when pooling is enabled. It will be null when the
    // pool is exhausted.
    IRNodeData* fFreeListHead = nullptr;

    // This tracks the enable state of the pool.
    bool fEnabled = false;

    // Our pooled data lives here.
    IRNodeData fNodes[kPoolSize];

    PoolData() {
        // Initialize each pool node as a free node. The free nodes form a singly-linked list, each
        // pointing to the next free node in sequence.
        for (int index = 0; index < kPoolSize - 1; ++index) {
            fNodes[index].fFreeListNext = &fNodes[index + 1];
        }

        fNodes[kPoolSize - 1].fFreeListNext = nullptr;
    }
};

static thread_local PoolData* sPoolData = new PoolData;

}  // namespace

void Pool::Enable() {
    PoolData& pool = *sPoolData;
    SkASSERT(!pool.fEnabled);

    pool.fEnabled = true;
    VLOG("ON    Pool:0x%016llX\n", (uint64_t)&pool);
}

void Pool::Disable() {
    PoolData& pool = *sPoolData;
    SkASSERT(pool.fEnabled);

    pool.fEnabled = false;
    VLOG("OFF   Pool:0x%016llX\n", (uint64_t)&pool);
}

void* Pool::AllocIRNode() {
    PoolData& pool = *sPoolData;

    // Take the first free node from the pool.
    PoolData::IRNodeData* node = pool.fFreeListHead;
    if (node && pool.fEnabled) {
        pool.fFreeListHead = node->fFreeListNext;
        VLOG("ALLOC Pool:0x%016llX Index:%04d         0x%016llX\n",
             (uint64_t)&pool, (int)(node - &pool.fNodes[0]), (uint64_t)node);
        return node->fBuffer;
    } else {
        // The pool is disabled or full; allocate nodes using malloc.
        void* ptr = ::operator new(sizeof(IRNode));
        VLOG("ALLOC Pool:0x%016llX Index:____ malloc  0x%016llX\n",
             (uint64_t)&pool, (uint64_t)ptr);
        return ptr;
    }
}

void Pool::FreeIRNode(void* node_v) {
    PoolData& pool = *sPoolData;

    auto* node = static_cast<PoolData::IRNodeData*>(node_v);
    if (node >= &pool.fNodes[0] && node < &pool.fNodes[kPoolSize]) {
        // This node came from our pool. Push it onto the top of the free list.
        VLOG("FREE  Pool:0x%016llX Index:%04d         0x%016llX\n",
             (uint64_t)&pool, (int)(node - &pool.fNodes[0]), (uint64_t)node);
        node->fFreeListNext = pool.fFreeListHead;
        pool.fFreeListHead = node;
    } else {
        // This node was malloced, so it must be freed.
        VLOG("FREE  Pool:0x%016llX Index:____ free    0x%016llX\n",
             (uint64_t)&pool, (uint64_t)node);
        ::operator delete(node);
    }
}

#endif

}  // namespace SkSL
