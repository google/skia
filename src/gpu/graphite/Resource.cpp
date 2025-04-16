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
                   size_t gpuMemorySize,
                   bool reusableRequiresPurgeable)
        : fRefs(RefIncrement(RefType::kUsage)) // Start with 1 usage ref and no others
        , fReusableRefMask(
                (reusableRequiresPurgeable ? PurgeableMask() : RefMask(RefType::kUsage)) |
                RefMask(RefType::kReturnQueue))
        , fSharedContext(sharedContext)
        , fOwnership(ownership)
        , fUniqueID(create_unique_id())
        , fGpuMemorySize(gpuMemorySize) {
    // At initialization time, a Resource should not be considered budgeted because it does not yet
    // belong to a ResourceCache (which manages a budget). Wrapped resources and owned-but-uncached
    // resources will never be added to a cache and can therefore depend on this default value (as
    // opposed to a resource having its budget and shareable state set via registerWithCache()).
    SkASSERT(fBudgeted == Budgeted::kNo);
    SkASSERT(fShareable == Shareable::kNo);
    SkASSERT(this->isUniquelyHeld());
}

Resource::~Resource() {
    // The cache should have released or destroyed this resource.
    SkASSERT(this->wasDestroyed());
}

void Resource::registerWithCache(sk_sp<ResourceCache> returnCache,
                                 const GraphiteResourceKey& key,
                                 Budgeted initialBudgetedState,
                                 Shareable initialShareableState) {
    // ResourceCache should be registered before the Resource escapes the ResourceProvider, e.g. it
    // has a single usage ref and no others.
    SkASSERT(this->isUniquelyHeld());
    SkASSERT(!fReturnCache);
    SkASSERT(returnCache);

    fKey = key;
    fReturnCache = std::move(returnCache);

    this->addRef<RefType::kCache>();

    this->setBudgeted(initialBudgetedState);
    this->setShareable(initialShareableState);
}

bool Resource::returnToCache() const {
    // No resource should have been destroyed if there was still any sort of ref on it.
    SkASSERT(!this->wasDestroyed());

    // Not all resources are registered with the cache, but returnToCache() should only be called
    // when they have been registered.
    SkASSERT(fReturnCache);
    // In order to be returned, the Resource's "return queue" ref bit must be set. Its cache ref
    // may not be set if the cache has been shut down (but `fReturnCache` remains valid and just
    // returns false to reject the resource return).
    SkASSERT(this->hasReturnQueueRef());
    return fReturnCache->returnResource(const_cast<Resource*>(this));
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

void Resource::dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump,
                                    bool inPurgeableQueue) const {
    if (this->ownership() == Ownership::kWrapped && !traceMemoryDump->shouldDumpWrappedObjects()) {
        return;
    }

    if (this->budgeted() == skgpu::Budgeted::kNo &&
        !traceMemoryDump->shouldDumpUnbudgetedObjects()) {
        return;
    }

    size_t size = this->gpuMemorySize();

    // Dump zero-sized objects (e.g. Samplers, pipelines, etc) per traceMemoryDump implementation.
    // Always dump memoryless textures.
    if (size == 0 && !traceMemoryDump->shouldDumpSizelessObjects() &&
        this->asTexture() == nullptr) {
        return;
    }

    SkString resourceName("skia/gpu_resources/resource_");
    resourceName.appendU32(this->uniqueID().asUInt());

    traceMemoryDump->dumpNumericValue(resourceName.c_str(), "size", "bytes", size);
    traceMemoryDump->dumpStringValue(resourceName.c_str(), "type", this->getResourceType());
    traceMemoryDump->dumpStringValue(resourceName.c_str(), "label", this->getLabel().c_str());
    if (inPurgeableQueue) {
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

    this->onDumpMemoryStatistics(traceMemoryDump, resourceName.c_str());

    // TODO: implement this to report real gpu id backing the resource. Will be virtual implemented
    // by backend specific resource subclasses.
    //this->setMemoryBacking(traceMemoryDump, resourceName);
}

} // namespace skgpu::graphite
