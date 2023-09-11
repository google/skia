/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/base/SkSpinlock.h"
#include "src/gpu/ganesh/GrMemoryPool.h"
#include "src/gpu/ganesh/GrProcessor.h"

#include <memory>

// We use a global pool protected by a mutex(spinlock). Chrome may use the same GrContext on
// different threads. The GrContext is not used concurrently on different threads and there is a
// memory barrier between accesses of a context on different threads. Also, there may be multiple
// GrContexts and those contexts may be in use concurrently on different threads.
namespace {
#if !defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
static SkSpinlock gProcessorSpinlock;
#endif
class MemoryPoolAccessor {
public:

// We know in the Android framework there is only one GrContext.
#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
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
