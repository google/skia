/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGpuResource.h"
#include "GrContext.h"
#include "GrResourceCache.h"
#include "GrGpu.h"
#include "GrGpuResourcePriv.h"
#include "SkTraceMemoryDump.h"

static inline GrResourceCache* get_resource_cache(GrGpu* gpu) {
    SkASSERT(gpu);
    SkASSERT(gpu->getContext());
    SkASSERT(gpu->getContext()->getResourceCache());
    return gpu->getContext()->getResourceCache();
}

GrGpuResource::GrGpuResource(GrGpu* gpu)
    : fExternalFlushCntWhenBecamePurgeable(0)
    , fGpu(gpu)
    , fGpuMemorySize(kInvalidGpuMemorySize)
    , fBudgeted(SkBudgeted::kNo)
    , fRefsWrappedObjects(false)
    , fUniqueID(CreateUniqueID()) {
    SkDEBUGCODE(fCacheArrayIndex = -1);
}

void GrGpuResource::registerWithCache(SkBudgeted budgeted) {
    SkASSERT(fBudgeted == SkBudgeted::kNo);
    fBudgeted = budgeted;
    this->computeScratchKey(&fScratchKey);
    get_resource_cache(fGpu)->resourceAccess().insertResource(this);
}

void GrGpuResource::registerWithCacheWrapped() {
    SkASSERT(fBudgeted == SkBudgeted::kNo);
    // Currently resources referencing wrapped objects are not budgeted.
    fRefsWrappedObjects = true;
    get_resource_cache(fGpu)->resourceAccess().insertResource(this);
}

GrGpuResource::~GrGpuResource() {
    // The cache should have released or destroyed this resource.
    SkASSERT(this->wasDestroyed());
}

void GrGpuResource::release() {
    SkASSERT(fGpu);
    this->onRelease();
    get_resource_cache(fGpu)->resourceAccess().removeResource(this);
    fGpu = nullptr;
    fGpuMemorySize = 0;
}

void GrGpuResource::abandon() {
    if (this->wasDestroyed()) {
        return;
    }
    SkASSERT(fGpu);
    this->onAbandon();
    get_resource_cache(fGpu)->resourceAccess().removeResource(this);
    fGpu = nullptr;
    fGpuMemorySize = 0;
}

void GrGpuResource::dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const {
    // Dump resource as "skia/gpu_resources/resource_#".
    SkString dumpName("skia/gpu_resources/resource_");
    dumpName.appendU32(this->uniqueID().asUInt());

    traceMemoryDump->dumpNumericValue(dumpName.c_str(), "size", "bytes", this->gpuMemorySize());

    if (this->isPurgeable()) {
        traceMemoryDump->dumpNumericValue(dumpName.c_str(), "purgeable_size", "bytes",
                                          this->gpuMemorySize());
    }

    // Call setMemoryBacking to allow sub-classes with implementation specific backings (such as GL
    // objects) to provide additional information.
    this->setMemoryBacking(traceMemoryDump, dumpName);
}

const GrContext* GrGpuResource::getContext() const {
    if (fGpu) {
        return fGpu->getContext();
    } else {
        return nullptr;
    }
}

GrContext* GrGpuResource::getContext() {
    if (fGpu) {
        return fGpu->getContext();
    } else {
        return nullptr;
    }
}

void GrGpuResource::didChangeGpuMemorySize() const {
    if (this->wasDestroyed()) {
        return;
    }

    size_t oldSize = fGpuMemorySize;
    SkASSERT(kInvalidGpuMemorySize != oldSize);
    fGpuMemorySize = kInvalidGpuMemorySize;
    get_resource_cache(fGpu)->resourceAccess().didChangeGpuMemorySize(this, oldSize);
}

void GrGpuResource::removeUniqueKey() {
    if (this->wasDestroyed()) {
        return;
    }
    SkASSERT(fUniqueKey.isValid());
    get_resource_cache(fGpu)->resourceAccess().removeUniqueKey(this);
}

void GrGpuResource::setUniqueKey(const GrUniqueKey& key) {
    SkASSERT(this->internalHasRef());
    SkASSERT(key.isValid());

    // Uncached resources can never have a unique key, unless they're wrapped resources. Wrapped
    // resources are a special case: the unique keys give us a weak ref so that we can reuse the
    // same resource (rather than re-wrapping). When a wrapped resource is no longer referenced,
    // it will always be released - it is never converted to a scratch resource.
    if (SkBudgeted::kNo == this->resourcePriv().isBudgeted() && !this->fRefsWrappedObjects) {
        return;
    }

    if (this->wasDestroyed()) {
        return;
    }

    get_resource_cache(fGpu)->resourceAccess().changeUniqueKey(this, key);
}

void GrGpuResource::notifyAllCntsAreZero(CntType lastCntTypeToReachZero) const {
    if (this->wasDestroyed()) {
        // We've already been removed from the cache. Goodbye cruel world!
        delete this;
        return;
    }

    // We should have already handled this fully in notifyRefCntIsZero().
    SkASSERT(kRef_CntType != lastCntTypeToReachZero);

    GrGpuResource* mutableThis = const_cast<GrGpuResource*>(this);
    static const uint32_t kFlag =
        GrResourceCache::ResourceAccess::kAllCntsReachedZero_RefNotificationFlag;
    get_resource_cache(fGpu)->resourceAccess().notifyCntReachedZero(mutableThis, kFlag);
}

bool GrGpuResource::notifyRefCountIsZero() const {
    if (this->wasDestroyed()) {
        // handle this in notifyAllCntsAreZero().
        return true;
    }

    GrGpuResource* mutableThis = const_cast<GrGpuResource*>(this);
    uint32_t flags = GrResourceCache::ResourceAccess::kRefCntReachedZero_RefNotificationFlag;
    if (!this->internalHasPendingIO()) {
        flags |= GrResourceCache::ResourceAccess::kAllCntsReachedZero_RefNotificationFlag;
    }
    get_resource_cache(fGpu)->resourceAccess().notifyCntReachedZero(mutableThis, flags);

    // There is no need to call our notifyAllCntsAreZero function at this point since we already
    // told the cache about the state of cnts.
    return false;
}

void GrGpuResource::removeScratchKey() {
    if (!this->wasDestroyed() && fScratchKey.isValid()) {
        get_resource_cache(fGpu)->resourceAccess().willRemoveScratchKey(this);
        fScratchKey.reset();
    }
}

void GrGpuResource::makeBudgeted() {
    if (!this->wasDestroyed() && SkBudgeted::kNo == fBudgeted) {
        // Currently resources referencing wrapped objects are not budgeted.
        SkASSERT(!fRefsWrappedObjects);
        fBudgeted = SkBudgeted::kYes;
        get_resource_cache(fGpu)->resourceAccess().didChangeBudgetStatus(this);
    }
}

void GrGpuResource::makeUnbudgeted() {
    if (!this->wasDestroyed() && SkBudgeted::kYes == fBudgeted &&
        !fUniqueKey.isValid()) {
        fBudgeted = SkBudgeted::kNo;
        get_resource_cache(fGpu)->resourceAccess().didChangeBudgetStatus(this);
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
