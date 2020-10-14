/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlResourceProvider.h"

#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/mtl/GrMtlCommandBuffer.h"
#include "src/gpu/mtl/GrMtlGpu.h"
#include "src/gpu/mtl/GrMtlPipelineState.h"
#include "src/gpu/mtl/GrMtlUtil.h"

#include "src/sksl/SkSLCompiler.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

GrMtlResourceProvider::GrMtlResourceProvider(GrMtlGpu* gpu)
    : fGpu(gpu) {
    fPipelineStateCache.reset(new PipelineStateCache(gpu));
}

GrMtlPipelineState* GrMtlResourceProvider::findOrCreateCompatiblePipelineState(
        GrRenderTarget* renderTarget,
        const GrProgramInfo& programInfo) {
    return fPipelineStateCache->refPipelineState(renderTarget, programInfo);
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

GrMtlSampler* GrMtlResourceProvider::findOrCreateCompatibleSampler(GrSamplerState params) {
    GrMtlSampler* sampler;
    sampler = fSamplers.find(GrMtlSampler::GenerateKey(params));
    if (!sampler) {
        sampler = GrMtlSampler::Create(fGpu, params);
        fSamplers.add(sampler);
    }
    SkASSERT(sampler);
    return sampler;
}

void GrMtlResourceProvider::destroyResources() {
    fSamplers.foreach([&](GrMtlSampler* sampler) { sampler->unref(); });
    fSamplers.reset();

    fDepthStencilStates.foreach([&](GrMtlDepthStencil* stencil) { stencil->unref(); });
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
    : fMap(gpu->getContext()->priv().options().fRuntimeProgramCacheSize)
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
        const GrProgramInfo& programInfo) {
#ifdef GR_PIPELINE_STATE_CACHE_STATS
    ++fTotalRequests;
#endif

    const GrMtlCaps& caps = fGpu->mtlCaps();

    GrProgramDesc desc = caps.makeDesc(renderTarget, programInfo);
    if (!desc.isValid()) {
        GrCapsDebugf(fGpu->caps(), "Failed to build mtl program descriptor!\n");
        return nullptr;
    }

    std::unique_ptr<Entry>* entry = fMap.find(desc);
    if (!entry) {
#ifdef GR_PIPELINE_STATE_CACHE_STATS
        ++fCacheMisses;
#endif
        GrMtlPipelineState* pipelineState(GrMtlPipelineStateBuilder::CreatePipelineState(
            fGpu, renderTarget, desc, programInfo));
        if (!pipelineState) {
            return nullptr;
        }
        entry = fMap.insert(desc, std::unique_ptr<Entry>(new Entry(fGpu, pipelineState)));
        return (*entry)->fPipelineState.get();
    }
    return (*entry)->fPipelineState.get();
}
