/*
* Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ThreadSafeResourceProvider.h"

#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Sampler.h"

namespace skgpu::graphite {

ThreadSafeResourceProvider::ThreadSafeResourceProvider(
        std::unique_ptr<ResourceProvider> resourceProvider)
    : fWrappedProvider(std::move(resourceProvider)) {}

sk_sp<Sampler> ThreadSafeResourceProvider::findOrCreateCompatibleSampler(const SamplerDesc& desc) {
    SkAutoSpinlock lock{fSpinLock};

    sk_sp<Sampler> sampler = fWrappedProvider->findOrCreateCompatibleSampler(desc);
    SkAssertResult(sampler->gpuMemorySize() == 0);
    return sampler;
}

#if defined(SK_DEBUG)
size_t ThreadSafeResourceProvider::getResourceCacheLimit() const {
    SkAutoSpinlock lock{fSpinLock};
    return fWrappedProvider->getResourceCacheLimit();
}

size_t ThreadSafeResourceProvider::getResourceCacheCurrentBudgetedBytes() const {
    SkAutoSpinlock lock{fSpinLock};
    return fWrappedProvider->getResourceCacheCurrentBudgetedBytes();
}

size_t ThreadSafeResourceProvider::getResourceCacheCurrentPurgeableBytes() const {
    SkAutoSpinlock lock{fSpinLock};
    return fWrappedProvider->getResourceCacheCurrentPurgeableBytes();
}
#endif

void ThreadSafeResourceProvider::dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const {
    SkAutoSpinlock lock{fSpinLock};
    fWrappedProvider->dumpMemoryStatistics(traceMemoryDump);
}

void ThreadSafeResourceProvider::freeGpuResources() {
    SkAutoSpinlock lock{fSpinLock};
    fWrappedProvider->freeGpuResources();
}

void ThreadSafeResourceProvider::purgeResourcesNotUsedSince(StdSteadyClock::time_point purgeTime) {
    SkAutoSpinlock lock{fSpinLock};
    fWrappedProvider->purgeResourcesNotUsedSince(purgeTime);
}

void ThreadSafeResourceProvider::forceProcessReturnedResources() {
    SkAutoSpinlock lock{fSpinLock};
    fWrappedProvider->forceProcessReturnedResources();
}

} // namespace skgpu::graphite

