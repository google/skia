/*
* Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ThreadSafeResourceProvider_DEFINED
#define skgpu_graphite_ThreadSafeResourceProvider_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/base/SkSpinlock.h"
#include "src/gpu/GpuTypesPriv.h"

#include <memory>

class SkTraceMemoryDump;

namespace skgpu::graphite {

class ResourceProvider;
class Sampler;
struct SamplerDesc;

// The ThreadSafeResourceProvider wraps a backend-specific ResourceProvider and guards its
// usage via a SpinLock. It exposes the smallest API surface required.
// Note that once ResourceCaches are thread safe this could be replaced with the Context's
// ResourceProvider.
class ThreadSafeResourceProvider {
public:
    ThreadSafeResourceProvider(std::unique_ptr<ResourceProvider>);

    sk_sp<Sampler> findOrCreateCompatibleSampler(const SamplerDesc&) SK_EXCLUDES(fSpinLock);

#if defined(SK_DEBUG)
    size_t getResourceCacheLimit() const SK_EXCLUDES(fSpinLock);
    size_t getResourceCacheCurrentBudgetedBytes() const SK_EXCLUDES(fSpinLock);
    size_t getResourceCacheCurrentPurgeableBytes() const SK_EXCLUDES(fSpinLock);
#endif

    void dumpMemoryStatistics(SkTraceMemoryDump*) const SK_EXCLUDES(fSpinLock);
    void freeGpuResources() SK_EXCLUDES(fSpinLock);
    void purgeResourcesNotUsedSince(StdSteadyClock::time_point purgeTime) SK_EXCLUDES(fSpinLock);
    void forceProcessReturnedResources() SK_EXCLUDES(fSpinLock);

protected:
    mutable SkSpinlock fSpinLock;

    std::unique_ptr<ResourceProvider> fWrappedProvider SK_GUARDED_BY(fSpinLock);
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_ThreadSafeResourceProvider_DEFINED
