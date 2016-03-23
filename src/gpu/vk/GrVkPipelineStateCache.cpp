/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrVkResourceProvider.h"

#include "GrVkGpu.h"
#include "GrProcessor.h"
#include "GrVkPipelineState.h"
#include "GrVkPipelineStateBuilder.h"
#include "SkRTConf.h"
#include "SkTSearch.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLProgramDataManager.h"

#ifdef PIPELINE_STATE_CACHE_STATS
SK_CONF_DECLARE(bool, c_DisplayCache, "gpu.displayCache", false,
                "Display pipeline state cache usage.");
#endif

typedef GrGLSLProgramDataManager::UniformHandle UniformHandle;

struct GrVkResourceProvider::PipelineStateCache::Entry {

    Entry() : fPipelineState(nullptr), fLRUStamp(0) {}

    SkAutoTUnref<GrVkPipelineState> fPipelineState;
    unsigned int                    fLRUStamp;
};

struct GrVkResourceProvider::PipelineStateCache::PipelineDescLess {
    bool operator() (const GrVkPipelineState::Desc& desc, const Entry* entry) {
        SkASSERT(entry->fPipelineState.get());
        return GrVkPipelineState::Desc::Less(desc, entry->fPipelineState->getDesc());
    }

    bool operator() (const Entry* entry, const GrVkPipelineState::Desc& desc) {
        SkASSERT(entry->fPipelineState.get());
        return GrVkPipelineState::Desc::Less(entry->fPipelineState->getDesc(), desc);
    }
};

GrVkResourceProvider::PipelineStateCache::PipelineStateCache(GrVkGpu* gpu)
    : fCount(0)
    , fCurrLRUStamp(0)
    , fGpu(gpu)
#ifdef PIPELINE_STATE_CACHE_STATS
    , fTotalRequests(0)
    , fCacheMisses(0)
    , fHashMisses(0)
#endif
{
    for (int i = 0; i < 1 << kHashBits; ++i) {
        fHashTable[i] = nullptr;
    }
}

GrVkResourceProvider::PipelineStateCache::~PipelineStateCache() {
    SkASSERT(0 == fCount);
    // dump stats
#ifdef PIPELINE_STATE_CACHE_STATS
    if (c_DisplayCache) {
        SkDebugf("--- Pipeline State Cache ---\n");
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

void GrVkResourceProvider::PipelineStateCache::reset() {
    for (int i = 0; i < fCount; ++i) {
        delete fEntries[i];
        fEntries[i] = nullptr;
    }
    fCount = 0;

    // zero out hash table
    for (int i = 0; i < 1 << kHashBits; i++) {
        fHashTable[i] = nullptr;
    }

    fCurrLRUStamp = 0;
}

void GrVkResourceProvider::PipelineStateCache::abandon() {
    for (int i = 0; i < fCount; ++i) {
        SkASSERT(fEntries[i]->fPipelineState.get());
        fEntries[i]->fPipelineState->abandonGPUResources();
    }
    this->reset();
}

void GrVkResourceProvider::PipelineStateCache::release() {
    for (int i = 0; i < fCount; ++i) {
        SkASSERT(fEntries[i]->fPipelineState.get());
        fEntries[i]->fPipelineState->freeGPUResources(fGpu);
    }
    this->reset();
}

int GrVkResourceProvider::PipelineStateCache::search(const GrVkPipelineState::Desc& desc) const {
    PipelineDescLess less;
    return SkTSearch(fEntries, fCount, desc, sizeof(Entry*), less);
}

GrVkPipelineState* GrVkResourceProvider::PipelineStateCache::refPipelineState(
                                                               const GrPipeline& pipeline,
                                                               const GrPrimitiveProcessor& primProc,
                                                               GrPrimitiveType primiteType,
                                                               const GrVkRenderPass& renderPass) {
#ifdef PIPELINE_STATE_CACHE_STATS
    ++fTotalRequests;
#endif

    Entry* entry = nullptr;

    // Get GrVkProgramDesc
    GrVkPipelineState::Desc desc;
    if (!GrVkProgramDescBuilder::Build(&desc.fProgramDesc,
                                       primProc,
                                       pipeline,
                                       *fGpu->vkCaps().glslCaps())) {
        GrCapsDebugf(fGpu->caps(), "Failed to build vk program descriptor!\n");
        return false;
    }

    // Get vulkan specific descriptor key
    GrVkPipelineState::BuildStateKey(pipeline, primiteType, &desc.fStateKey);
    // Get checksum of entire PipelineDesc
    int keyLength = desc.fStateKey.count();
    SkASSERT(0 == (keyLength % 4));
    // Seed the checksum with the checksum of the programDesc then add the vulkan key to it.
    desc.fChecksum = SkChecksum::Murmur3(desc.fStateKey.begin(), keyLength,
                                         desc.fProgramDesc.getChecksum());

    uint32_t hashIdx = desc.fChecksum;
    hashIdx ^= hashIdx >> 16;
    if (kHashBits <= 8) {
        hashIdx ^= hashIdx >> 8;
    }
    hashIdx &= ((1 << kHashBits) - 1);
    Entry* hashedEntry = fHashTable[hashIdx];
    if (hashedEntry && hashedEntry->fPipelineState->getDesc() == desc) {
        SkASSERT(hashedEntry->fPipelineState);
        entry = hashedEntry;
    }

    int entryIdx;
    if (nullptr == entry) {
        entryIdx = this->search(desc);
        if (entryIdx >= 0) {
            entry = fEntries[entryIdx];
#ifdef PIPELINE_STATE_CACHE_STATS
            ++fHashMisses;
#endif
        }
    }

    if (nullptr == entry) {
        // We have a cache miss
#ifdef PIPELINE_STATE_CACHE_STATS
        ++fCacheMisses;
#endif
        GrVkPipelineState* pipelineState =
            GrVkPipelineStateBuilder::CreatePipelineState(fGpu,
                                                          pipeline,
                                                          primProc,
                                                          primiteType,
                                                          desc,
                                                          renderPass);
        if (nullptr == pipelineState) {
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
            int purgedHashIdx = entry->fPipelineState->getDesc().fChecksum & ((1 << kHashBits) - 1);
            if (fHashTable[purgedHashIdx] == entry) {
                fHashTable[purgedHashIdx] = nullptr;
            }
            entry->fPipelineState->freeGPUResources(fGpu);
        }
        SkASSERT(fEntries[purgeIdx] == entry);
        entry->fPipelineState.reset(pipelineState);
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
        SkASSERT(fEntries[0]->fPipelineState.get());
        for (int i = 0; i < fCount - 1; ++i) {
            SkASSERT(fEntries[i + 1]->fPipelineState.get());
            const GrVkPipelineState::Desc& a = fEntries[i]->fPipelineState->getDesc();
            const GrVkPipelineState::Desc& b = fEntries[i + 1]->fPipelineState->getDesc();
            SkASSERT(GrVkPipelineState::Desc::Less(a, b));
            SkASSERT(!GrVkPipelineState::Desc::Less(b, a));
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
    return SkRef(entry->fPipelineState.get());
}
