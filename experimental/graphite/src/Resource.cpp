/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/Resource.h"

#include "experimental/graphite/src/ResourceCache.h"

namespace skgpu {

Resource::Resource(const Gpu* gpu, Ownership ownership)
        : fGpu(gpu)
        , fUsageRefCnt(1)
        , fCommandBufferRefCnt(0)
        , fCacheRefCnt(0)
        , fOwnership(ownership) {}

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
    SkASSERT(fGpu);
    this->freeGpuData();
    fGpu = nullptr;
    // TODO: If we ever support freeing all the backend objects without deleting the object, we'll
    // need to add a hasAnyRefs() check here.
    delete this;
}

bool Resource::isPurgeable() const {
    return !this->hasAnyRefs();
}

} // namespace skgpu

