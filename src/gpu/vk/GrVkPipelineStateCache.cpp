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
#include "SkOpts.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLProgramDataManager.h"

#ifdef GR_PIPELINE_STATE_CACHE_STATS
// Display pipeline state cache usage
static const bool c_DisplayVkPipelineCache{false};
#endif

struct GrVkResourceProvider::PipelineStateCache::Entry {

    Entry() : fPipelineState(nullptr) {}

    static const GrVkPipelineState::Desc& GetKey(const Entry* entry) {
        return entry->fPipelineState->getDesc();
    }

    static uint32_t Hash(const GrVkPipelineState::Desc& key) {
        return key.getChecksum();
    }

    sk_sp<GrVkPipelineState> fPipelineState;

private:
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(Entry);
};

GrVkResourceProvider::PipelineStateCache::PipelineStateCache(GrVkGpu* gpu)
    : fCount(0)
    , fGpu(gpu)
#ifdef GR_PIPELINE_STATE_CACHE_STATS
    , fTotalRequests(0)
    , fCacheMisses(0)
#endif
{}

GrVkResourceProvider::PipelineStateCache::~PipelineStateCache() {
    SkASSERT(0 == fCount);
    // dump stats
#ifdef GR_PIPELINE_STATE_CACHE_STATS
    if (c_DisplayVkPipelineCache) {
        SkDebugf("--- Pipeline State Cache ---\n");
        SkDebugf("Total requests: %d\n", fTotalRequests);
        SkDebugf("Cache misses: %d\n", fCacheMisses);
        SkDebugf("Cache miss %%: %f\n", (fTotalRequests > 0) ?
                 100.f * fCacheMisses / fTotalRequests :
                 0.f);
        SkDebugf("---------------------\n");
    }
#endif
}

void GrVkResourceProvider::PipelineStateCache::reset() {
    fHashTable.foreach([](Entry** entry) {
        delete *entry;
    });
    fHashTable.reset();
    fCount = 0;
}

void GrVkResourceProvider::PipelineStateCache::abandon() {
    fHashTable.foreach([](Entry** entry) {
        SkASSERT((*entry)->fPipelineState.get());
        (*entry)->fPipelineState->abandonGPUResources();
    });

    this->reset();
}

void GrVkResourceProvider::PipelineStateCache::release() {
    fHashTable.foreach([this](Entry** entry) {
        SkASSERT((*entry)->fPipelineState.get());
        (*entry)->fPipelineState->freeGPUResources(fGpu);
    });

    this->reset();
}

sk_sp<GrVkPipelineState> GrVkResourceProvider::PipelineStateCache::refPipelineState(
                                                               const GrPipeline& pipeline,
                                                               const GrPrimitiveProcessor& primProc,
                                                               GrPrimitiveType primitiveType,
                                                               const GrVkRenderPass& renderPass) {
#ifdef GR_PIPELINE_STATE_CACHE_STATS
    ++fTotalRequests;
#endif
    // Get GrVkProgramDesc
    GrVkPipelineState::Desc desc;
    if (!GrVkPipelineState::Desc::Build(&desc, primProc, pipeline, primitiveType,
                                        *fGpu->vkCaps().glslCaps())) {
        GrCapsDebugf(fGpu->caps(), "Failed to build vk program descriptor!\n");
        return nullptr;
    }
    desc.finalize();

    Entry* entry = nullptr;
    if (Entry** entryptr = fHashTable.find(desc)) {
        SkASSERT(*entryptr);
        entry = *entryptr;
    }
    if (!entry) {
#ifdef GR_PIPELINE_STATE_CACHE_STATS
        ++fCacheMisses;
#endif
        sk_sp<GrVkPipelineState> pipelineState(
            GrVkPipelineStateBuilder::CreatePipelineState(fGpu,
                                                          pipeline,
                                                          primProc,
                                                          primitiveType,
                                                          desc,
                                                          renderPass));
        if (nullptr == pipelineState) {
            return nullptr;
        }
        if (fCount < kMaxEntries) {
            entry = new Entry;
            fCount++;
        } else {
            SkASSERT(fCount == kMaxEntries);
            entry = fLRUList.head();
            fLRUList.remove(entry);
            entry->fPipelineState->freeGPUResources(fGpu);
            fHashTable.remove(entry->fPipelineState->getDesc());
        }
        entry->fPipelineState = std::move(pipelineState);
        fHashTable.set(entry);
        fLRUList.addToTail(entry);
        return entry->fPipelineState;
    } else {
        fLRUList.remove(entry);
        fLRUList.addToTail(entry);
    }
    return entry->fPipelineState;
}
