/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/gl/GrGLGpu.h"

#include "include/private/SkTSearch.h"
#include "src/gpu/GrProcessor.h"
#include "src/gpu/GrProgramDesc.h"
#include "src/gpu/gl/GrGLPathRendering.h"
#include "src/gpu/gl/builders/GrGLProgramBuilder.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"

#ifdef PROGRAM_CACHE_STATS
// Display program cache usage
static const bool c_DisplayCache{false};
#endif

typedef GrGLSLProgramDataManager::UniformHandle UniformHandle;

struct GrGLGpu::ProgramCache::Entry {
    Entry(sk_sp<GrGLProgram> program) : fProgram(std::move(program)) {}

    sk_sp<GrGLProgram> fProgram;
};

GrGLGpu::ProgramCache::ProgramCache(GrGLGpu* gpu)
    : fMap(kMaxEntries)
    , fGpu(gpu)
#ifdef PROGRAM_CACHE_STATS
    , fTotalRequests(0)
    , fCacheMisses(0)
    , fHashMisses(0)
#endif
{}

GrGLGpu::ProgramCache::~ProgramCache() {
    // dump stats
#ifdef PROGRAM_CACHE_STATS
    if (c_DisplayCache) {
        SkDebugf("--- Program Cache ---\n");
        SkDebugf("Total requests: %d\n", fTotalRequests);
        SkDebugf("Cache misses: %d\n", fCacheMisses);
        SkDebugf("Cache miss %%: %f\n", (fTotalRequests > 0) ?
                                            100.f * fCacheMisses / fTotalRequests :
                                            0.f);
        int cacheHits = fTotalRequests - fCacheMisses;
        SkDebugf("Hash miss %%: %f\n", (cacheHits > 0) ? 100.f * fHashMisses / cacheHits : 0.f);
        SkDebugf("---------------------\n");
    }
#endif
}

void GrGLGpu::ProgramCache::abandon() {
    fMap.foreach([](std::unique_ptr<Entry>* e) {
        (*e)->fProgram->abandon();
    });

    this->reset();
}

void GrGLGpu::ProgramCache::reset() {
#ifdef PROGRAM_CACHE_STATS
    fTotalRequests = 0;
    fCacheMisses = 0;
    fHashMisses = 0;
#endif

    fMap.reset();
}

GrGLProgram* GrGLGpu::ProgramCache::refProgram(GrGLGpu* gpu,
                                               GrRenderTarget* renderTarget,
                                               GrSurfaceOrigin origin,
                                               const GrPrimitiveProcessor& primProc,
                                               const GrTextureProxy* const primProcProxies[],
                                               const GrPipeline& pipeline,
                                               bool isPoints) {
#ifdef PROGRAM_CACHE_STATS
    ++fTotalRequests;
#endif

    // Get GrGLProgramDesc
    GrProgramDesc desc;
    if (!GrProgramDesc::Build(&desc, renderTarget, primProc, isPoints, pipeline, gpu)) {
        GrCapsDebugf(gpu->caps(), "Failed to gl program descriptor!\n");
        return nullptr;
    }
    std::unique_ptr<Entry>* entry = fMap.find(desc);
    if (!entry) {
        // Didn't find an origin-independent version, check with the specific origin
        desc.setSurfaceOriginKey(GrGLSLFragmentShaderBuilder::KeyForSurfaceOrigin(origin));
        entry = fMap.find(desc);
    }
    if (!entry) {
        // We have a cache miss
#ifdef PROGRAM_CACHE_STATS
        ++fCacheMisses;
#endif
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
