/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DGpuDescriptorTableManager_DEFINED
#define GrD3DGpuDescriptorTableManager_DEFINED

#include "src/gpu/d3d/GrD3DDescriptorHeap.h"

class GrD3DCommandList;
class GrD3DGpu;

class GrD3DDescriptorTable {
public:
    GrD3DDescriptorTable(D3D12_GPU_DESCRIPTOR_HANDLE base, size_t incrementSize,
                         unsigned int numDescriptors)
        : fBaseDescriptor(base)
        , fHandleIncrementSize(incrementSize)
        , fNumDescriptors(numDescriptors) {}

    D3D12_GPU_DESCRIPTOR_HANDLE getDescriptor(int index) {
        D3D12_GPU_DESCRIPTOR_HANDLE descriptor = fBaseDescriptor;
        descriptor.ptr += index*fHandleIncrementSize;
        return descriptor;
    }

private:
    D3D12_GPU_DESCRIPTOR_HANDLE fBaseDescriptor;
    size_t fHandleIncrementSize;
    unsigned int fNumDescriptors;
};

class GrD3DDescriptorTableManager {
public:
    GrD3DDescriptorTableManager(GrD3DGpu*);

    std::unique_ptr<GrD3DDescriptorTable> createShaderOrConstantResourceTable(GrD3DGpu*,
                                                                              unsigned int count);
    std::unique_ptr<GrD3DDescriptorTable> createSamplerTable(GrD3DGpu*, unsigned int count);

    void prepForSubmit(GrD3DCommandList*);

private:
    class Heap : public GrRecycledResource {
    public:
        static sk_sp<Heap> Make(GrD3DGpu* gpu, D3D12_DESCRIPTOR_HEAP_TYPE type,
                                unsigned int numDescriptors);

        std::unique_ptr<GrD3DDescriptorTable> allocateTable(unsigned int count);
        bool canAllocate(unsigned int count) const {
            return (fNumDescriptors - fNextAvailable) >= count;
        }
        D3D12_DESCRIPTOR_HEAP_TYPE type() const { return fType; }

        void reset() {
            fNextAvailable = 0;
        }

    private:
        Heap(GrD3DGpu* gpu, std::unique_ptr<GrD3DDescriptorHeap>& heap,
             D3D12_DESCRIPTOR_HEAP_TYPE type, unsigned int numDescriptors)
            : INHERITED()
            , fGpu(gpu)
            , fHeap(std::move(heap))
            , fType(type)
            , fNextAvailable(0)
            , fNumDescriptors(numDescriptors) {
        }

        void freeGPUData() const override {}
        void onRecycle() const override;

#ifdef SK_TRACE_MANAGED_RESOURCES
        void dumpInfo() const override {
            SkDebugf("GrD3DDescriptorTable::Heap: %d (%d refs)\n", fHeap.get(), this->getRefCnt());
        }
#endif

        GrD3DGpu* fGpu;
        std::unique_ptr<GrD3DDescriptorHeap> fHeap;
        D3D12_DESCRIPTOR_HEAP_TYPE fType;
        unsigned int fNextAvailable;
        unsigned int fNumDescriptors;

        typedef GrRecycledResource INHERITED;
    };

    class HeapPool {
    public:
        HeapPool(GrD3DGpu*, D3D12_DESCRIPTOR_HEAP_TYPE);

        std::unique_ptr<GrD3DDescriptorTable> allocateTable(GrD3DGpu*, unsigned int count);
        void reset();
        void prepForSubmit(GrD3DCommandList*);
        void recycle(sk_sp<Heap>);

    private:
        std::vector<sk_sp<Heap>> fDescriptorHeaps;
        int fMaxAvailableDescriptors;
        D3D12_DESCRIPTOR_HEAP_TYPE fHeapType;
    };

    void recycle(Heap*);

    HeapPool fCBVSRVDescriptorPool;
    HeapPool fSamplerDescriptorPool;
};

#endif
