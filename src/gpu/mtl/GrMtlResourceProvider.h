/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
*/

#ifndef GrMtlResourceProvider_DEFINED
#define GrMtlResourceProvider_DEFINED

#include "include/private/SkSpinlock.h"
#include "include/private/SkTArray.h"
#include "src/core/SkLRUCache.h"
#include "src/core/SkTDynamicHash.h"
#include "src/gpu/GrProgramDesc.h"
#include "src/gpu/GrThreadSafePipelineBuilder.h"
#include "src/gpu/mtl/GrMtlDepthStencil.h"
#include "src/gpu/mtl/GrMtlPipelineStateBuilder.h"
#include "src/gpu/mtl/GrMtlSampler.h"

#import <Metal/Metal.h>

class GrMtlGpu;
class GrMtlCommandBuffer;

class GrMtlResourceProvider {
public:
    GrMtlResourceProvider(GrMtlGpu* gpu);

    GrMtlPipelineState* findOrCreateCompatiblePipelineState(
            const GrProgramDesc&,const GrProgramInfo&,
            GrThreadSafePipelineBuilder::Stats::ProgramCacheResult* stat = nullptr);
    bool precompileShader(const SkData& key, const SkData& data);

    // Finds or creates a compatible MTLDepthStencilState based on the GrStencilSettings.
    GrMtlDepthStencil* findOrCreateCompatibleDepthStencilState(const GrStencilSettings&,
                                                               GrSurfaceOrigin);

    // Finds or creates a compatible MTLSamplerState based on the GrSamplerState.
    GrMtlSampler* findOrCreateCompatibleSampler(GrSamplerState);

    // Destroy any cached resources. To be called before releasing the MtlDevice.
    void destroyResources();

#if GR_TEST_UTILS
    void resetShaderCacheForTesting() const { fPipelineStateCache->release(); }
#endif

private:
#ifdef SK_DEBUG
#define GR_PIPELINE_STATE_CACHE_STATS
#endif

    class PipelineStateCache : public GrThreadSafePipelineBuilder {
    public:
        PipelineStateCache(GrMtlGpu* gpu);
        ~PipelineStateCache() override;

        void release();
        GrMtlPipelineState* refPipelineState(const GrProgramDesc&, const GrProgramInfo&,
                                             Stats::ProgramCacheResult*);
        bool precompileShader(const SkData& key, const SkData& data);

    private:
        GrMtlPipelineState* onRefPipelineState(const GrProgramDesc&, const GrProgramInfo&,
                                               Stats::ProgramCacheResult*);

        struct Entry;

        struct DescHash {
            uint32_t operator()(const GrProgramDesc& desc) const {
                return SkOpts::hash_fn(desc.asKey(), desc.keyLength(), 0);
            }
        };

        SkLRUCache<const GrProgramDesc, std::unique_ptr<Entry>, DescHash> fMap;

        GrMtlGpu*                   fGpu;
    };

    GrMtlGpu* fGpu;

    // Cache of GrMtlPipelineStates
    std::unique_ptr<PipelineStateCache> fPipelineStateCache;

    SkTDynamicHash<GrMtlSampler, GrMtlSampler::Key> fSamplers;
    SkTDynamicHash<GrMtlDepthStencil, GrMtlDepthStencil::Key> fDepthStencilStates;
};

#endif
