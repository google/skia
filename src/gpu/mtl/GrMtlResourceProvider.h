/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
*/

#ifndef GrMtlResourceProvider_DEFINED
#define GrMtlResourceProvider_DEFINED

#include "include/private/SkTArray.h"
#include "src/core/SkLRUCache.h"
#include "src/gpu/mtl/GrMtlCopyPipelineState.h"
#include "src/gpu/mtl/GrMtlPipelineStateBuilder.h"

#import <metal/metal.h>

class GrMtlGpu;

class GrMtlResourceProvider {
public:
    GrMtlResourceProvider(GrMtlGpu* gpu);

    GrMtlCopyPipelineState* findOrCreateCopyPipelineState(MTLPixelFormat dstPixelFormat,
                                                          id<MTLFunction> vertexFunction,
                                                          id<MTLFunction> fragmentFunction,
                                                          MTLVertexDescriptor* vertexDescriptor);

    GrMtlPipelineState* findOrCreateCompatiblePipelineState(
        GrRenderTarget*, GrSurfaceOrigin,
        const GrPipeline&,
        const GrPrimitiveProcessor&,
        const GrTextureProxy* const primProcProxies[],
        GrPrimitiveType);

private:
#ifdef SK_DEBUG
#define GR_PIPELINE_STATE_CACHE_STATS
#endif

    class PipelineStateCache : public ::SkNoncopyable {
    public:
        PipelineStateCache(GrMtlGpu* gpu);
        ~PipelineStateCache();

        GrMtlPipelineState* refPipelineState(GrRenderTarget*, GrSurfaceOrigin,
                                             const GrPrimitiveProcessor&,
                                             const GrTextureProxy* const primProcProxies[],
                                             const GrPipeline&,
                                             GrPrimitiveType);

    private:
        enum {
            // We may actually have kMaxEntries+1 PipelineStates in context because we create a new
            // PipelineState before evicting from the cache.
            kMaxEntries = 128,
        };

        struct Entry;

        struct DescHash {
            uint32_t operator()(const GrProgramDesc& desc) const {
                return SkOpts::hash_fn(desc.asKey(), desc.keyLength(), 0);
            }
        };

        SkLRUCache<const GrMtlPipelineStateBuilder::Desc, std::unique_ptr<Entry>, DescHash> fMap;

        GrMtlGpu*                    fGpu;

#ifdef GR_PIPELINE_STATE_CACHE_STATS
        int                         fTotalRequests;
        int                         fCacheMisses;
#endif
    };

    SkTArray<std::unique_ptr<GrMtlCopyPipelineState>> fCopyPipelineStateCache;

    GrMtlGpu* fGpu;

    // Cache of GrMtlPipelineStates
    std::unique_ptr<PipelineStateCache> fPipelineStateCache;
};

#endif
