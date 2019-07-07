/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
*/

#ifndef GrMtlResourceProvider_DEFINED
#define GrMtlResourceProvider_DEFINED

#include "include/private/SkSpinlock.h"
#include "include/private/SkTArray.h"
#include "src/core/SkLRUCache.h"
#include "src/gpu/mtl/GrMtlDepthStencil.h"
#include "src/gpu/mtl/GrMtlPipelineStateBuilder.h"
#include "src/gpu/mtl/GrMtlSampler.h"

#import <Metal/Metal.h>

class GrMtlGpu;
class GrMtlCommandBuffer;

class GrMtlResourceProvider {
public:
    GrMtlResourceProvider(GrMtlGpu* gpu);

    GrMtlPipelineState* findOrCreateCompatiblePipelineState(
        GrRenderTarget*, GrSurfaceOrigin,
        const GrPipeline&,
        const GrPrimitiveProcessor&,
        const GrTextureProxy* const primProcProxies[],
        GrPrimitiveType);

    // Finds or creates a compatible MTLDepthStencilState based on the GrStencilSettings.
    GrMtlDepthStencil* findOrCreateCompatibleDepthStencilState(const GrStencilSettings&,
                                                               GrSurfaceOrigin);

    // Finds or creates a compatible MTLSamplerState based on the GrSamplerState.
    GrMtlSampler* findOrCreateCompatibleSampler(const GrSamplerState&, uint32_t maxMipLevel);

    id<MTLBuffer> getDynamicBuffer(size_t size, size_t* offset);
    void addBufferCompletionHandler(GrMtlCommandBuffer* cmdBuffer);

    // Destroy any cached resources. To be called before releasing the MtlDevice.
    void destroyResources();

private:
#ifdef SK_DEBUG
#define GR_PIPELINE_STATE_CACHE_STATS
#endif

    class PipelineStateCache : public ::SkNoncopyable {
    public:
        PipelineStateCache(GrMtlGpu* gpu);
        ~PipelineStateCache();

        void release();
        GrMtlPipelineState* refPipelineState(GrRenderTarget*, GrSurfaceOrigin,
                                             const GrPrimitiveProcessor&,
                                             const GrTextureProxy* const primProcProxies[],
                                             const GrPipeline&,
                                             GrPrimitiveType);

    private:
        enum {
            // We may actually have kMaxEntries+1 PipelineStates in context because we create a new
            // PipelineState before evicting from the cache.
            kMaxEntries = 128,
        };

        struct Entry;

        struct DescHash {
            uint32_t operator()(const GrProgramDesc& desc) const {
                return SkOpts::hash_fn(desc.asKey(), desc.keyLength(), 0);
            }
        };

        SkLRUCache<const GrMtlPipelineStateBuilder::Desc, std::unique_ptr<Entry>, DescHash> fMap;

        GrMtlGpu*                    fGpu;

#ifdef GR_PIPELINE_STATE_CACHE_STATS
        int                         fTotalRequests;
        int                         fCacheMisses;
#endif
    };

    // Buffer allocator
    class BufferSuballocator : public SkRefCnt {
    public:
        BufferSuballocator(id<MTLDevice> device, size_t size);
        ~BufferSuballocator() {
            fBuffer = nil;
            fTotalSize = 0;
        }

        id<MTLBuffer> getAllocation(size_t size, size_t* offset);
        void addCompletionHandler(GrMtlCommandBuffer* cmdBuffer);
        size_t size() { return fTotalSize; }

    private:
        id<MTLBuffer> fBuffer;
        size_t        fTotalSize;
        size_t        fHead SK_GUARDED_BY(fMutex);     // where we start allocating
        size_t        fTail SK_GUARDED_BY(fMutex);     // where we start deallocating
        SkSpinlock    fMutex;
    };
    static constexpr size_t kBufferSuballocatorStartSize = 1024*1024;
    static constexpr size_t kBufferSuballocatorMaxSize = 8*1024*1024;

    GrMtlGpu* fGpu;

    // Cache of GrMtlPipelineStates
    std::unique_ptr<PipelineStateCache> fPipelineStateCache;

    SkTDynamicHash<GrMtlSampler, GrMtlSampler::Key> fSamplers;
    SkTDynamicHash<GrMtlDepthStencil, GrMtlDepthStencil::Key> fDepthStencilStates;

    // This is ref-counted because we might delete the GrContext before the command buffer
    // finishes. The completion handler will retain a reference to this so it won't get
    // deleted along with the GrContext.
    sk_sp<BufferSuballocator> fBufferSuballocator;
};

#endif
