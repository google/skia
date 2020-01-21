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
#include "src/gpu/vk/GrVkPipeline.h"
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

    Entry(GrVkGpu* gpu, const GrVkPrecompiledPipeline& precompiledPipeline)
        : fGpu(gpu)
        , fPrecompiledPipeline(precompiledPipeline) {}

    ~Entry() {
        if (fPipelineState) {
            fPipelineState->freeGPUResources(fGpu);
        } else if (fPrecompiledPipeline.fPipeline) {
            fPrecompiledPipeline.fPipeline->unref(fGpu);
        }
    }

    GrVkGpu* fGpu;
    std::unique_ptr<GrVkPipelineState> fPipelineState;
    GrVkPrecompiledPipeline fPrecompiledPipeline;
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

void GrVkResourceProvider::PipelineStateCache::release() {
    fMap.reset();
}

GrVkPipelineState* GrVkResourceProvider::PipelineStateCache::refPipelineState(
        GrRenderTarget* renderTarget,
        const GrProgramInfo& programInfo,
        VkRenderPass compatibleRenderPass) {
#ifdef GR_PIPELINE_STATE_CACHE_STATS
    ++fTotalRequests;
#endif

#ifdef SK_DEBUG
    if (programInfo.pipeline().isStencilEnabled()) {
        SkASSERT(renderTarget->renderTargetPriv().getStencilAttachment());
        SkASSERT(renderTarget->renderTargetPriv().numStencilBits() == 8);
    }
#endif

    GrProgramDesc desc = fGpu->caps()->makeDesc(renderTarget, programInfo);
    if (!desc.isValid()) {
        GrCapsDebugf(fGpu->caps(), "Failed to build vk program descriptor!\n");
        return nullptr;
    }

    std::unique_ptr<Entry>* entry = fMap.find(desc);
    if (entry && !(*entry)->fPipelineState) {
        // We've pre-compiled the Vk pipeline, but don't have the GrVkPipelineState scaffolding
        const GrVkPrecompiledPipeline* precompiledPipeline = &((*entry)->fPrecompiledPipeline);
        SkASSERT(precompiledPipeline->fPipeline != nullptr);
        SkASSERT((*entry)->fGpu == fGpu);
        GrVkPipelineState* pipelineState(GrVkPipelineStateBuilder::CreatePipelineState(
                fGpu, renderTarget, programInfo, &desc, compatibleRenderPass, precompiledPipeline));
        if (!pipelineState) {
            // Should we purge the precompiled pipeline from the cache at this point?
            SkDEBUGFAIL("Couldn't create pipeline state from precompiled pipeline");
            return nullptr;
        }
        (*entry)->fPipelineState.reset(pipelineState);
    } else if (!entry) {
#ifdef GR_PIPELINE_STATE_CACHE_STATS
        ++fCacheMisses;
#endif
        GrVkPipelineState* pipelineState(GrVkPipelineStateBuilder::CreatePipelineState(
                fGpu, renderTarget, programInfo, &desc, compatibleRenderPass));
        if (!pipelineState) {
            return nullptr;
        }
        entry = fMap.insert(desc, std::unique_ptr<Entry>(new Entry(fGpu, pipelineState)));
    }
    return (*entry)->fPipelineState.get();
}

bool GrVkResourceProvider::PipelineStateCache::precompileShader(const SkData& key,
                                                                const SkData& data) {
    GrProgramDesc desc;
    if (!GrProgramDesc::BuildFromData(&desc, key.data(), key.size())) {
        return false;
    }

    std::unique_ptr<Entry>* entry = fMap.find(desc);
    if (entry) {
        // We've already seen/compiled this shader
        return true;
    }

    GrVkPrecompiledPipeline precompiledPipeline;
    if (!GrVkPipelineStateBuilder::PrecompilePipeline(&precompiledPipeline, fGpu, data)) {
        return false;
    }

    fMap.insert(desc, std::unique_ptr<Entry>(new Entry(fGpu, precompiledPipeline)));
    return true;
}
