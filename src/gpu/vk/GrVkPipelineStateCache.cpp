/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "src/core/SkOpts.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrProcessor.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrStencilSettings.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkPipelineState.h"
#include "src/gpu/vk/GrVkPipelineStateBuilder.h"
#include "src/gpu/vk/GrVkResourceProvider.h"

#ifdef SK_DEBUG
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
    , fGpu(gpu) {
}

GrVkResourceProvider::PipelineStateCache::~PipelineStateCache() {
    SkASSERT(0 == fMap.count());
    // dump stats
#ifdef SK_DEBUG
    if (c_DisplayVkPipelineCache) {
        using CacheResult = GrGpu::Stats::ProgramCacheResult;

        int misses = fGpu->stats()->numInlineProgramCacheResult(CacheResult::kMiss) +
                     fGpu->stats()->numPreProgramCacheResult(CacheResult::kMiss);

        int total = misses + fGpu->stats()->numInlineProgramCacheResult(CacheResult::kHit) +
                             fGpu->stats()->numPreProgramCacheResult(CacheResult::kHit);

        SkDebugf("--- Pipeline State Cache ---\n");
        SkDebugf("Total requests: %d\n", total);
        SkDebugf("Cache misses: %d\n", misses);
        SkDebugf("Cache miss %%: %f\n", (total > 0) ? 100.f * misses / total : 0.0f);
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
#ifdef SK_DEBUG
    if (programInfo.isStencilEnabled()) {
        SkASSERT(renderTarget->getStencilAttachment());
        SkASSERT(renderTarget->numStencilBits() == 8);
        SkASSERT(renderTarget->getStencilAttachment()->numSamples() ==
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
        GrVkPipelineState* pipelineState(GrVkPipelineStateBuilder::CreatePipelineState(
                fGpu, renderTarget, desc, programInfo, compatibleRenderPass));
        if (!pipelineState) {
            return nullptr;
        }
        entry = fMap.insert(desc, std::make_unique<Entry>(fGpu, pipelineState));
        return (*entry)->fPipelineState.get();
    }
    return (*entry)->fPipelineState.get();
}
