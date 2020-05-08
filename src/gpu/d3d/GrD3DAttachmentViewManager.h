/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DAttachmentViewManager_DEFINED
#define GrD3DAttachmentViewManager_DEFINED

#include "src/gpu/d3d/GrD3DDescriptorHeap.h"

class GrD3DGpu;

class GrD3DAttachmentViewManager {
public:
    GrD3DAttachmentViewManager(GrD3DGpu*);

    D3D12_CPU_DESCRIPTOR_HANDLE createRenderTargetView(GrD3DGpu*, ID3D12Resource* textureResource);
    void recycleRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE*);

    D3D12_CPU_DESCRIPTOR_HANDLE createDepthStencilView(GrD3DGpu*, ID3D12Resource* textureResource);
    void recycleDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE*);

private:
    class Heap {
    public:
        static std::unique_ptr<Heap> Make(GrD3DGpu* gpu, D3D12_DESCRIPTOR_HEAP_TYPE type,
                                          unsigned int numDescriptors);

        D3D12_CPU_DESCRIPTOR_HANDLE allocateCPUHandle();
        bool freeCPUHandle(D3D12_CPU_DESCRIPTOR_HANDLE*);

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

        D3D12_CPU_DESCRIPTOR_HANDLE allocateHandle(GrD3DGpu*);
        void releaseHandle(D3D12_CPU_DESCRIPTOR_HANDLE*);

    private:
        std::vector<std::unique_ptr<Heap>> fDescriptorHeaps;
        int fMaxAvailableDescriptors;
        D3D12_DESCRIPTOR_HEAP_TYPE fHeapType;
    };

    HeapPool fRTVDescriptorPool;
    HeapPool fDSVDescriptorPool;
};

#endif
