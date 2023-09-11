/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Resource.h"

#include "src/gpu/graphite/ResourceCache.h"

namespace skgpu::graphite {

Resource::Resource(const SharedContext* sharedContext,
                   Ownership ownership,
                   skgpu::Budgeted budgeted,
                   size_t gpuMemorySize)
        : fSharedContext(sharedContext)
        , fUsageRefCnt(1)
        , fCommandBufferRefCnt(0)
        , fCacheRefCnt(0)
        , fOwnership(ownership)
        , fGpuMemorySize(gpuMemorySize)
        , fBudgeted(budgeted) {
    // If we don't own the resource that must mean its wrapped in a client object. Thus we should
    // not be budgeted
    SkASSERT(fOwnership == Ownership::kOwned || fBudgeted == skgpu::Budgeted::kNo);
}

Resource::~Resource() {
    // The cache should have released or destroyed this resource.
    SkASSERT(this->wasDestroyed());
}

void Resource::registerWithCache(sk_sp<ResourceCache> returnCache) {
    SkASSERT(!fReturnCache);
    SkASSERT(returnCache);

    fReturnCache = std::move(returnCache);
}

bool Resource::notifyARefIsZero(LastRemovedRef removedRef) const {
    // No resource should have been destroyed if there was still any sort of ref on it.
    SkASSERT(!this->wasDestroyed());

    Resource* mutableThis = const_cast<Resource*>(this);

    // TODO: We have not switched all resources to use the ResourceCache yet. Once we do we should
    // be able to assert that we have an fCacheReturn.
    // SkASSERT(fReturnCache);
    if (removedRef != LastRemovedRef::kCache &&
        fReturnCache &&
        fReturnCache->returnResource(mutableThis, removedRef)) {
        return false;
    }

    if (!this->hasAnyRefs()) {
        return true;
    }
    return false;
}

void Resource::internalDispose() {
    SkASSERT(fSharedContext);
    this->invokeReleaseProc();
    this->freeGpuData();
    fSharedContext = nullptr;
    // TODO: If we ever support freeing all the backend objects without deleting the object, we'll
    // need to add a hasAnyRefs() check here.
    delete this;
}

bool Resource::isPurgeable() const {
    // For being purgeable we don't care if there are cacheRefs on the object since the cacheRef
    // will always be greater than 1 since we add one on insert and don't remove that ref until
    // the Resource is removed from the cache.
    return !(this->hasUsageRef() || this->hasCommandBufferRef());
}

} // namespace skgpu::graphite

