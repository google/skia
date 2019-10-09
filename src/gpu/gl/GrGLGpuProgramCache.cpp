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
    fMap.foreach([](std::unique_ptr<Entry>* e) {
        if ((*e)->fProgram) {
            (*e)->fProgram->abandon();
        }
    });

    this->reset();
}

void GrGLGpu::ProgramCache::reset() {
    fMap.reset();
}

GrGLProgram* GrGLGpu::ProgramCache::refProgram(GrGLGpu* gpu,
                                               GrRenderTarget* renderTarget,
                                               const GrProgramInfo& programInfo,
                                               bool isPoints) {


    // TODO: can this be unified between GL, Vk and Mtl?
    // Get GrGLProgramDesc
    GrProgramDesc desc;
    if (!GrProgramDesc::Build(&desc, renderTarget, programInfo, isPoints, gpu)) {
        GrCapsDebugf(gpu->caps(), "Failed to gl program descriptor!\n");
        return nullptr;
    }

    std::unique_ptr<Entry>* entry = fMap.find(desc);
    if (entry && !(*entry)->fProgram) {
        // We've pre-compiled the GL program, but don't have the GrGLProgram scaffolding
        const GrGLPrecompiledProgram* precompiledProgram = &((*entry)->fPrecompiledProgram);
        SkASSERT(precompiledProgram->fProgramID != 0);
        GrGLProgram* program = GrGLProgramBuilder::CreateProgram(renderTarget, programInfo,
                                                                 &desc, fGpu,
                                                                 precompiledProgram);
        if (nullptr == program) {
            // Should we purge the program ID from the cache at this point?
            SkDEBUGFAIL("Couldn't create program from precompiled program");
            return nullptr;
        }
        (*entry)->fProgram.reset(program);
    } else if (!entry) {
        // We have a cache miss
        GrGLProgram* program = GrGLProgramBuilder::CreateProgram(renderTarget, programInfo,
                                                                 &desc, fGpu);
        if (nullptr == program) {
            return nullptr;
        }
        entry = fMap.insert(desc, std::unique_ptr<Entry>(new Entry(sk_sp<GrGLProgram>(program))));
    }

    return SkRef((*entry)->fProgram.get());
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
