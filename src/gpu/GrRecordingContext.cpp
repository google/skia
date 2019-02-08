/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrRecordingContext.h"

#include "GrCaps.h"
#include "GrMemoryPool.h"
#include "GrRecordingContextPriv.h"
#include "GrSkSLFPFactoryCache.h"

GrRecordingContext::GrRecordingContext(GrBackendApi backend,
                                       const GrContextOptions& options,
                                       uint32_t contextID)
        : INHERITED(backend, options, contextID) {
}

GrRecordingContext::~GrRecordingContext() { }

sk_sp<GrOpMemoryPool> GrRecordingContext::refOpMemoryPool() {
    if (!fOpMemoryPool) {
        // DDL TODO: should the size of the memory pool be decreased in DDL mode? CPU-side memory
        // consumed in DDL mode vs. normal mode for a single skp might be a good metric of wasted
        // memory.
        fOpMemoryPool = sk_sp<GrOpMemoryPool>(new GrOpMemoryPool(16384, 16384));
    }

    SkASSERT(fOpMemoryPool);
    return fOpMemoryPool;
}

GrOpMemoryPool* GrRecordingContext::opMemoryPool() {
    return this->refOpMemoryPool().get();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
sk_sp<const GrCaps> GrRecordingContextPriv::refCaps() const {
    return fContext->refCaps();
}

sk_sp<GrSkSLFPFactoryCache> GrRecordingContextPriv::fpFactoryCache() {
    return fContext->fpFactoryCache();
}

sk_sp<GrOpMemoryPool> GrRecordingContextPriv::refOpMemoryPool() {
    return fContext->refOpMemoryPool();
}
