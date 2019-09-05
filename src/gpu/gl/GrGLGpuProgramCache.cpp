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
    Entry(sk_sp<GrGLProgram> program) : fProgram(std::move(program)) {}

    sk_sp<GrGLProgram> fProgram;
};

GrGLGpu::ProgramCache::ProgramCache(GrGLGpu* gpu)
    : fMap(gpu->getContext()->priv().options().fRuntimeProgramCacheSize)
    , fGpu(gpu) {}

GrGLGpu::ProgramCache::~ProgramCache() {}

void GrGLGpu::ProgramCache::abandon() {
    fMap.foreach([](std::unique_ptr<Entry>* e) {
        (*e)->fProgram->abandon();
    });

    this->reset();
}

void GrGLGpu::ProgramCache::reset() {
    fMap.reset();
}

GrGLProgram* GrGLGpu::ProgramCache::refProgram(GrGLGpu* gpu,
                                               GrRenderTarget* renderTarget,
                                               GrSurfaceOrigin origin,
                                               const GrPrimitiveProcessor& primProc,
                                               const GrTextureProxy* const primProcProxies[],
                                               const GrPipeline& pipeline,
                                               bool isPoints) {
    // Get GrGLProgramDesc
    GrProgramDesc desc;
    if (!GrProgramDesc::Build(&desc, renderTarget, primProc, isPoints, pipeline, gpu)) {
        GrCapsDebugf(gpu->caps(), "Failed to gl program descriptor!\n");
        return nullptr;
    }
    // If we knew the shader won't depend on origin, we could skip this (and use the same program
    // for both origins). Instrumenting all fragment processors would be difficult and error prone.
    desc.setSurfaceOriginKey(GrGLSLFragmentShaderBuilder::KeyForSurfaceOrigin(origin));

    std::unique_ptr<Entry>* entry = fMap.find(desc);
    if (!entry) {
        // We have a cache miss
        GrGLProgram* program = GrGLProgramBuilder::CreateProgram(renderTarget, origin,
                                                                 primProc, primProcProxies,
                                                                 pipeline, &desc, fGpu);
        if (nullptr == program) {
            return nullptr;
        }
        entry = fMap.insert(desc, std::unique_ptr<Entry>(new Entry(sk_sp<GrGLProgram>(program))));
    }

    return SkRef((*entry)->fProgram.get());
}
