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
static const bool c_DisplayVkPipelineCache{true};
#endif

struct GrVkResourceProvider::PipelineStateCache::Entry {
    Entry(GrVkGpu* gpu, GrVkPipelineState* pipelineState)
    : fGpu(gpu)
    , fPipelineState(pipelineState) {}

    ~Entry() {
        if (fPipelineState) {
            fPipelineState->freeGPUResources();
        }
    }

    GrVkGpu* fGpu;
    std::unique_ptr<GrVkPipelineState> fPipelineState;
};

GrVkResourceProvider::PipelineStateCache::PipelineStateCache(GrVkGpu* gpu)
    : fMap(gpu->getContext()->priv().options().fRuntimeProgramCacheSize)
    , fGpu(gpu)
#ifdef GR_PIPELINE_STATE_CACHE_STATS
    , fTotalRequests1(0)
    , fCacheMisses1(0)
#endif
{}

GrVkResourceProvider::PipelineStateCache::~PipelineStateCache() {
    SkASSERT(0 == fMap.count());
    // dump stats
#ifdef GR_PIPELINE_STATE_CACHE_STATS
    if (c_DisplayVkPipelineCache) {
        SkDebugf("--- Pipeline State Cache ---\n");
        SkDebugf("Total requests: %d\n", fTotalRequests1);
        SkDebugf("Cache misses: %d\n", fCacheMisses1);
        SkDebugf("Cache miss %%: %f\n", (fTotalRequests1 > 0) ?
                 100.f * fCacheMisses1 / fTotalRequests1 :
                 0.0f);

        int misses = fGpu->stats()->numInlineProgramCacheResult(GrGpu::Stats::ProgramCacheResult::kMiss);
        misses += fGpu->stats()->numPreProgramCacheResult(GrGpu::Stats::ProgramCacheResult::kMiss);

        int total = misses;
        total += fGpu->stats()->numInlineProgramCacheResult(GrGpu::Stats::ProgramCacheResult::kHit);
        total += fGpu->stats()->numPreProgramCacheResult(GrGpu::Stats::ProgramCacheResult::kHit);

        SkDebugf("Alt - Cache miss %%: %f\n", (total > 0) ? 100.f * misses / total : 0.0f);
        SkDebugf("---------------------\n");
    }
#endif
}

void GrVkResourceProvider::PipelineStateCache::release() {
    fMap.reset();
}

GrVkPipelineState* GrVkResourceProvider::PipelineStateCache::findOrCreatePipelineState(
        GrRenderTarget* renderTarget,
        const GrProgramInfo& programInfo,
        VkRenderPass compatibleRenderPass) {
#ifdef GR_PIPELINE_STATE_CACHE_STATS
    ++fTotalRequests1;
#endif

#ifdef SK_DEBUG
    if (programInfo.pipeline().isStencilEnabled()) {
        SkASSERT(renderTarget->renderTargetPriv().getStencilAttachment());
        SkASSERT(renderTarget->renderTargetPriv().numStencilBits() == 8);
        SkASSERT(renderTarget->renderTargetPriv().getStencilAttachment()->numSamples() ==
                 programInfo.numStencilSamples());
    }
#endif

    GrProgramDesc desc = fGpu->caps()->makeDesc(renderTarget, programInfo);
    if (!desc.isValid()) {
        GrCapsDebugf(fGpu->caps(), "Failed to build vk program descriptor!\n");
        return nullptr;
    }

    GrGpu::Stats::ProgramCacheResult stat;
    auto tmp = this->findOrCreatePipelineState(renderTarget, desc, programInfo,
                                               compatibleRenderPass, &stat);
    if (!tmp) {
        fGpu->stats()->incNumInlineCompilationFailures();
    } else {
        fGpu->stats()->incNumInlineProgramCacheResult(stat);
    }

    return tmp;
}

GrVkPipelineState* GrVkResourceProvider::PipelineStateCache::findOrCreatePipelineState(
        GrRenderTarget* renderTarget,
        const GrProgramDesc& desc,
        const GrProgramInfo& programInfo,
        VkRenderPass compatibleRenderPass,
        GrGpu::Stats::ProgramCacheResult* stat) {
    if (stat) {
        *stat = GrGpu::Stats::ProgramCacheResult::kHit;
    }

    std::unique_ptr<Entry>* entry = fMap.find(desc);
    if (!entry) {
        if (stat) {
            *stat = GrGpu::Stats::ProgramCacheResult::kMiss;
        }
#ifdef GR_PIPELINE_STATE_CACHE_STATS
        ++fCacheMisses1;
#endif
        GrVkPipelineState* pipelineState(GrVkPipelineStateBuilder::CreatePipelineState(
                fGpu, renderTarget, desc, programInfo, compatibleRenderPass));
        if (!pipelineState) {
            return nullptr;
        }
        entry = fMap.insert(desc, std::unique_ptr<Entry>(new Entry(fGpu, pipelineState)));
        return (*entry)->fPipelineState.get();
    }
    return (*entry)->fPipelineState.get();
}
