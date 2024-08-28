/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/private/base/SkAssert.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkLRUCache.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrProgramDesc.h"
#include "src/gpu/ganesh/gl/GrGLGpu.h"
#include "src/gpu/ganesh/gl/GrGLProgram.h"
#include "src/gpu/ganesh/gl/builders/GrGLProgramBuilder.h"

#include <memory>
#include <utility>

class GrProgramInfo;

struct GrGLGpu::ProgramCache::Entry {
    Entry(sk_sp<GrGLProgram> program)
        : fProgram(std::move(program)) {}

    Entry(const GrGLPrecompiledProgram& precompiledProgram)
        : fPrecompiledProgram(precompiledProgram) {}

    sk_sp<GrGLProgram> fProgram;
    GrGLPrecompiledProgram fPrecompiledProgram;
};

GrGLGpu::ProgramCache::ProgramCache(int runtimeProgramCacheSize)
    : fMap(runtimeProgramCacheSize) {
}

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

sk_sp<GrGLProgram> GrGLGpu::ProgramCache::findOrCreateProgram(GrDirectContext* dContext,
                                                              const GrProgramInfo& programInfo) {
    const GrCaps* caps = dContext->priv().caps();

    GrProgramDesc desc = caps->makeDesc(/*renderTarget*/nullptr, programInfo);
    if (!desc.isValid()) {
        GrCapsDebugf(caps, "Failed to gl program descriptor!\n");
        return nullptr;
    }

    Stats::ProgramCacheResult stat;
    sk_sp<GrGLProgram> tmp = this->findOrCreateProgramImpl(dContext, desc, programInfo, &stat);
    if (!tmp) {
        fStats.incNumInlineCompilationFailures();
    } else {
        fStats.incNumInlineProgramCacheResult(stat);
    }

    return tmp;
}

sk_sp<GrGLProgram> GrGLGpu::ProgramCache::findOrCreateProgram(GrDirectContext* dContext,
                                                              const GrProgramDesc& desc,
                                                              const GrProgramInfo& programInfo,
                                                              Stats::ProgramCacheResult* stat) {
    sk_sp<GrGLProgram> tmp = this->findOrCreateProgramImpl(dContext, desc, programInfo, stat);
    if (!tmp) {
        fStats.incNumPreCompilationFailures();
    } else {
        fStats.incNumPreProgramCacheResult(*stat);
    }

    return tmp;
}

sk_sp<GrGLProgram> GrGLGpu::ProgramCache::findOrCreateProgramImpl(GrDirectContext* dContext,
                                                                  const GrProgramDesc& desc,
                                                                  const GrProgramInfo& programInfo,
                                                                  Stats::ProgramCacheResult* stat) {
    *stat = Stats::ProgramCacheResult::kHit;
    std::unique_ptr<Entry>* entry = fMap.find(desc);
    if (entry && !(*entry)->fProgram) {
        // We've pre-compiled the GL program, but don't have the GrGLProgram scaffolding
        const GrGLPrecompiledProgram* precompiledProgram = &((*entry)->fPrecompiledProgram);
        SkASSERT(precompiledProgram->fProgramID != 0);
        (*entry)->fProgram = GrGLProgramBuilder::CreateProgram(dContext, desc, programInfo,
                                                               precompiledProgram);
        if (!(*entry)->fProgram) {
            // Should we purge the program ID from the cache at this point?
            SkDEBUGFAIL("Couldn't create program from precompiled program");
            fStats.incNumCompilationFailures();
            return nullptr;
        }
        fStats.incNumPartialCompilationSuccesses();
        *stat = Stats::ProgramCacheResult::kPartial;
    } else if (!entry) {
        // We have a cache miss
        sk_sp<GrGLProgram> program = GrGLProgramBuilder::CreateProgram(dContext, desc, programInfo);
        if (!program) {
            fStats.incNumCompilationFailures();
            return nullptr;
        }
        fStats.incNumCompilationSuccesses();
        entry = fMap.insert(desc, std::make_unique<Entry>(std::move(program)));
        *stat = Stats::ProgramCacheResult::kMiss;
    }

    return (*entry)->fProgram;
}

bool GrGLGpu::ProgramCache::precompileShader(GrDirectContext* dContext,
                                             const SkData& key,
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

    GrGLPrecompiledProgram precompiledProgram;
    if (!GrGLProgramBuilder::PrecompileProgram(dContext, &precompiledProgram, data)) {
        return false;
    }

    fMap.insert(desc, std::make_unique<Entry>(precompiledProgram));
    return true;
}
