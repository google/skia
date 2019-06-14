/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlResourceProvider.h"

#include "src/gpu/mtl/GrMtlCommandBuffer.h"
#include "src/gpu/mtl/GrMtlGpu.h"
#include "src/gpu/mtl/GrMtlPipelineState.h"
#include "src/gpu/mtl/GrMtlUtil.h"

#include "src/sksl/SkSLCompiler.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

GrMtlResourceProvider::GrMtlResourceProvider(GrMtlGpu* gpu)
    : fGpu(gpu)
    , fBufferState({nil, 0, 0}) {
    fPipelineStateCache.reset(new PipelineStateCache(gpu));
}

GrMtlPipelineState* GrMtlResourceProvider::findOrCreateCompatiblePipelineState(
        GrRenderTarget* renderTarget, GrSurfaceOrigin origin,
        const GrPipeline& pipeline, const GrPrimitiveProcessor& proc,
        const GrTextureProxy* const primProcProxies[], GrPrimitiveType primType) {
    return fPipelineStateCache->refPipelineState(renderTarget, origin, proc, primProcProxies,
                                                 pipeline, primType);
}

////////////////////////////////////////////////////////////////////////////////////////////////

GrMtlDepthStencil* GrMtlResourceProvider::findOrCreateCompatibleDepthStencilState(
        const GrStencilSettings& stencil, GrSurfaceOrigin origin) {
    GrMtlDepthStencil* depthStencilState;
    GrMtlDepthStencil::Key key = GrMtlDepthStencil::GenerateKey(stencil, origin);
    depthStencilState = fDepthStencilStates.find(key);
    if (!depthStencilState) {
        depthStencilState = GrMtlDepthStencil::Create(fGpu, stencil, origin);
        fDepthStencilStates.add(depthStencilState);
    }
    SkASSERT(depthStencilState);
    return depthStencilState;
}

GrMtlSampler* GrMtlResourceProvider::findOrCreateCompatibleSampler(const GrSamplerState& params,
                                                                   uint32_t maxMipLevel) {
    GrMtlSampler* sampler;
    sampler = fSamplers.find(GrMtlSampler::GenerateKey(params, maxMipLevel));
    if (!sampler) {
        sampler = GrMtlSampler::Create(fGpu, params, maxMipLevel);
        fSamplers.add(sampler);
    }
    SkASSERT(sampler);
    return sampler;
}

void GrMtlResourceProvider::destroyResources() {
    // Iterate through all stored GrMtlSamplers and unref them before resetting the hash.
    SkTDynamicHash<GrMtlSampler, GrMtlSampler::Key>::Iter samplerIter(&fSamplers);
    for (; !samplerIter.done(); ++samplerIter) {
        (*samplerIter).unref();
    }
    fSamplers.reset();

    // Iterate through all stored GrMtlDepthStencils and unref them before resetting the hash.
    SkTDynamicHash<GrMtlDepthStencil, GrMtlDepthStencil::Key>::Iter dsIter(&fDepthStencilStates);
    for (; !dsIter.done(); ++dsIter) {
        (*dsIter).unref();
    }
    fDepthStencilStates.reset();

    fPipelineStateCache->release();
}

////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GR_PIPELINE_STATE_CACHE_STATS
// Display pipeline state cache usage
static const bool c_DisplayMtlPipelineCache{false};
#endif

struct GrMtlResourceProvider::PipelineStateCache::Entry {
    Entry(GrMtlGpu* gpu, GrMtlPipelineState* pipelineState)
    : fGpu(gpu)
    , fPipelineState(pipelineState) {}

    GrMtlGpu* fGpu;
    std::unique_ptr<GrMtlPipelineState> fPipelineState;
};

GrMtlResourceProvider::PipelineStateCache::PipelineStateCache(GrMtlGpu* gpu)
    : fMap(kMaxEntries)
    , fGpu(gpu)
#ifdef GR_PIPELINE_STATE_CACHE_STATS
    , fTotalRequests(0)
    , fCacheMisses(0)
#endif
{}

