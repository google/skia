/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DResourceProvider_DEFINED
#define GrD3DResourceProvider_DEFINED

#include "include/gpu/d3d/GrD3DTypes.h"
#include "include/private/SkTArray.h"
#include "src/core/SkLRUCache.h"
#include "src/gpu/GrProgramDesc.h"
#include "src/gpu/d3d/GrD3DAttachmentViewManager.h"
#include "src/gpu/d3d/GrD3DRootSignature.h"

#include <memory>

class GrD3DDirectCommandList;
class GrD3DGpu;
class GrD3DPipelineState;

class GrD3DResourceProvider {
public:
    GrD3DResourceProvider(GrD3DGpu*);

    std::unique_ptr<GrD3DDirectCommandList> findOrCreateDirectCommandList();

    void recycleDirectCommandList(std::unique_ptr<GrD3DDirectCommandList>);

    sk_sp<GrD3DRootSignature> findOrCreateRootSignature(int numTextureSamplers);

    D3D12_CPU_DESCRIPTOR_HANDLE createRenderTargetView(ID3D12Resource* textureResource);
    void recycleRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE*);

    D3D12_CPU_DESCRIPTOR_HANDLE createDepthStencilView(ID3D12Resource* textureResource);
    void recycleDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE*);

   sk_sp<GrD3DPipelineState> findOrCreateCompatiblePipelineState(GrRenderTarget*,
                                                                 const GrProgramInfo&);

private:
#ifdef SK_DEBUG
#define GR_PIPELINE_STATE_CACHE_STATS
#endif

    class PipelineStateCache : public ::SkNoncopyable {
    public:
        PipelineStateCache(GrD3DGpu* gpu);
        ~PipelineStateCache();

        sk_sp<GrD3DPipelineState> refPipelineState(GrRenderTarget*, const GrProgramInfo&);

    private:
        struct Entry;

        struct DescHash {
            uint32_t operator()(const GrProgramDesc& desc) const {
                return SkOpts::hash_fn(desc.asKey(), desc.keyLength(), 0);
            }
        };

        SkLRUCache<const GrProgramDesc, std::unique_ptr<Entry>, DescHash> fMap;

        GrD3DGpu* fGpu;

#ifdef GR_PIPELINE_STATE_CACHE_STATS
        int fTotalRequests;
        int fCacheMisses;
#endif
    };

    GrD3DGpu* fGpu;

    SkSTArray<4, std::unique_ptr<GrD3DDirectCommandList>> fAvailableDirectCommandLists;
    SkSTArray<4, sk_sp<GrD3DRootSignature>> fRootSignatures;

    GrD3DAttachmentViewManager fAttachmentViewManager;

    std::unique_ptr<PipelineStateCache> fPipelineStateCache;
};

#endif
