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
#include "src/gpu/GrProgramDesc.h"
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
        const GrProgramDesc& programDesc,
        const GrProgramInfo& programInfo,
        GrThreadSafePipelineBuilder::Stats::ProgramCacheResult* stat) {
    return fPipelineStateCache->refPipelineState(programDesc, programInfo, stat);
}

bool GrMtlResourceProvider::precompileShader(const SkData& key, const SkData& data) {
    return fPipelineStateCache->precompileShader(key, data);
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

struct GrMtlResourceProvider::PipelineStateCache::Entry {
    Entry(GrMtlPipelineState* pipelineState)
            : fPipelineState(pipelineState) {}
    Entry(const GrMtlPrecompiledLibraries& precompiledLibraries)
            : fPipelineState(nullptr)
            , fPrecompiledLibraries(precompiledLibraries) {}

    std::unique_ptr<GrMtlPipelineState> fPipelineState;

    // TODO: change to one library once we can build that
    GrMtlPrecompiledLibraries fPrecompiledLibraries;
};

GrMtlResourceProvider::PipelineStateCache::PipelineStateCache(GrMtlGpu* gpu)
    : fMap(gpu->getContext()->priv().options().fRuntimeProgramCacheSize)
    , fGpu(gpu) {}

GrMtlResourceProvider::PipelineStateCache::~PipelineStateCache() {
    SkASSERT(0 == fMap.count());
}

void GrMtlResourceProvider::PipelineStateCache::release() {
    fMap.reset();
}

GrMtlPipelineState* GrMtlResourceProvider::PipelineStateCache::refPipelineState(
        const GrProgramDesc& desc,
        const GrProgramInfo& programInfo,
        Stats::ProgramCacheResult* stat) {

    if (!stat) {
        // If stat is NULL we are using inline compilation rather than through DDL,
        // so we need to track those stats as well.
        GrThreadSafePipelineBuilder::Stats::ProgramCacheResult stat;
        auto tmp = this->onRefPipelineState(desc, programInfo, &stat);
        if (!tmp) {
            fStats.incNumInlineCompilationFailures();
        } else {
            fStats.incNumInlineProgramCacheResult(stat);
        }
        return tmp;
    } else {
        return this->onRefPipelineState(desc, programInfo, stat);
    }
}

GrMtlPipelineState* GrMtlResourceProvider::PipelineStateCache::onRefPipelineState(
        const GrProgramDesc& desc,
        const GrProgramInfo& programInfo,
        Stats::ProgramCacheResult* stat) {
    *stat = Stats::ProgramCacheResult::kHit;
    std::unique_ptr<Entry>* entry = fMap.find(desc);
    if (entry && !(*entry)->fPipelineState) {
        // We've pre-compiled the MSL shaders but don't have the pipelineState
        const GrMtlPrecompiledLibraries* precompiledLibs = &((*entry)->fPrecompiledLibraries);
        SkASSERT(precompiledLibs->fPipelineState);
        (*entry)->fPipelineState.reset(
                GrMtlPipelineStateBuilder::CreatePipelineState(fGpu, desc, programInfo,
                                                               precompiledLibs));
        if (!(*entry)->fPipelineState) {
            // Should we purge the precompiled shaders from the cache at this point?
            SkDEBUGFAIL("Couldn't create pipelineState from precompiled shaders");
            fStats.incNumCompilationFailures();
            return nullptr;
        }
        // release the ref on the pipeline state
        (*entry)->fPrecompiledLibraries.fPipelineState = nil;

        fStats.incNumPartialCompilationSuccesses();
        *stat = Stats::ProgramCacheResult::kPartial;
    } else if (!entry) {
        GrMtlPipelineState* pipelineState(
                GrMtlPipelineStateBuilder::CreatePipelineState(fGpu, desc, programInfo));
        if (!pipelineState) {
            fStats.incNumCompilationFailures();
           return nullptr;
        }
        fStats.incNumCompilationSuccesses();
        entry = fMap.insert(desc, std::unique_ptr<Entry>(new Entry(pipelineState)));
        *stat = Stats::ProgramCacheResult::kMiss;
        return (*entry)->fPipelineState.get();
    }
    return (*entry)->fPipelineState.get();
}

bool GrMtlResourceProvider::PipelineStateCache::precompileShader(const SkData& key,
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

    GrMtlPrecompiledLibraries precompiledLibraries;
    if (!GrMtlPipelineStateBuilder::PrecompileShaders(fGpu, data, &precompiledLibraries)) {
        return false;
    }

    fMap.insert(desc, std::make_unique<Entry>(precompiledLibraries));
    return true;

}
