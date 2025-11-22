/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_PersistentPipelineStorage_DEFINED
#define skgpu_graphite_PersistentPipelineStorage_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAPI.h"

class SkData;

namespace skgpu::graphite {

/**
 * Abstract class which can be implemented to allow Graphite to persist Pipeline data across
 * Context lifetimes
 */
class SK_API PersistentPipelineStorage {
public:
    virtual ~PersistentPipelineStorage() = default;

    /**
     * Should return the data that had been previously stored. It should return null if there
     * is no prior data.
     */
    virtual sk_sp<SkData> load() = 0;

    /**
     * Should persist the provided Pipeline data.
     */
    virtual void store(const SkData& data) = 0;

protected:
    PersistentPipelineStorage() = default;
    PersistentPipelineStorage(const PersistentPipelineStorage&) = delete;
    PersistentPipelineStorage& operator=(const PersistentPipelineStorage&) = delete;
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_PersistentPipelineStorage_DEFINED
