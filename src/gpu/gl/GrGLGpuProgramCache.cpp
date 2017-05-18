/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLGpu.h"

#include "builders/GrGLProgramBuilder.h"
#include "GrProcessor.h"
#include "GrProgramDesc.h"
#include "GrGLPathRendering.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "SkTSearch.h"

#ifdef PROGRAM_CACHE_STATS
// Display program cache usage
static const bool c_DisplayCache{false};
#endif

typedef GrGLSLProgramDataManager::UniformHandle UniformHandle;

struct GrGLGpu::ProgramCache::Entry {
    Entry(sk_sp<GrGLProgram> program)
    : fProgram(std::move(program)) {}

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
#ifdef PROGRAM_CACHE_STATS
    fTotalRequests = 0;
    fCacheMisses = 0;
    fHashMisses = 0;
#endif
}

GrGLProgram* GrGLGpu::ProgramCache::refProgram(const GrGLGpu* gpu,
                                               const GrPipeline& pipeline,
                                               const GrPrimitiveProcessor& primProc,
                                               bool isPoints) {
#ifdef PROGRAM_CACHE_STATS
    ++fTotalRequests;
#endif

    // Get GrGLProgramDesc
    GrProgramDesc desc;
    if (!GrProgramDesc::Build(&desc, primProc, isPoints, pipeline, *gpu->caps()->shaderCaps())) {
        GrCapsDebugf(gpu->caps(), "Failed to gl program descriptor!\n");
        return nullptr;
    }
    desc.finalize();
    std::unique_ptr<Entry>* entry = fMap.find(desc);
    if (!entry) {
        // Didn't find an origin-independent version, check with the specific origin
        GrSurfaceOrigin origin = pipeline.getRenderTarget()->origin();
        desc.setSurfaceOriginKey(GrGLSLFragmentShaderBuilder::KeyForSurfaceOrigin(origin));
        desc.finalize();
        entry = fMap.find(desc);
    }
    if (!entry) {
        // We have a cache miss
#ifdef PROGRAM_CACHE_STATS
        ++fCacheMisses;
#endif
        GrGLProgram* program = GrGLProgramBuilder::CreateProgram(pipeline, primProc, &desc, fGpu);
        if (nullptr == program) {
            return nullptr;
        }
        entry = fMap.insert(desc, std::unique_ptr<Entry>(new Entry(sk_sp<GrGLProgram>(program))));
    }

    return SkRef((*entry)->fProgram.get());
}
