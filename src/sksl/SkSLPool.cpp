/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLPool.h"

#define VLOG(...) // printf(__VA_ARGS__)

namespace SkSL {

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

static MemoryPool* get_thread_local_memory_pool() {
    return static_cast<MemoryPool*>(pthread_getspecific(get_pthread_key()));
}

static void set_thread_local_memory_pool(MemoryPool* poolData) {
    pthread_setspecific(get_pthread_key(), poolData);
}

#else

static thread_local MemoryPool* sMemPool = nullptr;

static MemoryPool* get_thread_local_memory_pool() {
    return sMemPool;
}

static void set_thread_local_memory_pool(MemoryPool* memPool) {
    sMemPool = memPool;
}

#endif

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

void* Pool::AllocIRNode(size_t size) {
    // Is a pool attached?
    MemoryPool* memPool = get_thread_local_memory_pool();
    if (memPool) {
        void* node = memPool->allocate(size);
        VLOG("ALLOC  Pool:0x%016llX  0x%016llX\n", (uint64_t)memPool, (uint64_t)node);
        return node;
    }

    // There's no pool attached. Allocate nodes using the system allocator.
    void* node = ::operator new(size);
    VLOG("ALLOC  Pool:__________________  0x%016llX\n", (uint64_t)node);
    return node;
}

void Pool::FreeIRNode(void* node) {
    // Is a pool attached?
    MemoryPool* memPool = get_thread_local_memory_pool();
    if (memPool) {
        VLOG("FREE   Pool:0x%016llX  0x%016llX\n", (uint64_t)memPool, (uint64_t)node);
        memPool->release(node);
        return;
    }

    // There's no pool attached. Free it using the system allocator.
    VLOG("FREE   Pool:__________________  0x%016llX\n", (uint64_t)node);
    ::operator delete(node);
}

}  // namespace SkSL
