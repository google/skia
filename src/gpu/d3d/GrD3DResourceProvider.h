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
#include "include/private/SkTHash.h"
#include "src/core/SkLRUCache.h"
#include "src/gpu/GrProgramDesc.h"
#include "src/gpu/d3d/GrD3DConstantRingBuffer.h"
#include "src/gpu/d3d/GrD3DCpuDescriptorManager.h"
#include "src/gpu/d3d/GrD3DDescriptorTableManager.h"
#include "src/gpu/d3d/GrD3DRootSignature.h"

#include <memory>

class GrD3DDirectCommandList;
class GrD3DGpu;
class GrD3DPipelineState;
class GrSamplerState;

class GrD3DResourceProvider {
public:
    GrD3DResourceProvider(GrD3DGpu*);

    void destroyResources();

    std::unique_ptr<GrD3DDirectCommandList> findOrCreateDirectCommandList();

    void recycleDirectCommandList(std::unique_ptr<GrD3DDirectCommandList>);

    sk_sp<GrD3DRootSignature> findOrCreateRootSignature(int numTextureSamplers);

    GrD3DDescriptorHeap::CPUHandle createRenderTargetView(ID3D12Resource* textureResource);
    void recycleRenderTargetView(const GrD3DDescriptorHeap::CPUHandle&);

    GrD3DDescriptorHeap::CPUHandle createDepthStencilView(ID3D12Resource* textureResource);
    void recycleDepthStencilView(const GrD3DDescriptorHeap::CPUHandle&);

    GrD3DDescriptorHeap::CPUHandle createConstantBufferView(ID3D12Resource* bufferResource,
                                                            size_t offset,
                                                            size_t size);
    GrD3DDescriptorHeap::CPUHandle createShaderResourceView(ID3D12Resource* resource);
    void recycleConstantOrShaderView(const GrD3DDescriptorHeap::CPUHandle&);

    D3D12_CPU_DESCRIPTOR_HANDLE findOrCreateCompatibleSampler(const GrSamplerState& params);


    std::unique_ptr<GrD3DDescriptorTable> createShaderOrConstantResourceTable(unsigned int size);
    std::unique_ptr<GrD3DDescriptorTable> createSamplerTable(unsigned int size);
    GrD3DDescriptorTableManager* descriptorTableMgr() {
        return &fDescriptorTableManager;
    }

    sk_sp<GrD3DPipelineState> findOrCreateCompatiblePipelineState(GrRenderTarget*,
                                                                 const GrProgramInfo&);

    D3D12_GPU_VIRTUAL_ADDRESS uploadConstantData(void* data, size_t size);
    void prepForSubmit();

    void markPipelineStateUniformsDirty() { fPipelineStateCache->markPipelineStateUniformsDirty(); }

#if GR_TEST_UTILS
    void resetShaderCacheForTesting() const { fPipelineStateCache->release(); }
#endif

private:
#ifdef SK_DEBUG
#define GR_PIPELINE_STATE_CACHE_STATS
#endif

    class PipelineStateCache : public ::SkNoncopyable {
    public:
        PipelineStateCache(GrD3DGpu* gpu);
        ~PipelineStateCache();

        void release();
        sk_sp<GrD3DPipelineState> refPipelineState(GrRenderTarget*, const GrProgramInfo&);

        void markPipelineStateUniformsDirty();

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

    GrD3DCpuDescriptorManager fCpuDescriptorManager;
    GrD3DDescriptorTableManager fDescriptorTableManager;

    sk_sp<GrD3DConstantRingBuffer> fConstantBuffer;

    std::unique_ptr<PipelineStateCache> fPipelineStateCache;

    SkTHashMap<uint32_t, D3D12_CPU_DESCRIPTOR_HANDLE> fSamplers;
};

#endif
