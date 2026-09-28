/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkSpinlock.h"
#include "src/gpu/ganesh/GrMemoryPool.h"
#include "src/gpu/ganesh/GrProcessor.h"

#include <memory>

// GrProcessors are allocated from a single process-global memory pool. By default, access to the
// pool is serialized with a spinlock: Chrome may use the same GrContext on different threads (never
// concurrently, and with a memory barrier between accesses), and there may also be multiple
// GrContexts in use concurrently on different threads.
//
// Clients that guarantee that at most one GrDirectContext is ever in use in the process (and thus
// that this pool is never accessed concurrently) may define SK_ASSUME_SINGLE_GANESH_CONTEXT to
// elide the lock entirely. Note that this is a per-process guarantee: it is violated if a second
// library in the same process uses its own Ganesh context concurrently, even if each individual
// component only creates one.
namespace {
#if !defined(SK_ASSUME_SINGLE_GANESH_CONTEXT)
static SkSpinlock gProcessorSpinlock;
#endif
class MemoryPoolAccessor {
public:
#if defined(SK_ASSUME_SINGLE_GANESH_CONTEXT)
    MemoryPoolAccessor() {}
    ~MemoryPoolAccessor() {}
#else
    MemoryPoolAccessor() { gProcessorSpinlock.acquire(); }
    ~MemoryPoolAccessor() { gProcessorSpinlock.release(); }
#endif

    GrMemoryPool* pool() const {
        static GrMemoryPool* gPool = GrMemoryPool::Make(4096, 4096).release();
        return gPool;
    }
};
}  // namespace

///////////////////////////////////////////////////////////////////////////////

void* GrProcessor::operator new(size_t size) { return MemoryPoolAccessor().pool()->allocate(size); }

void* GrProcessor::operator new(size_t object_size, size_t footer_size) {
    return MemoryPoolAccessor().pool()->allocate(object_size + footer_size);
}

void GrProcessor::operator delete(void* target) {
    return MemoryPoolAccessor().pool()->release(target);
}
