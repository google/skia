/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_PrecompileContext_DEFINED
#define skgpu_graphite_PrecompileContext_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SingleOwner.h"
#include "include/private/SkAPI.h"

#include <chrono>
#include <memory>
#include <string>

class SkData;

namespace skgpu::graphite {

class SharedContext;
class PrecompileContextPriv;
class ResourceProvider;

// The PrecompileContext is spawned from a generating Context via Context::makePrecompileContext.
// It should only be used on a single thread but that can be different from the main thread (i.e.,
// the one the Context is operating on). Many PrecompileContext's can be operating in parallel
// but the majority of the benefit will be from the threaded compilation within the
// PrecompileContext. As for that, the PipelineContext(s) borrow(s) the Executor from the
// generating Context. To make lifetime management of the Executor reasonable, if the
// Context is deleted before its PrecompileContexts, the PrecompileContexts will lose access
// to the Executor and revert to single threaded compilation.
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
     * Get a human-readable version of a serialized pipeline key and, optionally, the unique
     * hash of the Pipeline.
     *
     * @param serializedPipelineKey   serialized Pipeline key.
     * @param uniqueHash              If non-null, this will be filled in with the unique hash.
     *                                Note that the uniqueHash is only valid for the lifetime
     *                                of the Context used to create this PrecompileContext.
     * @return                        A human-readable version of the provided key; "" on failure.
     */
    std::string getPipelineLabel(sk_sp<SkData> serializedPipelineKey,
                                 uint32_t* uniqueHash = nullptr);

    enum class ExternalFormatResult {
        kInvalid,               // the serialized key was invalid
        kNoExternalFormat,
        kHasExternalFormat
    };

    /**
     * Determine if a serialized pipeline key contains a usage of an external texture format.
     *
     * @param serializedPipelineKey   serialized Pipeline key.
     * @return                        a tri-state value (see ExternalFormatResult)
     */
    ExternalFormatResult containsExternalFormat(sk_sp<SkData> serializedPipelineKey) const;

    // Provides access to functions that aren't part of the public API.
    PrecompileContextPriv priv();
    const PrecompileContextPriv priv() const;  // NOLINT(readability-const-return-type)

private:
    friend class PrecompileContextPriv;
    friend class Context; // for ctor

    explicit PrecompileContext(sk_sp<SharedContext>);

    // The PrecompileContext should not be used on multiple threads
    mutable SingleOwner fSingleOwner;
    sk_sp<SharedContext> fSharedContext;
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_PrecompileContext_DEFINED
