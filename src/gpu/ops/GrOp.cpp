/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrOp.h"

#include "GrMemoryPool.h"
#include "SkSpinlock.h"

// TODO I noticed a small benefit to using a larger exclusive pool for ops. Its very small, but
// seems to be mostly consistent.  There is a lot in flux right now, but we should really revisit
// this.


// We use a global pool protected by a mutex(spinlock). Chrome may use the same GrContext on
// different threads. The GrContext is not used concurrently on different threads and there is a
// memory barrier between accesses of a context on different threads. Also, there may be multiple
// GrContexts and those contexts may be in use concurrently on different threads.
namespace {
static SkSpinlock gOpPoolSpinLock;
class MemoryPoolAccessor {
public:

// We know in the Android framework there is only one GrContext.
#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
    MemoryPoolAccessor() {}
    ~MemoryPoolAccessor() {}
#else
    MemoryPoolAccessor() { gOpPoolSpinLock.acquire(); }
    ~MemoryPoolAccessor() { gOpPoolSpinLock.release(); }
#endif

    GrMemoryPool* pool() const {
        static GrMemoryPool gPool(16384, 16384);
        return &gPool;
    }
};
}

int32_t GrOp::gCurrOpClassID = GrOp::kIllegalOpID;

int32_t GrOp::gCurrOpUniqueID = GrOp::kIllegalOpID;

void* GrOp::operator new(size_t size) {
    return MemoryPoolAccessor().pool()->allocate(size);
}

void GrOp::operator delete(void* target) {
    return MemoryPoolAccessor().pool()->release(target);
}

GrOp::GrOp(uint32_t classID)
    : fClassID(classID)
    , fUniqueID(kIllegalOpID) {
    SkASSERT(classID == SkToU32(fClassID));
    SkDEBUGCODE(fBoundsFlags = kUninitialized_BoundsFlag);
}

GrOp::~GrOp() {}
