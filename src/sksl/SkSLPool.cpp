/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLPool.h"

#include "include/private/SkSLDefines.h"

#define VLOG(...) // printf(__VA_ARGS__)

namespace SkSL {

static thread_local MemoryPool* sMemPool = nullptr;

static MemoryPool* get_thread_local_memory_pool() {
    return sMemPool;
}

static void set_thread_local_memory_pool(MemoryPool* memPool) {
    sMemPool = memPool;
}

Pool::~Pool() {
    if (get_thread_local_memory_pool() == fMemPool.get()) {
        SkDEBUGFAIL("SkSL pool is being destroyed while it is still attached to the thread");
        set_thread_local_memory_pool(nullptr);
    }

    fMemPool->reportLeaks();
    SkASSERT(fMemPool->isEmpty());

    VLOG("DELETE Pool:0x%016llX\n", (uint64_t)fMemPool.get());
}

std::unique_ptr<Pool> Pool::Create() {
    auto pool = std::unique_ptr<Pool>(new Pool);
    pool->fMemPool = MemoryPool::Make(/*preallocSize=*/65536, /*minAllocSize=*/32768);
    VLOG("CREATE Pool:0x%016llX\n", (uint64_t)pool->fMemPool.get());
    return pool;
}

bool Pool::IsAttached() {
    return get_thread_local_memory_pool();
}

void Pool::attachToThread() {
    VLOG("ATTACH Pool:0x%016llX\n", (uint64_t)fMemPool.get());
    SkASSERT(get_thread_local_memory_pool() == nullptr);
    set_thread_local_memory_pool(fMemPool.get());
}

void Pool::detachFromThread() {
    MemoryPool* memPool = get_thread_local_memory_pool();
    VLOG("DETACH Pool:0x%016llX\n", (uint64_t)memPool);
    SkASSERT(memPool == fMemPool.get());
    memPool->resetScratchSpace();
    set_thread_local_memory_pool(nullptr);
}

void* Pool::AllocMemory(size_t size) {
    // Is a pool attached?
    MemoryPool* memPool = get_thread_local_memory_pool();
    if (memPool) {
        void* ptr = memPool->allocate(size);
        VLOG("ALLOC  Pool:0x%016llX  0x%016llX\n", (uint64_t)memPool, (uint64_t)ptr);
        return ptr;
    }

    // There's no pool attached. Allocate memory using the system allocator.
    void* ptr = ::operator new(size);
    VLOG("ALLOC  Pool:__________________  0x%016llX\n", (uint64_t)ptr);
    return ptr;
}

void Pool::FreeMemory(void* ptr) {
    // Is a pool attached?
    MemoryPool* memPool = get_thread_local_memory_pool();
    if (memPool) {
        VLOG("FREE   Pool:0x%016llX  0x%016llX\n", (uint64_t)memPool, (uint64_t)ptr);
        memPool->release(ptr);
        return;
    }

    // There's no pool attached. Free it using the system allocator.
    VLOG("FREE   Pool:__________________  0x%016llX\n", (uint64_t)ptr);
    ::operator delete(ptr);
}

}  // namespace SkSL
