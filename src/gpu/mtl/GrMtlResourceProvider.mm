/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlResourceProvider.h"

#include "include/gpu/GrContextOptions.h"
#include "src/gpu/GrContextPriv.h"
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
    fBufferSuballocator.reset(new BufferSuballocator(gpu->device(), kBufferSuballocatorStartSize));
    // TODO: maxBufferLength seems like a reasonable metric to determine fBufferSuballocatorMaxSize
    // but may need tuning. Might also need a GrContextOption to let the client set this.
#ifdef SK_BUILD_FOR_MAC
    int64_t maxBufferLength = 1024*1024*1024;
#else
    int64_t maxBufferLength = 256*1024*1024;
#endif
#if GR_METAL_SDK_VERSION >= 200
    if ([gpu->device() respondsToSelector:@selector(maxBufferLength)]) {
       maxBufferLength = gpu->device().maxBufferLength;
    }
#endif
    fBufferSuballocatorMaxSize = maxBufferLength/16;
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
    : fMap(gpu->getContext()->priv().options().fRuntimeProgramCacheSize)
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
    // If we knew the shader won't depend on origin, we could skip this (and use the same program
    // for both origins). Instrumenting all fragment processors would be difficult and error prone.
    desc.setSurfaceOriginKey(GrGLSLFragmentShaderBuilder::KeyForSurfaceOrigin(origin));

    std::unique_ptr<Entry>* entry = fMap.find(desc);
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

////////////////////////////////////////////////////////////////////////////////////////////////

static id<MTLBuffer> alloc_dynamic_buffer(id<MTLDevice> device, size_t size) {
    return [device newBufferWithLength: size
#ifdef SK_BUILD_FOR_MAC
                               options: MTLResourceStorageModeManaged];
#else
                               options: MTLResourceStorageModeShared];
#endif

}

// The idea here is that we create a ring buffer which is used for all dynamic allocations
// below a certain size. When a dynamic GrMtlBuffer is mapped, it grabs a portion of this
// buffer and uses it. On a subsequent map it will grab a different portion of the buffer.
// This prevents the buffer from overwriting itself before it's submitted to the command
// stream.

GrMtlResourceProvider::BufferSuballocator::BufferSuballocator(id<MTLDevice> device, size_t size)
        : fBuffer(alloc_dynamic_buffer(device, size))
        , fTotalSize(size)
        , fHead(0)
        , fTail(0) {
    // We increment fHead and fTail without bound and let overflow handle any wrapping.
    // Because of this, size needs to be a power of two.
    SkASSERT(SkIsPow2(size));
}

id<MTLBuffer> GrMtlResourceProvider::BufferSuballocator::getAllocation(size_t size,
                                                                       size_t* offset) {
    // capture current state locally (because fTail could be overwritten by the completion handler)
    size_t head, tail;
    SkAutoSpinlock lock(fMutex);
    head = fHead;
    tail = fTail;

    // The head and tail indices increment without bound, wrapping with overflow,
    // so we need to mod them down to the actual bounds of the allocation to determine
    // which blocks are available.
    size_t modHead = head & (fTotalSize - 1);
    size_t modTail = tail & (fTotalSize - 1);

    bool full = (head != tail && modHead == modTail);


    // We don't want large allocations to eat up this buffer, so we allocate them separately.
    if (full || size > fTotalSize/2) {
        return nil;
    }

    // case 1: free space lies at the beginning and/or the end of the buffer
    if (modHead >= modTail) {
        // check for room at the end
        if (fTotalSize - modHead < size) {
            // no room at the end, check the beginning
            if (modTail < size) {
                // no room at the beginning
                return nil;
            }
            // we are going to allocate from the beginning, adjust head to '0' position
            head += fTotalSize - modHead;
            modHead = 0;
        }
    // case 2: free space lies in the middle of the buffer, check for room there
    } else if (modTail - modHead < size) {
        // no room in the middle
        return nil;
    }

    *offset = modHead;
    // We're not sure what the usage of the next allocation will be --
    // to be safe we'll use 16 byte alignment.
    fHead = GrSizeAlignUp(head + size, 16);
    return fBuffer;
}

void GrMtlResourceProvider::BufferSuballocator::addCompletionHandler(
        GrMtlCommandBuffer* cmdBuffer) {
    this->ref();
    SkAutoSpinlock lock(fMutex);
    size_t newTail = fHead;
    cmdBuffer->addCompletedHandler(^(id <MTLCommandBuffer>commandBuffer) {
        // Make sure SkAutoSpinlock goes out of scope before
        // the BufferSuballocator is potentially deleted.
        {
            SkAutoSpinlock lock(fMutex);
            fTail = newTail;
        }
        this->unref();
    });
}

id<MTLBuffer> GrMtlResourceProvider::getDynamicBuffer(size_t size, size_t* offset) {
    id<MTLBuffer> buffer = fBufferSuballocator->getAllocation(size, offset);
    if (buffer) {
        return buffer;
    }

    // Try to grow allocation (old allocation will age out).
    // We grow up to a maximum size, and only grow if the requested allocation will
    // fit into half of the new buffer (to prevent very large transient buffers forcing
    // growth when they'll never fit anyway).
    if (fBufferSuballocator->size() < fBufferSuballocatorMaxSize &&
        size <= fBufferSuballocator->size()) {
        fBufferSuballocator.reset(new BufferSuballocator(fGpu->device(),
                                                         2*fBufferSuballocator->size()));
        id<MTLBuffer> buffer = fBufferSuballocator->getAllocation(size, offset);
        if (buffer) {
            return buffer;
        }
    }

    *offset = 0;
    return alloc_dynamic_buffer(fGpu->device(), size);
}

void GrMtlResourceProvider::addBufferCompletionHandler(GrMtlCommandBuffer* cmdBuffer) {
    fBufferSuballocator->addCompletionHandler(cmdBuffer);
}
