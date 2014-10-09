
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
    SkASSERT(gpu);
    SkASSERT(gpu->getContext());
    SkASSERT(gpu->getContext()->getResourceCache2());
    return gpu->getContext()->getResourceCache2();
}

static inline GrResourceCache* get_resource_cache(GrGpu* gpu) {
    SkASSERT(gpu);
    SkASSERT(gpu->getContext());
    SkASSERT(gpu->getContext()->getResourceCache());
    return gpu->getContext()->getResourceCache();
}

GrGpuResource::GrGpuResource(GrGpu* gpu, bool isWrapped)
    : fGpu(gpu)
    , fCacheEntry(NULL)
    , fUniqueID(CreateUniqueID())
    , fScratchKey(GrResourceKey::NullScratchKey()) {
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
    // subclass should have released this.
    SkASSERT(this->wasDestroyed());
}

void GrGpuResource::release() { 
    if (fGpu) {
        this->onRelease();
        get_resource_cache2(fGpu)->removeResource(this);
        fGpu = NULL;
    }
}

void GrGpuResource::abandon() {
    if (fGpu) {
        this->onAbandon();
        get_resource_cache2(fGpu)->removeResource(this);
        fGpu = NULL;
    }
}

const GrContext* GrGpuResource::getContext() const {
    if (fGpu) {
        return fGpu->getContext();
    } else {
        return NULL;
    }
}

GrContext* GrGpuResource::getContext() {
    if (fGpu) {
        return fGpu->getContext();
    } else {
        return NULL;
    }
}

void GrGpuResource::notifyIsPurgable() const {
    if (fCacheEntry && !this->wasDestroyed()) {
        get_resource_cache(fGpu)->notifyPurgable(this);
    }
}

void GrGpuResource::setScratchKey(const GrResourceKey& scratchKey) {
    SkASSERT(fScratchKey.isNullScratch());
    SkASSERT(scratchKey.isScratch());
    SkASSERT(!scratchKey.isNullScratch());
    fScratchKey = scratchKey;
}

const GrResourceKey* GrGpuResource::getContentKey() const {
    if (fCacheEntry && !fCacheEntry->key().isScratch()) {
        return &fCacheEntry->key();
    }
    return NULL;
}

bool GrGpuResource::isScratch() const {
    // Currently scratch resources have a cache entry in GrResourceCache with a scratch key.
    return NULL != fCacheEntry && fCacheEntry->key().isScratch();
}

uint32_t GrGpuResource::CreateUniqueID() {
    static int32_t gUniqueID = SK_InvalidUniqueID;
    uint32_t id;
    do {
        id = static_cast<uint32_t>(sk_atomic_inc(&gUniqueID) + 1);
    } while (id == SK_InvalidUniqueID);
    return id;
}
