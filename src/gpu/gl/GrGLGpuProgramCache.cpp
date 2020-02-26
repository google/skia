/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/gl/GrGLGpu.h"

#include "include/gpu/GrContextOptions.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrProcessor.h"
#include "src/gpu/GrProgramDesc.h"
#include "src/gpu/gl/builders/GrGLProgramBuilder.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"

struct GrGLGpu::ProgramCache::Entry {
    Entry(sk_sp<GrGLProgram> program)
        : fProgram(std::move(program)) {}

    Entry(const GrGLPrecompiledProgram& precompiledProgram)
        : fPrecompiledProgram(precompiledProgram) {}

    sk_sp<GrGLProgram> fProgram;
    GrGLPrecompiledProgram fPrecompiledProgram;
};

GrGLGpu::ProgramCache::ProgramCache(GrGLGpu* gpu)
    : fMap(gpu->getContext()->priv().options().fRuntimeProgramCacheSize)
    , fGpu(gpu) {}

GrGLGpu::ProgramCache::~ProgramCache() {}

void GrGLGpu::ProgramCache::abandon() {
    fMap.foreach([](GrProgramDesc*, std::unique_ptr<Entry>* e) {
        if ((*e)->fProgram) {
            (*e)->fProgram->abandon();
        }
    });

    this->reset();
}

void GrGLGpu::ProgramCache::reset() {
    fMap.reset();
}

sk_sp<GrGLProgram> GrGLGpu::ProgramCache::findOrCreateProgram(GrRenderTarget* renderTarget,
                                                              const GrProgramInfo& programInfo) {
    const GrCaps& caps = *fGpu->caps();

    GrProgramDesc desc = caps.makeDesc(renderTarget, programInfo);
    if (!desc.isValid()) {
        GrCapsDebugf(&caps, "Failed to gl program descriptor!\n");
        return nullptr;
    }

    Stats::ProgramCacheResult stat;
    sk_sp<GrGLProgram> tmp = this->findOrCreateProgram(renderTarget, desc, programInfo, &stat);
    if (!tmp) {
        fGpu->fStats.incNumInlineCompilationFailures();
    } else {
        fGpu->fStats.incNumInlineProgramCacheResult(stat);
    }

    return tmp;
}

sk_sp<GrGLProgram> GrGLGpu::ProgramCache::findOrCreateProgram(GrRenderTarget* renderTarget,
                                                              const GrProgramDesc& desc,
                                                              const GrProgramInfo& programInfo,
                                                              Stats::ProgramCacheResult* stat) {
    *stat = Stats::ProgramCacheResult::kHit;
    std::unique_ptr<Entry>* entry = fMap.find(desc);
    if (entry && !(*entry)->fProgram) {
        // We've pre-compiled the GL program, but don't have the GrGLProgram scaffolding
        const GrGLPrecompiledProgram* precompiledProgram = &((*entry)->fPrecompiledProgram);
        SkASSERT(precompiledProgram->fProgramID != 0);
        (*entry)->fProgram = GrGLProgramBuilder::CreateProgram(fGpu, renderTarget, desc,
                                                               programInfo, precompiledProgram);
        if (!(*entry)->fProgram) {
            // Should we purge the program ID from the cache at this point?
            SkDEBUGFAIL("Couldn't create program from precompiled program");
            fGpu->fStats.incNumCompilationFailures();
            return nullptr;
        }
        fGpu->fStats.incNumPartialCompilationSuccesses();
        *stat = Stats::ProgramCacheResult::kPartial;
    } else if (!entry) {
        // We have a cache miss
        sk_sp<GrGLProgram> program = GrGLProgramBuilder::CreateProgram(fGpu, renderTarget,
                                                                       desc, programInfo);
        if (!program) {
            fGpu->fStats.incNumCompilationFailures();
            return nullptr;
        }
        fGpu->fStats.incNumCompilationSuccesses();
        entry = fMap.insert(desc, std::unique_ptr<Entry>(new Entry(std::move(program))));
        *stat = Stats::ProgramCacheResult::kMiss;
    }

    return (*entry)->fProgram;
}

bool GrGLGpu::ProgramCache::precompileShader(const SkData& key, const SkData& data) {
    GrProgramDesc desc;
    if (!GrProgramDesc::BuildFromData(&desc, key.data(), key.size())) {
        return false;
    }

    std::unique_ptr<Entry>* entry = fMap.find(desc);
    if (entry) {
        // We've already seen/compiled this shader
        return true;
    }

    GrGLPrecompiledProgram precompiledProgram;
    if (!GrGLProgramBuilder::PrecompileProgram(&precompiledProgram, fGpu, data)) {
        return false;
    }

    fMap.insert(desc, std::unique_ptr<Entry>(new Entry(precompiledProgram)));
    return true;
}
