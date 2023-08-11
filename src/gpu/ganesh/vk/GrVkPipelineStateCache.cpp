/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrProcessor.h"
#include "src/gpu/ganesh/GrRenderTarget.h"
#include "src/gpu/ganesh/GrStencilSettings.h"
#include "src/gpu/ganesh/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/ganesh/vk/GrVkGpu.h"
#include "src/gpu/ganesh/vk/GrVkPipelineState.h"
#include "src/gpu/ganesh/vk/GrVkPipelineStateBuilder.h"
#include "src/gpu/ganesh/vk/GrVkResourceProvider.h"

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
        using CacheResult = Stats::ProgramCacheResult;

        int misses = fStats.numInlineProgramCacheResult(CacheResult::kMiss) +
                     fStats.numPreProgramCacheResult(CacheResult::kMiss);

        int total = misses + fStats.numInlineProgramCacheResult(CacheResult::kHit) +
                             fStats.numPreProgramCacheResult(CacheResult::kHit);

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
        VkRenderPass compatibleRenderPass,
        bool overrideSubpassForResolveLoad) {
#ifdef SK_DEBUG
    if (programInfo.isStencilEnabled()) {
        SkASSERT(renderTarget->getStencilAttachment(programInfo.numSamples() > 1));
        SkASSERT(renderTarget->numStencilBits(programInfo.numSamples() > 1) == 8);
        SkASSERT(renderTarget->getStencilAttachment(programInfo.numSamples() > 1)->numSamples() ==
                 programInfo.numSamples());
    }
#endif

    auto flags = overrideSubpassForResolveLoad
            ? GrCaps::ProgramDescOverrideFlags::kVulkanHasResolveLoadSubpass
            : GrCaps::ProgramDescOverrideFlags::kNone;

    GrProgramDesc desc = fGpu->caps()->makeDesc(renderTarget, programInfo, flags);
    if (!desc.isValid()) {
        GrCapsDebugf(fGpu->caps(), "Failed to build vk program descriptor!\n");
        return nullptr;
    }

    Stats::ProgramCacheResult stat;
    auto tmp = this->findOrCreatePipelineStateImpl(desc, programInfo, compatibleRenderPass,
                                                   overrideSubpassForResolveLoad, &stat);
    if (!tmp) {
        fStats.incNumInlineCompilationFailures();
    } else {
        fStats.incNumInlineProgramCacheResult(stat);
    }

    return tmp;
}

GrVkPipelineState* GrVkResourceProvider::PipelineStateCache::findOrCreatePipelineStateImpl(
        const GrProgramDesc& desc,
        const GrProgramInfo& programInfo,
        VkRenderPass compatibleRenderPass,
        bool overrideSubpassForResolveLoad,
        Stats::ProgramCacheResult* stat) {
    if (stat) {
        *stat = Stats::ProgramCacheResult::kHit;
    }

    std::unique_ptr<Entry>* entry = fMap.find(desc);
    if (!entry) {
        if (stat) {
            *stat = Stats::ProgramCacheResult::kMiss;
        }
        GrVkPipelineState* pipelineState(GrVkPipelineStateBuilder::CreatePipelineState(
                fGpu, desc, programInfo, compatibleRenderPass, overrideSubpassForResolveLoad));
        if (!pipelineState) {
            return nullptr;
        }
        entry = fMap.insert(desc, std::make_unique<Entry>(fGpu, pipelineState));
        return (*entry)->fPipelineState.get();
    }
    return (*entry)->fPipelineState.get();
}
