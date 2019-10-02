/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "include/gpu/GrContextOptions.h"
#include "src/core/SkOpts.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrProcessor.h"
#include "src/gpu/GrRenderTargetPriv.h"
#include "src/gpu/GrStencilSettings.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkPipelineState.h"
#include "src/gpu/vk/GrVkPipelineStateBuilder.h"
#include "src/gpu/vk/GrVkResourceProvider.h"

#ifdef GR_PIPELINE_STATE_CACHE_STATS
// Display pipeline state cache usage
static const bool c_DisplayVkPipelineCache{false};
#endif

struct GrVkResourceProvider::PipelineStateCache::Entry {
    Entry(GrVkGpu* gpu, GrVkPipelineState* pipelineState)
    : fGpu(gpu)
    , fPipelineState(pipelineState) {}

    ~Entry() {
        if (fPipelineState) {
            fPipelineState->freeGPUResources(fGpu);
        }
    }

    GrVkGpu* fGpu;
    std::unique_ptr<GrVkPipelineState> fPipelineState;
};

GrVkResourceProvider::PipelineStateCache::PipelineStateCache(GrVkGpu* gpu)
    : fMap(gpu->getContext()->priv().options().fRuntimeProgramCacheSize)
    , fGpu(gpu)
#ifdef GR_PIPELINE_STATE_CACHE_STATS
    , fTotalRequests(0)
    , fCacheMisses(0)
#endif
{}

GrVkResourceProvider::PipelineStateCache::~PipelineStateCache() {
    SkASSERT(0 == fMap.count());
    // dump stats
#ifdef GR_PIPELINE_STATE_CACHE_STATS
    if (c_DisplayVkPipelineCache) {
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

void GrVkResourceProvider::PipelineStateCache::abandon() {
    fMap.foreach([](std::unique_ptr<Entry>* e) {
        (*e)->fPipelineState->abandonGPUResources();
        (*e)->fPipelineState = nullptr;
    });
    fMap.reset();
}

void GrVkResourceProvider::PipelineStateCache::release() {
    fMap.reset();
}

GrVkPipelineState* GrVkResourceProvider::PipelineStateCache::refPipelineState(
        GrRenderTarget* renderTarget,
        int numSamples,
        GrSurfaceOrigin origin,
        const GrPrimitiveProcessor& primProc,
        const GrTextureProxy* const primProcProxies[],
        const GrPipeline& pipeline,
        GrPrimitiveType primitiveType,
        VkRenderPass compatibleRenderPass) {
#ifdef GR_PIPELINE_STATE_CACHE_STATS
    ++fTotalRequests;
#endif
    GrStencilSettings stencil;
    if (pipeline.isStencilEnabled()) {
        // TODO: attach stencil and create settings during render target flush.
        SkASSERT(renderTarget->renderTargetPriv().getStencilAttachment());
        stencil.reset(*pipeline.getUserStencil(), pipeline.hasStencilClip(),
                      renderTarget->renderTargetPriv().numStencilBits());
    }

    // Get GrVkProgramDesc
    GrVkPipelineStateBuilder::Desc desc;
    if (!GrVkPipelineStateBuilder::Desc::Build(&desc, renderTarget, primProc, pipeline, stencil,
                                               primitiveType, fGpu)) {
        GrCapsDebugf(fGpu->caps(), "Failed to build vk program descriptor!\n");
        return nullptr;
    }
    // If we knew the shader won't depend on origin, we could skip this (and use the same program
    // for both origins). Instrumenting all fragment processors would be difficult and error prone.
    desc.setSurfaceOriginKey(GrGLSLFragmentShaderBuilder::KeyForSurfaceOrigin(origin));

    std::unique_ptr<Entry>* entry = fMap.find(desc);
    if (!entry) {
#ifdef GR_PIPELINE_STATE_CACHE_STATS
        ++fCacheMisses;
#endif
        GrVkPipelineState* pipelineState(GrVkPipelineStateBuilder::CreatePipelineState(
                fGpu, renderTarget, numSamples, origin, primProc, primProcProxies, pipeline,
                stencil, primitiveType, &desc, compatibleRenderPass));
        if (nullptr == pipelineState) {
            return nullptr;
        }
        entry = fMap.insert(desc, std::unique_ptr<Entry>(new Entry(fGpu, pipelineState)));
        return (*entry)->fPipelineState.get();
    }
    return (*entry)->fPipelineState.get();
}
