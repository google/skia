/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/Resource.h"

namespace skgpu {

Resource::Resource(const Gpu* gpu) : fGpu(gpu) {}

Resource::~Resource() {
    // The cache should have released or destroyed this resource.
    SkASSERT(this->wasDestroyed());
}

void Resource::notifyARefCntIsZero(LastRemovedRef removedRef) const {
    // TODO: Eventually we'll go through the cache to release the resource, but for now we just do
    // this immediately.
    SkASSERT(removedRef == LastRemovedRef::kUsageRef);
    Resource* mutableThis = const_cast<Resource*>(this);
    mutableThis->freeGpuData();
}

void Resource::freeGpuData() {
    SkASSERT(fGpu);
    this->onFreeGpuData();
    fGpu = nullptr;
    delete this;
}

} // namespace skgpu

