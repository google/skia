
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

GrGpuResource::GrGpuResource(GrGpu* gpu, LifeCycle lifeCycle)
    : fGpu(gpu)
    , fGpuMemorySize(kInvalidGpuMemorySize)
    , fLifeCycle(lifeCycle)
    , fUniqueID(CreateUniqueID()) {
}

void GrGpuResource::registerWithCache() {
    get_resource_cache2(fGpu)->resourceAccess().insertResource(this);
}

GrGpuResource::~GrGpuResource() {
    // The cache should have released or destroyed this resource.
    SkASSERT(this->wasDestroyed());
}

void GrGpuResource::release() { 
    SkASSERT(fGpu);
    this->onRelease();
    get_resource_cache2(fGpu)->resourceAccess().removeResource(this);
    fGpu = NULL;
    fGpuMemorySize = 0;
}

void GrGpuResource::abandon() {
    SkASSERT(fGpu);
    this->onAbandon();
    get_resource_cache2(fGpu)->resourceAccess().removeResource(this);
    fGpu = NULL;
    fGpuMemorySize = 0;
}

const SkData* GrGpuResource::setCustomData(const SkData* data) {
    SkSafeRef(data);
    fData.reset(data);
    return data;
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

void GrGpuResource::didChangeGpuMemorySize() const {
    if (this->wasDestroyed()) {
        return;
    }

    size_t oldSize = fGpuMemorySize;
    SkASSERT(kInvalidGpuMemorySize != oldSize);
    fGpuMemorySize = kInvalidGpuMemorySize;
    get_resource_cache2(fGpu)->resourceAccess().didChangeGpuMemorySize(this, oldSize);
}

bool GrGpuResource::setContentKey(const GrContentKey& key) {
    // Currently this can only be called once and can't be called when the resource is scratch.
    SkASSERT(this->internalHasRef());

    // Wrapped and uncached resources can never have a content key.
    if (!this->cacheAccess().isBudgeted()) {
        return false;
    }

    if (fContentKey.isValid() || this->wasDestroyed()) {
        return false;
    }

    fContentKey = key;

    if (!get_resource_cache2(fGpu)->resourceAccess().didSetContentKey(this)) {
        fContentKey.reset();
        return false;
    }
    return true;
}

void GrGpuResource::notifyIsPurgable() const {
    if (this->wasDestroyed()) {
        // We've already been removed from the cache. Goodbye cruel world!
        SkDELETE(this);
    } else {
        GrGpuResource* mutableThis = const_cast<GrGpuResource*>(this);
        get_resource_cache2(fGpu)->resourceAccess().notifyPurgable(mutableThis);
    }
}

void GrGpuResource::setScratchKey(const GrScratchKey& scratchKey) {
    SkASSERT(!fScratchKey.isValid());
    SkASSERT(scratchKey.isValid());
    // Wrapped resources can never have a scratch key.
    if (this->isWrapped()) {
        return;
    }
    fScratchKey = scratchKey;
}

void GrGpuResource::removeScratchKey() {
    if (!this->wasDestroyed() && fScratchKey.isValid()) {
        get_resource_cache2(fGpu)->resourceAccess().willRemoveScratchKey(this);
        fScratchKey.reset();
    }
}

void GrGpuResource::makeBudgeted() {
    if (GrGpuResource::kUncached_LifeCycle == fLifeCycle) {
        fLifeCycle = kCached_LifeCycle;
        get_resource_cache2(fGpu)->resourceAccess().didChangeBudgetStatus(this);
    }
}

void GrGpuResource::makeUnbudgeted() {
    if (GrGpuResource::kCached_LifeCycle == fLifeCycle && !fContentKey.isValid()) {
        fLifeCycle = kUncached_LifeCycle;
        get_resource_cache2(fGpu)->resourceAccess().didChangeBudgetStatus(this);
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