GrMtlResourceProvider::PipelineStateCache::~PipelineStateCache() {
    SkASSERT(0 == fMap.count());
    // dump stats
#ifdef GR_PIPELINE_STATE_CACHE_STATS
    if (c_DisplayMtlPipelineCache) {
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

void GrMtlResourceProvider::PipelineStateCache::release() {
    fMap.reset();
}

GrMtlPipelineState* GrMtlResourceProvider::PipelineStateCache::refPipelineState(
        GrRenderTarget* renderTarget,
        GrSurfaceOrigin origin,
        const GrPrimitiveProcessor& primProc,
        const GrTextureProxy* const primProcProxies[],
        const GrPipeline& pipeline,
        GrPrimitiveType primType) {
#ifdef GR_PIPELINE_STATE_CACHE_STATS
    ++fTotalRequests;
#endif
    // Get GrMtlProgramDesc
    GrMtlPipelineStateBuilder::Desc desc;
    if (!GrMtlPipelineStateBuilder::Desc::Build(&desc, renderTarget, primProc, pipeline, primType,
                                                fGpu)) {
        GrCapsDebugf(fGpu->caps(), "Failed to build mtl program descriptor!\n");
        return nullptr;
    }

    std::unique_ptr<Entry>* entry = fMap.find(desc);
    if (!entry) {
        // Didn't find an origin-independent version, check with the specific origin
        desc.setSurfaceOriginKey(GrGLSLFragmentShaderBuilder::KeyForSurfaceOrigin(origin));
        entry = fMap.find(desc);
    }
    if (!entry) {
#ifdef GR_PIPELINE_STATE_CACHE_STATS
        ++fCacheMisses;
#endif
        GrMtlPipelineState* pipelineState(GrMtlPipelineStateBuilder::CreatePipelineState(
                fGpu, renderTarget, origin, primProc, primProcProxies, pipeline, &desc));
        if (nullptr == pipelineState) {
            return nullptr;
        }
        entry = fMap.insert(desc, std::unique_ptr<Entry>(new Entry(fGpu, pipelineState)));
        return (*entry)->fPipelineState.get();
    }
    return (*entry)->fPipelineState.get();
}

id<MTLBuffer> GrMtlResourceProvider::getDynamicBuffer(size_t size, size_t* offset) {
    static size_t kRingBufferSize = 8*1024*1024;

    // The idea here is that we create a ring buffer which is used for all dynamic allocations
    // below a certain size. When a dynamic GrMtlBuffer is mapped, it grabs a portion of this
    // buffer and uses it. On a subsequent map it will grab a different portion of the buffer.
    // This prevents the buffer from overwriting itself before it's submitted to the command
    // stream.

    // Create a new buffer if we need to.
    // If the requested size is larger than the shared buffer size, then we'll
    // just make the allocation and the owning GrMtlBuffer will manage it (this
    // only happens with buffers created by GrBufferAllocPool).
    //

    // Make the initial buffer
    if (!fBufferState.fAllocation) {
        id<MTLBuffer> buffer;
        buffer = [fGpu->device() newBufferWithLength: kRingBufferSize
//#ifdef SK_BUILD_FOR_MAC
//                                             options: MTLResourceStorageModeManaged];
//#else
                                             options: MTLResourceStorageModeShared];
//#endif
        if (nil == buffer) {
            return nil;
        }

        fBufferState.fAllocation = buffer;
        fBufferState.fHead = 0;
        fBufferState.fTail = 0;
    }

    size_t head = fBufferState.fHead;
    const size_t tail = fBufferState.fTail;

    size_t modHead = (head & (kRingBufferSize - 1));
    size_t modTail = (tail & (kRingBufferSize - 1));

    bool full = (modHead == modTail && head != tail);

    // Try to put in ring buffer
    if (!full /*&& size < 32*1024*/) {
        // capture current state
        bool canAllocate = false;
        if (modHead >= modTail) {
            // room at the end
            if (kRingBufferSize - modHead >= size) {
                canAllocate = true;
            // room at the front
            } else if (modTail >= size) {
                // jump to '0'
                head += kRingBufferSize - modHead;
                modHead = 0;
                canAllocate = true;
            }
        } else if (modTail - modHead >= size) {
            canAllocate = true;
        }

        if (canAllocate) {
            *offset = modHead;
            fBufferState.fHead = head + size;
            return fBufferState.fAllocation;
        }
    }

    id<MTLBuffer> buffer = [fGpu->device() newBufferWithLength: size
//#ifdef SK_BUILD_FOR_MAC
//                                                       options: MTLResourceStorageModeManaged];
//#else
                                                       options: MTLResourceStorageModeShared];
//#endif
    *offset = 0;
    return buffer;
}

void GrMtlResourceProvider::addBufferCompletionHandler(GrMtlCommandBuffer* cmdBuffer) {
    __block size_t newTail = fBufferState.fHead;
    cmdBuffer->addCompletedHandler(^(id <MTLCommandBuffer>commandBuffer) {
        this->fBufferState.fTail = newTail;
    });
}
