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
#include "SkBase64.h"

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

#include <fstream>

class ShaderCache : public GrGLProgramBuilder::PersistentCache {
    SkString getFilename(const GrProgramDesc& key) {
        SkString result("/tmp/cache/");
        size_t bufferSize = key.keyLength() * 4 / 3 + 3;
        std::unique_ptr<char> buffer = std::unique_ptr<char>((char*) malloc(bufferSize));
        size_t length = SkBase64::Encode(key.asKey(), key.keyLength(), buffer.get(),
                               "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.@=");
        SkASSERT(length <= bufferSize);
        result.append(buffer.get(), length);
        return result;
    }

    bool load(const GrProgramDesc& key, SkSL::String* outVS, SkSL::String* outGS,
              SkSL::String* outFS) override {
        std::ifstream in(this->getFilename(key).c_str());
        if (!in.good()) {
            return false;
        }
        std::string vs;
        std::string gs;
        std::string fs;
        std::getline(in, vs, '\0');
        std::getline(in, gs, '\0');
        std::getline(in, fs, '\0');
        if (in.bad()) {
            return false;
        }
        *outVS = vs.c_str();
        *outGS = gs.c_str();
        *outFS = fs.c_str();
        return true;
    }

    void store(const GrProgramDesc& key, const SkSL::String& outVS, const SkSL::String& outGS,
               const SkSL::String& outFS) override {
        std::ofstream out(this->getFilename(key).c_str());
        if (out.bad()) {
            return;
        }
        out << outVS.c_str() << '\0';
        out << outGS.c_str() << '\0';
        out << outFS.c_str() << '\0';
    }
};

ShaderCache cache;

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
        GrSurfaceOrigin origin = pipeline.proxy()->origin();
        desc.setSurfaceOriginKey(GrGLSLFragmentShaderBuilder::KeyForSurfaceOrigin(origin));
        desc.finalize();
        entry = fMap.find(desc);
    }
    if (!entry) {
        // We have a cache miss
#ifdef PROGRAM_CACHE_STATS
        ++fCacheMisses;
#endif
        GrGLProgram* program = GrGLProgramBuilder::CreateProgram(pipeline, primProc, &desc, fGpu,
                                                                 &cache);
        if (nullptr == program) {
            return nullptr;
        }
        entry = fMap.insert(desc, std::unique_ptr<Entry>(new Entry(sk_sp<GrGLProgram>(program))));
    }

    return SkRef((*entry)->fProgram.get());
}
