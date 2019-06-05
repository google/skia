/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlResourceProvider.h"

#include "src/gpu/mtl/GrMtlCommandBuffer.h"
#include "src/gpu/mtl/GrMtlGpu.h"
#include "src/gpu/mtl/GrMtlPipelineState.h"
#include "src/gpu/mtl/GrMtlUtil.h"

#include "src/sksl/SkSLCompiler.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

GrMtlResourceProvider::GrMtlResourceProvider(GrMtlGpu* gpu)
    : fGpu(gpu)
    , fBufferState({nil, 0, 0}) {
    fPipelineStateCache.reset(new PipelineStateCache(gpu));
}

GrMtlPipelineState* GrMtlResourceProvider::findOrCreateCompatiblePipelineState(
        GrRenderTarget* renderTarget, GrSurfaceOrigin origin,
        const GrPipeline& pipeline, const GrPrimitiveProcessor& proc,
        const GrTextureProxy* const primProcProxies[], GrPrimitiveType primType) {
    return fPipelineStateCache->refPipelineState(renderTarget, origin, proc, primProcProxies,
                                                 pipeline, primType);
}

////////////////////////////////////////////////////////////////////////////////////////////////

GrMtlDepthStencil* GrMtlResourceProvider::findOrCreateCompatibleDepthStencilState(
        const GrStencilSettings& stencil, GrSurfaceOrigin origin) {
    GrMtlDepthStencil* depthStencilState;
    GrMtlDepthStencil::Key key = GrMtlDepthStencil::GenerateKey(stencil, origin);
    depthStencilState = fDepthStencilStates.find(key);
    if (!depthStencilState) {
        depthStencilState = GrMtlDepthStencil::Create(fGpu, stencil, origin);
        fDepthStencilStates.add(depthStencilState);
    }
    SkASSERT(depthStencilState);
    return depthStencilState;
}

GrMtlSampler* GrMtlResourceProvider::findOrCreateCompatibleSampler(const GrSamplerState& params,
                                                                   uint32_t maxMipLevel) {
    GrMtlSampler* sampler;
    sampler = fSamplers.find(GrMtlSampler::GenerateKey(params, maxMipLevel));
    if (!sampler) {
        sampler = GrMtlSampler::Create(fGpu, params, maxMipLevel);
        fSamplers.add(sampler);
    }
    SkASSERT(sampler);
    return sampler;
}

void GrMtlResourceProvider::destroyResources() {
    // Iterate through all stored GrMtlSamplers and unref them before resetting the hash.
    SkTDynamicHash<GrMtlSampler, GrMtlSampler::Key>::Iter samplerIter(&fSamplers);
    for (; !samplerIter.done(); ++samplerIter) {
        (*samplerIter).unref();
    }
    fSamplers.reset();

    // Iterate through all stored GrMtlDepthStencils and unref them before resetting the hash.
    SkTDynamicHash<GrMtlDepthStencil, GrMtlDepthStencil::Key>::Iter dsIter(&fDepthStencilStates);
    for (; !dsIter.done(); ++dsIter) {
        (*dsIter).unref();
    }
    fDepthStencilStates.reset();

    fPipelineStateCache->release();
}

////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GR_PIPELINE_STATE_CACHE_STATS
// Display pipeline state cache usage
static const bool c_DisplayMtlPipelineCache{false};
#endif

struct GrMtlResourceProvider::PipelineStateCache::Entry {
    Entry(GrMtlGpu* gpu, GrMtlPipelineState* pipelineState)
    : fGpu(gpu)
    , fPipelineState(pipelineState) {}

    GrMtlGpu* fGpu;
    std::unique_ptr<GrMtlPipelineState> fPipelineState;
};

GrMtlResourceProvider::PipelineStateCache::PipelineStateCache(GrMtlGpu* gpu)
    : fMap(kMaxEntries)
    , fGpu(gpu)
#ifdef GR_PIPELINE_STATE_CACHE_STATS
    , fTotalRequests(0)
    , fCacheMisses(0)
#endif
{}

