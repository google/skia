/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrVkResourceProvider.h"

#include "GrVkGpu.h"
#include "GrProcessor.h"
#include "GrRenderTargetPriv.h" // TODO: remove once refPipelineState gets passed stencil settings.
#include "GrVkPipelineState.h"
#include "GrVkPipelineStateBuilder.h"
#include "SkOpts.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLProgramDataManager.h"

#ifdef GR_PIPELINE_STATE_CACHE_STATS
// Display pipeline state cache usage
static const bool c_DisplayVkPipelineCache{false};
#endif

struct GrVkResourceProvider::PipelineStateCache::Entry {
    Entry(GrVkGpu* gpu, sk_sp<GrVkPipelineState> pipelineState)
    : fGpu(gpu)
    , fPipelineState(pipelineState) {}

    ~Entry() {
        if (fPipelineState) {
            fPipelineState->freeGPUResources(fGpu);
        }
    }

    GrVkGpu* fGpu;
    sk_sp<GrVkPipelineState> fPipelineState;
};

GrVkResourceProvider::PipelineStateCache::PipelineStateCache(GrVkGpu* gpu)
    : fMap(kMaxEntries)
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

sk_sp<GrVkPipelineState> GrVkResourceProvider::PipelineStateCache::refPipelineState(
                                                               const GrPipeline& pipeline,
                                                               const GrPrimitiveProcessor& primProc,
                                                               GrPrimitiveType primitiveType,
                                                               const GrVkRenderPass& renderPass) {
#ifdef GR_PIPELINE_STATE_CACHE_STATS
    ++fTotalRequests;
#endif
    GrStencilSettings stencil;
    if (pipeline.isStencilEnabled()) {
        GrRenderTarget* rt = pipeline.getRenderTarget();
        // TODO: attach stencil and create settings during render target flush.
        SkASSERT(rt->renderTargetPriv().getStencilAttachment());
        stencil.reset(*pipeline.getUserStencil(), pipeline.hasStencilClip(),
                      rt->renderTargetPriv().numStencilBits());
    }

    // Get GrVkProgramDesc
    GrVkPipelineState::Desc desc;
    if (!GrVkPipelineState::Desc::Build(&desc, primProc, pipeline, stencil,
                                        primitiveType, *fGpu->caps()->shaderCaps())) {
        GrCapsDebugf(fGpu->caps(), "Failed to build vk program descriptor!\n");
        return nullptr;
    }
    desc.finalize();

    std::unique_ptr<Entry>* entry = fMap.find(desc);
    if (!entry) {
        // Didn't find an origin-independent version, check with the specific origin
        GrSurfaceOrigin origin = pipeline.getRenderTarget()->origin();
        desc.setSurfaceOriginKey(GrGLSLFragmentShaderBuilder::KeyForSurfaceOrigin(origin));
        desc.finalize();
        entry = fMap.find(desc);
    }
    if (!entry) {
#ifdef GR_PIPELINE_STATE_CACHE_STATS
        ++fCacheMisses;
#endif
        sk_sp<GrVkPipelineState> pipelineState(
            GrVkPipelineStateBuilder::CreatePipelineState(fGpu,
                                                          pipeline,
                                                          stencil,
                                                          primProc,
                                                          primitiveType,
                                                          &desc,
                                                          renderPass));
        if (nullptr == pipelineState) {
            return nullptr;
        }
        entry = fMap.insert(desc, std::unique_ptr<Entry>(new Entry(fGpu,
                                                                   std::move(pipelineState))));
        return (*entry)->fPipelineState;
    }
    return (*entry)->fPipelineState;
}
