/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Resource.h"

#include "include/core/SkTraceMemoryDump.h"
#include "src/gpu/graphite/ResourceCache.h"

namespace skgpu::graphite {

namespace {
uint32_t create_unique_id() {
    static std::atomic<uint32_t> nextID{1};
    uint32_t id;
    do {
        id = nextID.fetch_add(1, std::memory_order_relaxed);
    } while (id == SK_InvalidUniqueID);
    return id;
}
} // namespace anonymous

Resource::Resource(const SharedContext* sharedContext,
                   Ownership ownership,
                   skgpu::Budgeted budgeted,
                   size_t gpuMemorySize,
                   std::string_view label,
                   bool commandBufferRefsAsUsageRefs)
        : fSharedContext(sharedContext)
        , fUsageRefCnt(1)
        , fCommandBufferRefCnt(0)
        , fCacheRefCnt(0)
        , fCommandBufferRefsAsUsageRefs(commandBufferRefsAsUsageRefs)
        , fOwnership(ownership)
        , fGpuMemorySize(gpuMemorySize)
        , fBudgeted(budgeted)
        , fUniqueID(create_unique_id()) {
    // If we don't own the resource that must mean its wrapped in a client object. Thus we should
    // not be budgeted
    SkASSERT(fOwnership == Ownership::kOwned || fBudgeted == skgpu::Budgeted::kNo);

    this->setLabel(label);
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

void Resource::dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const {
    if (this->ownership() == Ownership::kWrapped && !traceMemoryDump->shouldDumpWrappedObjects()) {
        return;
    }

    if (this->budgeted() == skgpu::Budgeted::kNo &&
        !traceMemoryDump->shouldDumpUnbudgetedObjects()) {
        return;
    }

    size_t size = this->gpuMemorySize();

    // Avoid dumping objects without a size (e.g. Samplers, pipelines, etc).
    // TODO: Would a client ever actually want to see all of this? Wouldn't be hard to add it as an
    // option.
    if (size == 0) {
        return;
    }

    SkString resourceName("skia/gpu_resources/resource_");
    resourceName.appendU32(this->uniqueID().asUInt());

    traceMemoryDump->dumpNumericValue(resourceName.c_str(), "size", "bytes", size);
    traceMemoryDump->dumpStringValue(resourceName.c_str(), "type", this->getResourceType());
    traceMemoryDump->dumpStringValue(resourceName.c_str(), "label", this->getLabel().c_str());
    if (this->isPurgeable()) {
        traceMemoryDump->dumpNumericValue(resourceName.c_str(), "purgeable_size", "bytes", size);
    }
    if (traceMemoryDump->shouldDumpWrappedObjects()) {
        traceMemoryDump->dumpWrappedState(resourceName.c_str(),
                                          this->ownership() == Ownership::kWrapped);
    }
    if (traceMemoryDump->shouldDumpUnbudgetedObjects()) {
        traceMemoryDump->dumpBudgetedState(resourceName.c_str(),
                                           this->budgeted() == skgpu::Budgeted::kYes);
    }

    // TODO: implement this to report real gpu id backing the resource. Will be virtual implemented
    // by backend specific resource subclasses.
    //this->setMemoryBacking(traceMemoryDump, resourceName);
}

} // namespace skgpu::graphite

