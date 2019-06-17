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
    : fGpu(gpu) {
    fPipelineStateCache.reset(new PipelineStateCache(gpu));

    // The idea here is that we create a ring buffer which is used for all dynamic allocations
    // below a certain size. When a dynamic GrMtlBuffer is mapped, it grabs a portion of this
    // buffer and uses it. On a subsequent map it will grab a different portion of the buffer.
    // This prevents the buffer from overwriting itself before it's submitted to the command
    // stream.
    static size_t kSharedDynamicBufferSize = 1024*1024;
    fBufferState.fAllocation = allocDynamicBuffer(kSharedDynamicBufferSize);
    fBufferState.fSize = kSharedDynamicBufferSize;
    fBufferState.fHead = 0;
    fBufferState.fTail = 0;
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

id<MTLBuffer> GrMtlResourceProvider::allocDynamicBuffer(size_t size) {
    return [fGpu->device() newBufferWithLength: size
#ifdef SK_BUILD_FOR_MAC
                                       options: MTLResourceStorageModeManaged];
#else
                                       options: MTLResourceStorageModeShared];
#endif
}

id<MTLBuffer> GrMtlResourceProvider::getDynamicBuffer(size_t size, size_t* offset) {
    // capture current state locally (because fTail could be overwritten by another thread)
    size_t head = fBufferState.fHead;
    const size_t tail = fBufferState.fTail;

    // The head and tail indices increment without bound, wrapping with overflow,
    // so we need to mod them down to the actual bounds of the allocation to determine
    // which blocks are available.
    size_t modHead = fBufferState.fHead & (fBufferState.fSize - 1);
    size_t modTail = tail & (fBufferState.fSize - 1);

    bool full = (fBufferState.fHead != tail && modHead == modTail);

    // We don't want large allocations to eat up this buffer, so we allocate them separately.
    if (!full && size <= fBufferState.fSize/2) {
        bool canAllocate = false;
        if (modHead >= modTail) {
            // room at the end
            if (fBufferState.fSize - modHead >= size) {
                canAllocate = true;
            // room at the front
            } else if (modTail >= size) {
                // jump to '0'
                head += fBufferState.fSize - modHead;
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

    // Make a new buffer just for this allocation
    id<MTLBuffer> buffer = allocDynamicBuffer(size);
    *offset = 0;
    return buffer;
}

void GrMtlResourceProvider::addBufferCompletionHandler(GrMtlCommandBuffer* cmdBuffer) {
    __block size_t newTail = fBufferState.fHead;
    cmdBuffer->addCompletedHandler(^(id <MTLCommandBuffer>commandBuffer) {
        this->fBufferState.fTail = newTail;
    });
}
