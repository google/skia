
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

GrGpuResource::GrGpuResource(GrGpu* gpu, bool isWrapped)
    : fGpu(gpu)
    , fGpuMemorySize(kInvalidGpuMemorySize)
    , fUniqueID(CreateUniqueID()) {
    if (isWrapped) {
        fFlags = kWrapped_Flag;
    } else {
        // By default all non-wrapped resources are budgeted.
        fFlags = kBudgeted_Flag;
    }
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

bool GrGpuResource::setContentKey(const GrResourceKey& contentKey) {
    // Currently this can only be called once and can't be called when the resource is scratch.
    SkASSERT(this->internalHasRef());

    // Wrapped resources can never have a key.
    if (this->isWrapped()) {
        return false;
    }

    if ((fFlags & kContentKeySet_Flag) || this->wasDestroyed()) {
        return false;
    }

    fContentKey = contentKey;
    fFlags |= kContentKeySet_Flag;

    if (!get_resource_cache2(fGpu)->resourceAccess().didSetContentKey(this)) {
         fFlags &= ~kContentKeySet_Flag;
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

uint32_t GrGpuResource::CreateUniqueID() {
    static int32_t gUniqueID = SK_InvalidUniqueID;
    uint32_t id;
    do {
        id = static_cast<uint32_t>(sk_atomic_inc(&gUniqueID) + 1);
    } while (id == SK_InvalidUniqueID);
    return id;
}

void GrGpuResource::setBudgeted(bool countsAgainstBudget) {
    // Wrapped resources never count against the budget, nothing to do. No point in changing the
    // budgeting of destroyed resources.
    if (this->isWrapped() || this->wasDestroyed()) {
        return;
    }

    uint32_t oldFlags = fFlags;
    if (countsAgainstBudget) {
        fFlags |= kBudgeted_Flag;
    } else {
        fFlags &= ~kBudgeted_Flag;
    }
    if (fFlags != oldFlags) {
        get_resource_cache2(fGpu)->resourceAccess().didChangeBudgetStatus(this);
    }
}
