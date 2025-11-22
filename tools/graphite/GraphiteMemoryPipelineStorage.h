/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GraphiteMemoryPipelineStorage_DEFINED
#define GraphiteMemoryPipelineStorage_DEFINED

#include "include/core/SkData.h"
#include "include/gpu/graphite/PersistentPipelineStorage.h"

namespace sk_gpu_test {

/**
 * This class can be used to store backend-specific Pipeline data
 */
class GraphiteMemoryPipelineStorage : public skgpu::graphite::PersistentPipelineStorage {
public:
    GraphiteMemoryPipelineStorage() = default;
    GraphiteMemoryPipelineStorage(const GraphiteMemoryPipelineStorage&) = delete;
    GraphiteMemoryPipelineStorage& operator=(const GraphiteMemoryPipelineStorage&) = delete;

    sk_sp<SkData> load() override;
    void store(const SkData& data) override;

    int numLoads() const { return fLoadCount; }
    int numStores() const { return fStoreCount; }
    void resetCacheStats() {
        fLoadCount = 0;
        fStoreCount = 0;
    }
    void reset() {
        this->resetCacheStats();
        fData.reset();
    }

private:
    int fLoadCount = 0;
    int fStoreCount = 0;
    sk_sp<SkData> fData;
};

}  // namespace sk_gpu_test

#endif // GraphiteMemoryPipelineStorage_DEFINED
