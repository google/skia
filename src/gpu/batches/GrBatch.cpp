/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBatch.h"

#include "GrMemoryPool.h"
#include "SkSpinlock.h"

// TODO I noticed a small benefit to using a larger exclusive pool for batches.  Its very small,
// but seems to be mostly consistent.  There is a lot in flux right now, but we should really
// revisit this when batch is everywhere


// We use a global pool protected by a mutex(spinlock). Chrome may use the same GrContext on
// different threads. The GrContext is not used concurrently on different threads and there is a
// memory barrier between accesses of a context on different threads. Also, there may be multiple
// GrContexts and those contexts may be in use concurrently on different threads.
namespace {
static SkSpinlock gBatchSpinlock;
class MemoryPoolAccessor {
public:
    MemoryPoolAccessor() { gBatchSpinlock.acquire(); }

    ~MemoryPoolAccessor() { gBatchSpinlock.release(); }

    GrMemoryPool* pool() const {
        static GrMemoryPool gPool(16384, 16384);
        return &gPool;
    }
};
}

int32_t GrBatch::gCurrBatchClassID = GrBatch::kIllegalBatchID;

int32_t GrBatch::gCurrBatchUniqueID = GrBatch::kIllegalBatchID;

void* GrBatch::operator new(size_t size) {
    return MemoryPoolAccessor().pool()->allocate(size);
}

void GrBatch::operator delete(void* target) {
    return MemoryPoolAccessor().pool()->release(target);
}

GrBatch::GrBatch(uint32_t classID)
    : fClassID(classID)
    , fUniqueID(kIllegalBatchID) {
    SkDEBUGCODE(fUsed = false;)
}

GrBatch::~GrBatch() {}
