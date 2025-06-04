/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_PrecompileContext_DEFINED
#define skgpu_graphite_PrecompileContext_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SingleOwner.h"
#include "include/private/base/SkAPI.h"

#include <chrono>
#include <memory>
#include <string>

class SkData;

namespace skgpu::graphite {

class SharedContext;
class PrecompileContextPriv;
class ResourceProvider;

class SK_API PrecompileContext {
public:
    ~PrecompileContext();

    /**
     * Purge Pipelines that haven't been used in the past 'msNotUsed' milliseconds
     * regardless of whether the pipeline cache is under budget.
     *
     * @param msNotUsed   Pipelines not used in these last milliseconds will be cleaned up.
     */
    void purgePipelinesNotUsedInMs(std::chrono::milliseconds msNotUsed);

    enum class StatOptions {
        // Emit histograms (using the SK_HISTOGRAM* macros) for Skia's Precompiled Pipeline
        // usage:
        //    Skia.Graphite.Precompile.NormalPreemptedByPrecompile
        //    Skia.Graphite.Precompile.UnpreemptedPrecompilePipelines
        //    Skia.Graphite.Precompile.UnusedPrecompiledPipelines
        kPrecompile,
        // Emit histograms (using the SK_HISTOGRAM* macros) for Skia's Pipeline cache usage:
        //    Skia.Graphite.PipelineCache.PipelineUsesInEpoch
        kPipelineCache,
    };

    /**
     * Emit histograms histograms related to Skia's Pipelines (c.f. the StatOptions enum).
     */
    void reportPipelineStats(StatOptions option = StatOptions::kPrecompile);

    /**
     * Precompile one specific Pipeline that has been previously serialized. Serialized pipeline
     * keys can be acquired via the ContextOptions::PipelineCallback.
     *
     * @param serializedPipelineKey   serialized Pipeline key.
     * @return                        true if a Pipeline was created from the key; false otherwise
     */
    bool precompile(sk_sp<SkData> serializedPipelineKey);

    /**
     * Get a human-readable version of a serialized pipeline key.
     *
     * @param serializedPipelineKey   serialized Pipeline key.
     * @return                        A human-readable version of the provided key; "" on failure.
     */
    std::string getPipelineLabel(sk_sp<SkData> serializedPipelineKey);

    // Provides access to functions that aren't part of the public API.
    PrecompileContextPriv priv();
    const PrecompileContextPriv priv() const;  // NOLINT(readability-const-return-type)

private:
    friend class PrecompileContextPriv;
    friend class Context; // for ctor

    PrecompileContext(sk_sp<SharedContext>);

    mutable SingleOwner fSingleOwner;
    sk_sp<SharedContext> fSharedContext;
    std::unique_ptr<ResourceProvider> fResourceProvider;
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_PrecompileContext_DEFINED
