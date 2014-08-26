
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrGpuResource.h"
#include "GrResourceCache2.h"
#include "GrGpu.h"

static inline GrResourceCache2* get_resource_cache2(GrGpu* gpu) {
    SkASSERT(NULL != gpu);
    SkASSERT(NULL != gpu->getContext());
    SkASSERT(NULL != gpu->getContext()->getResourceCache2());
    return gpu->getContext()->getResourceCache2();
}

GrGpuResource::GrGpuResource(GrGpu* gpu, bool isWrapped)
    : fGpu(gpu)
    , fRefCnt(1)
    , fCacheEntry(NULL)
    , fUniqueID(CreateUniqueID()) {
    if (isWrapped) {
        fFlags = kWrapped_FlagBit;
    } else {
        fFlags = 0;
    }
}

void GrGpuResource::registerWithCache() {
    get_resource_cache2(fGpu)->insertResource(this);
}

GrGpuResource::~GrGpuResource() {
    SkASSERT(0 == fRefCnt);
    // subclass should have released this.
    SkASSERT(this->wasDestroyed());
}

void GrGpuResource::release() { 
    if (NULL != fGpu) {
        this->onRelease();
        get_resource_cache2(fGpu)->removeResource(this);
        fGpu = NULL;
    }
}

void GrGpuResource::abandon() {
    if (NULL != fGpu) {
        this->onAbandon();
        get_resource_cache2(fGpu)->removeResource(this);
        fGpu = NULL;
    }
}

const GrContext* GrGpuResource::getContext() const {
    if (NULL != fGpu) {
        return fGpu->getContext();
    } else {
        return NULL;
    }
}

GrContext* GrGpuResource::getContext() {
    if (NULL != fGpu) {
        return fGpu->getContext();
    } else {
        return NULL;
    }
}

uint32_t GrGpuResource::CreateUniqueID() {
    static int32_t gUniqueID = SK_InvalidUniqueID;
    uint32_t id;
    do {
        id = static_cast<uint32_t>(sk_atomic_inc(&gUniqueID) + 1);
    } while (id == SK_InvalidUniqueID);
    return id;
}
