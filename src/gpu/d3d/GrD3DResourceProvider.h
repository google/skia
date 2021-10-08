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
#include "src/gpu/GrRingBuffer.h"
#include "src/gpu/d3d/GrD3DCommandSignature.h"
#include "src/gpu/d3d/GrD3DCpuDescriptorManager.h"
#include "src/gpu/d3d/GrD3DDescriptorTableManager.h"
#include "src/gpu/d3d/GrD3DPipeline.h"
#include "src/gpu/d3d/GrD3DRootSignature.h"
#include "src/gpu/d3d/GrD3DUtil.h"

#include <memory>

class GrD3DCommandSignature;
class GrD3DDirectCommandList;
class GrD3DGpu;
class GrD3DPipelineState;
class GrD3DRenderTarget;
class GrSamplerState;

class GrD3DResourceProvider {
public:
    GrD3DResourceProvider(GrD3DGpu*);

    void destroyResources();

    std::unique_ptr<GrD3DDirectCommandList> findOrCreateDirectCommandList();

    void recycleDirectCommandList(std::unique_ptr<GrD3DDirectCommandList>);

    sk_sp<GrD3DRootSignature> findOrCreateRootSignature(int numTextureSamplers,
                                                        int numUAVs = 0);

    sk_sp<GrD3DCommandSignature> findOrCreateCommandSignature(GrD3DCommandSignature::ForIndexed,
                                                              unsigned int slot);

    GrD3DDescriptorHeap::CPUHandle createRenderTargetView(ID3D12Resource* textureResource);
    void recycleRenderTargetView(const GrD3DDescriptorHeap::CPUHandle&);

    GrD3DDescriptorHeap::CPUHandle createDepthStencilView(ID3D12Resource* textureResource);
    void recycleDepthStencilView(const GrD3DDescriptorHeap::CPUHandle&);

    GrD3DDescriptorHeap::CPUHandle createConstantBufferView(ID3D12Resource* bufferResource,
                                                            size_t offset,
                                                            size_t size);
    GrD3DDescriptorHeap::CPUHandle createShaderResourceView(ID3D12Resource* resource,
                                                            unsigned int mostDetailedMip = 0,
                                                            unsigned int mipLevels = -1);
    GrD3DDescriptorHeap::CPUHandle createUnorderedAccessView(ID3D12Resource* resource,
                                                             unsigned int mipSlice);
    void recycleShaderView(const GrD3DDescriptorHeap::CPUHandle&);

    D3D12_CPU_DESCRIPTOR_HANDLE findOrCreateCompatibleSampler(const GrSamplerState& params);

    sk_sp<GrD3DDescriptorTable> findOrCreateShaderViewTable(
            const std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>& shaderViews);
    sk_sp<GrD3DDescriptorTable> findOrCreateSamplerTable(
            const std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>& samplers);
    GrD3DDescriptorTableManager* descriptorTableMgr() {
        return &fDescriptorTableManager;
    }

    GrD3DPipelineState* findOrCreateCompatiblePipelineState(GrD3DRenderTarget*,
                                                            const GrProgramInfo&);

    sk_sp<GrD3DPipeline> findOrCreateMipmapPipeline();

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
        GrD3DPipelineState* refPipelineState(GrD3DRenderTarget*, const GrProgramInfo&);

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

    class DescriptorTableCache : public ::SkNoncopyable {
    public:
        DescriptorTableCache(GrD3DGpu* gpu) : fGpu(gpu), fMap(64) {
            // Initialize the array we pass into CopyDescriptors for ranges.
            // At the moment any descriptor we pass into CopyDescriptors is only itself,
            // not the beginning of a range, so each range size is always 1.
            for (int i = 0; i < kRangeSizesCount; ++i) {
                fRangeSizes[i] = 1;
            }
        }
        ~DescriptorTableCache() = default;

        void release();
        typedef std::function<sk_sp<GrD3DDescriptorTable>(GrD3DGpu*, unsigned int)> CreateFunc;
        sk_sp<GrD3DDescriptorTable> findOrCreateDescTable(
                const std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>&, CreateFunc);

    private:
        GrD3DGpu* fGpu;

        typedef std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> DescTableKey;
        typedef sk_sp<GrD3DDescriptorTable> DescTableValue;
        struct DescTableHash {
            uint32_t operator()(DescTableKey key) const {
                return SkOpts::hash_fn(key.data(),
                                       key.size()*sizeof(D3D12_CPU_DESCRIPTOR_HANDLE), 0);
            }
        };
        SkLRUCache<DescTableKey, DescTableValue, DescTableHash> fMap;
        inline static constexpr int kRangeSizesCount = 8;
        unsigned int fRangeSizes[kRangeSizesCount];
    };

    GrD3DGpu* fGpu;

    SkSTArray<4, std::unique_ptr<GrD3DDirectCommandList>> fAvailableDirectCommandLists;
    SkSTArray<4, sk_sp<GrD3DRootSignature>> fRootSignatures;
    SkSTArray<2, sk_sp<GrD3DCommandSignature>> fCommandSignatures;

    GrD3DCpuDescriptorManager fCpuDescriptorManager;
    GrD3DDescriptorTableManager fDescriptorTableManager;

    std::unique_ptr<PipelineStateCache> fPipelineStateCache;
    sk_sp<GrD3DPipeline> fMipmapPipeline;

    SkTHashMap<uint32_t, D3D12_CPU_DESCRIPTOR_HANDLE> fSamplers;

    DescriptorTableCache fShaderResourceDescriptorTableCache;
    DescriptorTableCache fSamplerDescriptorTableCache;
};

#endif
