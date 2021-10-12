/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DCpuDescriptorManager_DEFINED
#define GrD3DCpuDescriptorManager_DEFINED

#include "src/gpu/d3d/GrD3DDescriptorHeap.h"
#include <vector>

class GrD3DGpu;

class GrD3DCpuDescriptorManager {
public:
    GrD3DCpuDescriptorManager(GrD3DGpu*);

    GrD3DDescriptorHeap::CPUHandle createRenderTargetView(GrD3DGpu*,
                                                          ID3D12Resource* textureResource);
    void recycleRenderTargetView(const GrD3DDescriptorHeap::CPUHandle&);

    GrD3DDescriptorHeap::CPUHandle createDepthStencilView(GrD3DGpu*,
                                                          ID3D12Resource* textureResource);
    void recycleDepthStencilView(const GrD3DDescriptorHeap::CPUHandle&);

    GrD3DDescriptorHeap::CPUHandle createConstantBufferView(GrD3DGpu*,
                                                            ID3D12Resource* bufferResource,
                                                            size_t offset,
                                                            size_t size);
    GrD3DDescriptorHeap::CPUHandle createShaderResourceView(GrD3DGpu*,
                                                            ID3D12Resource* resource,
                                                            unsigned int mostDetailedMip,
                                                            unsigned int mipLevels);
    GrD3DDescriptorHeap::CPUHandle createUnorderedAccessView(GrD3DGpu*,
                                                             ID3D12Resource* resource,
                                                             unsigned int mipSlice);
    void recycleShaderView(const GrD3DDescriptorHeap::CPUHandle&);

    GrD3DDescriptorHeap::CPUHandle createSampler(GrD3DGpu*,
                                                 D3D12_FILTER filter,
                                                 float maxLOD,
                                                 D3D12_TEXTURE_ADDRESS_MODE addressModeU,
                                                 D3D12_TEXTURE_ADDRESS_MODE addressModeV);
    void recycleSampler(const GrD3DDescriptorHeap::CPUHandle&);

private:
    class Heap {
    public:
        static std::unique_ptr<Heap> Make(GrD3DGpu* gpu, D3D12_DESCRIPTOR_HEAP_TYPE type,
                                          unsigned int numDescriptors);

        GrD3DDescriptorHeap::CPUHandle allocateCPUHandle();
        void freeCPUHandle(const GrD3DDescriptorHeap::CPUHandle&);
        bool ownsHandle(const GrD3DDescriptorHeap::CPUHandle& handle) {
            return handle.fHeapID == fHeap->uniqueID();
        }

        bool canAllocate() { return fFreeCount > 0; }

    private:
        Heap(std::unique_ptr<GrD3DDescriptorHeap>& heap, unsigned int numDescriptors)
            : fHeap(std::move(heap))
            , fFreeBlocks(numDescriptors)
            , fFreeCount(numDescriptors) {
            for (unsigned int i = 0; i < numDescriptors; ++i) {
                fFreeBlocks.set(i);
            }
        }

        std::unique_ptr<GrD3DDescriptorHeap> fHeap;
        SkBitSet fFreeBlocks;
        unsigned int fFreeCount;
    };

    class HeapPool {
    public:
        HeapPool(GrD3DGpu*, D3D12_DESCRIPTOR_HEAP_TYPE);

        GrD3DDescriptorHeap::CPUHandle allocateHandle(GrD3DGpu*);
        void releaseHandle(const GrD3DDescriptorHeap::CPUHandle&);

    private:
        std::vector<std::unique_ptr<Heap>> fDescriptorHeaps;
        int fMaxAvailableDescriptors;
        D3D12_DESCRIPTOR_HEAP_TYPE fHeapType;
    };

    HeapPool fRTVDescriptorPool;
    HeapPool fDSVDescriptorPool;
    HeapPool fShaderViewDescriptorPool;
    HeapPool fSamplerDescriptorPool;
};

#endif