GrMtlResourceProvider::PipelineStateCache::~PipelineStateCache() {
    SkASSERT(0 == fMap.count());
    // dump stats
#ifdef GR_PIPELINE_STATE_CACHE_STATS
    if (c_DisplayMtlPipelineCache) {
        SkDebugf("--- Pipeline State Cache ---\n");
        SkDebugf("Total requests: %d\n", fTotalRequests);
        SkDebugf("Cache misses: %d\n", fCacheMisses);
        SkDebugf("Cache miss %%: %f\n", (fTotalRequests > 0) ?
                 100.f * fCacheMisses / fTotalRequests :
                 0.f);
        SkDebugf("---------------------\n");
    }
#endif
}

void GrMtlResourceProvider::PipelineStateCache::release() {
    fMap.reset();
}

GrMtlPipelineState* GrMtlResourceProvider::PipelineStateCache::refPipelineState(
        GrRenderTarget* renderTarget,
        GrSurfaceOrigin origin,
        const GrPrimitiveProcessor& primProc,
        const GrTextureProxy* const primProcProxies[],
        const GrPipeline& pipeline,
        GrPrimitiveType primType) {
#ifdef GR_PIPELINE_STATE_CACHE_STATS
    ++fTotalRequests;
#endif
    // Get GrMtlProgramDesc
    GrMtlPipelineStateBuilder::Desc desc;
    if (!GrMtlPipelineStateBuilder::Desc::Build(&desc, renderTarget, primProc, pipeline, primType,
                                                fGpu)) {
        GrCapsDebugf(fGpu->caps(), "Failed to build mtl program descriptor!\n");
        return nullptr;
    }

    std::unique_ptr<Entry>* entry = fMap.find(desc);
    if (!entry) {
        // Didn't find an origin-independent version, check with the specific origin
        desc.setSurfaceOriginKey(GrGLSLFragmentShaderBuilder::KeyForSurfaceOrigin(origin));
        entry = fMap.find(desc);
    }
    if (!entry) {
#ifdef GR_PIPELINE_STATE_CACHE_STATS
        ++fCacheMisses;
#endif
        GrMtlPipelineState* pipelineState(GrMtlPipelineStateBuilder::CreatePipelineState(
                fGpu, renderTarget, origin, primProc, primProcProxies, pipeline, &desc));
        if (nullptr == pipelineState) {
            return nullptr;
        }
        entry = fMap.insert(desc, std::unique_ptr<Entry>(new Entry(fGpu, pipelineState)));
        return (*entry)->fPipelineState.get();
    }
    return (*entry)->fPipelineState.get();
}

id<MTLBuffer> GrMtlResourceProvider::getDynamicBuffer(size_t size, size_t* offset) {
    static size_t kSharedDynamicBufferSize = 16*1024;

    // The idea here is that we create a ring buffer which is used for all dynamic allocations
    // below a certain size. When a dynamic GrMtlBuffer is mapped, it grabs a portion of this
    // buffer and uses it. On a subsequent map it will grab a different portion of the buffer.
    // This prevents the buffer from overwriting itself before it's submitted to the command
    // stream.

    // Create a new buffer if we need to.
    // If the requested size is larger than the shared buffer size, then we'll
    // just make the allocation and the owning GrMtlBuffer will manage it (this
    // only happens with buffers created by GrBufferAllocPool).
    //
    // TODO: By sending addCompletedHandler: to MTLCommandBuffer we can track when buffers
    // are no longer in use and recycle them rather than creating a new one each time.
    if (fBufferState.fAllocationSize - fBufferState.fNextOffset < size) {
        size_t allocSize = (size >= kSharedDynamicBufferSize) ? size : kSharedDynamicBufferSize;
        id<MTLBuffer> buffer;
        buffer = [fGpu->device() newBufferWithLength: allocSize
#ifdef SK_BUILD_FOR_MAC
                                             options: MTLResourceStorageModeManaged];
#else
                                             options: MTLResourceStorageModeShared];
#endif
        if (nil == buffer) {
            return nil;
        }

        if (size >= kSharedDynamicBufferSize) {
            *offset = 0;
            return buffer;
        }

        fBufferState.fAllocation = buffer;
        fBufferState.fNextOffset = 0;
        fBufferState.fAllocationSize = kSharedDynamicBufferSize;
    }

    // Grab the next available block
    *offset = fBufferState.fNextOffset;
    fBufferState.fNextOffset += size;
    // Uniform buffer offsets need to be aligned to the nearest 256-byte boundary.
    fBufferState.fNextOffset = GrSizeAlignUp(fBufferState.fNextOffset, 256);

    return fBufferState.fAllocation;
}
