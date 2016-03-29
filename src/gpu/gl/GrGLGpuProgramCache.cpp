/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLGpu.h"

#include "builders/GrGLProgramBuilder.h"
#include "GrProcessor.h"
#include "GrGLPathRendering.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "SkRTConf.h"
#include "SkTSearch.h"

#ifdef PROGRAM_CACHE_STATS
SK_CONF_DECLARE(bool, c_DisplayCache, "gpu.displayCache", false,
                "Display program cache usage.");
#endif

typedef GrGLSLProgramDataManager::UniformHandle UniformHandle;

struct GrGLGpu::ProgramCache::Entry {

    Entry() : fProgram(nullptr), fLRUStamp(0) {}

    SkAutoTUnref<GrGLProgram>   fProgram;
    unsigned int                fLRUStamp;
};

struct GrGLGpu::ProgramCache::ProgDescLess {
    bool operator() (const GrProgramDesc& desc, const Entry* entry) {
        SkASSERT(entry->fProgram.get());
        return GrProgramDesc::Less(desc, entry->fProgram->getDesc());
    }

    bool operator() (const Entry* entry, const GrProgramDesc& desc) {
        SkASSERT(entry->fProgram.get());
        return GrProgramDesc::Less(entry->fProgram->getDesc(), desc);
    }
};

GrGLGpu::ProgramCache::ProgramCache(GrGLGpu* gpu)
    : fCount(0)
    , fCurrLRUStamp(0)
    , fGpu(gpu)
#ifdef PROGRAM_CACHE_STATS
    , fTotalRequests(0)
    , fCacheMisses(0)
    , fHashMisses(0)
#endif
{
    for (int i = 0; i < 1 << kHashBits; ++i) {
        fHashTable[i] = nullptr;
    }
}

GrGLGpu::ProgramCache::~ProgramCache() {
    for (int i = 0; i < fCount; ++i){
        delete fEntries[i];
    }
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
    for (int i = 0; i < fCount; ++i) {
        SkASSERT(fEntries[i]->fProgram.get());
        fEntries[i]->fProgram->abandon();
        delete fEntries[i];
        fEntries[i] = nullptr;
    }
    fCount = 0;

    // zero out hash table
    for (int i = 0; i < 1 << kHashBits; i++) {
        fHashTable[i] = nullptr;
    }

    fCurrLRUStamp = 0;
#ifdef PROGRAM_CACHE_STATS
    fTotalRequests = 0;
    fCacheMisses = 0;
    fHashMisses = 0;
#endif
}

int GrGLGpu::ProgramCache::search(const GrProgramDesc& desc) const {
    ProgDescLess less;
    return SkTSearch(fEntries, fCount, desc, sizeof(Entry*), less);
}

GrGLProgram* GrGLGpu::ProgramCache::refProgram(const GrGLGpu* gpu,
                                               const GrPipeline& pipeline,
                                               const GrPrimitiveProcessor& primProc) {
#ifdef PROGRAM_CACHE_STATS
    ++fTotalRequests;
#endif

    // Get GrGLProgramDesc
    GrGLProgramDesc desc;
    if (!GrGLProgramDescBuilder::Build(&desc, primProc, pipeline, *gpu->glCaps().glslCaps())) {
        GrCapsDebugf(gpu->caps(), "Failed to gl program descriptor!\n");
        return nullptr;
    }

    Entry* entry = nullptr;

    uint32_t hashIdx = desc.getChecksum();
    hashIdx ^= hashIdx >> 16;
    if (kHashBits <= 8) {
        hashIdx ^= hashIdx >> 8;
    }
    hashIdx &=((1 << kHashBits) - 1);
    Entry* hashedEntry = fHashTable[hashIdx];
    if (hashedEntry && hashedEntry->fProgram->getDesc() == desc) {
        SkASSERT(hashedEntry->fProgram);
        entry = hashedEntry;
    }

    int entryIdx;
    if (nullptr == entry) {
        entryIdx = this->search(desc);
        if (entryIdx >= 0) {
            entry = fEntries[entryIdx];
#ifdef PROGRAM_CACHE_STATS
            ++fHashMisses;
#endif
        }
    }

    if (nullptr == entry) {
        // We have a cache miss
#ifdef PROGRAM_CACHE_STATS
        ++fCacheMisses;
#endif
        GrGLProgram* program = GrGLProgramBuilder::CreateProgram(pipeline, primProc, desc, fGpu);
        if (nullptr == program) {
            return nullptr;
        }
        int purgeIdx = 0;
        if (fCount < kMaxEntries) {
            entry = new Entry;
            purgeIdx = fCount++;
            fEntries[purgeIdx] = entry;
        } else {
            SkASSERT(fCount == kMaxEntries);
            purgeIdx = 0;
            for (int i = 1; i < kMaxEntries; ++i) {
                if (fEntries[i]->fLRUStamp < fEntries[purgeIdx]->fLRUStamp) {
                    purgeIdx = i;
                }
            }
            entry = fEntries[purgeIdx];
            int purgedHashIdx = entry->fProgram->getDesc().getChecksum() & ((1 << kHashBits) - 1);
            if (fHashTable[purgedHashIdx] == entry) {
                fHashTable[purgedHashIdx] = nullptr;
            }
        }
        SkASSERT(fEntries[purgeIdx] == entry);
        entry->fProgram.reset(program);
        // We need to shift fEntries around so that the entry currently at purgeIdx is placed
        // just before the entry at ~entryIdx (in order to keep fEntries sorted by descriptor).
        entryIdx = ~entryIdx;
        if (entryIdx < purgeIdx) {
            //  Let E and P be the entries at index entryIdx and purgeIdx, respectively.
            //  If the entries array looks like this:
            //       aaaaEbbbbbPccccc
            //  we rearrange it to look like this:
            //       aaaaPEbbbbbccccc
            size_t copySize = (purgeIdx - entryIdx) * sizeof(Entry*);
            memmove(fEntries + entryIdx + 1, fEntries + entryIdx, copySize);
            fEntries[entryIdx] = entry;
        } else if (purgeIdx < entryIdx) {
            //  If the entries array looks like this:
            //       aaaaPbbbbbEccccc
            //  we rearrange it to look like this:
            //       aaaabbbbbPEccccc
            size_t copySize = (entryIdx - purgeIdx - 1) * sizeof(Entry*);
            memmove(fEntries + purgeIdx, fEntries + purgeIdx + 1, copySize);
            fEntries[entryIdx - 1] = entry;
        }
#ifdef SK_DEBUG
        SkASSERT(fEntries[0]->fProgram.get());
        for (int i = 0; i < fCount - 1; ++i) {
            SkASSERT(fEntries[i + 1]->fProgram.get());
            const GrProgramDesc& a = fEntries[i]->fProgram->getDesc();
            const GrProgramDesc& b = fEntries[i + 1]->fProgram->getDesc();
            SkASSERT(GrProgramDesc::Less(a, b));
            SkASSERT(!GrProgramDesc::Less(b, a));
        }
#endif
    }

    fHashTable[hashIdx] = entry;
    entry->fLRUStamp = fCurrLRUStamp;

    if (SK_MaxU32 == fCurrLRUStamp) {
        // wrap around! just trash our LRU, one time hit.
        for (int i = 0; i < fCount; ++i) {
            fEntries[i]->fLRUStamp = 0;
        }
    }
    ++fCurrLRUStamp;
    return SkRef(entry->fProgram.get());
}
