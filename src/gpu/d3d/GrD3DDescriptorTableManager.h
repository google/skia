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
class GrD3DDirectCommandList;
class GrD3DGpu;

class GrD3DDescriptorTable {
public:
    GrD3DDescriptorTable(D3D12_CPU_DESCRIPTOR_HANDLE baseCPU, D3D12_GPU_DESCRIPTOR_HANDLE baseGPU,
                         unsigned int numDescriptors)
        : fBaseCpuDescriptor(baseCPU)
        , fBaseGpuDescriptor(baseGPU)
#ifdef SK_DEBUG
        , fNumDescriptors(numDescriptors)
#endif
        {}

    const D3D12_CPU_DESCRIPTOR_HANDLE* baseCpuDescriptorPtr() {
        return &fBaseCpuDescriptor;
    }

    const D3D12_GPU_DESCRIPTOR_HANDLE baseGpuDescriptor() {
        return fBaseGpuDescriptor;
    }

private:
    D3D12_CPU_DESCRIPTOR_HANDLE fBaseCpuDescriptor;
    D3D12_GPU_DESCRIPTOR_HANDLE fBaseGpuDescriptor;
#ifdef SK_DEBUG
    unsigned int fNumDescriptors;
#endif
};

class GrD3DDescriptorTableManager {
public:
    GrD3DDescriptorTableManager(GrD3DGpu*);

    std::unique_ptr<GrD3DDescriptorTable> createShaderOrConstantResourceTable(GrD3DGpu*,
                                                                              unsigned int count);
    std::unique_ptr<GrD3DDescriptorTable> createSamplerTable(GrD3DGpu*, unsigned int count);

    void bindHeaps(GrD3DDirectCommandList*);
    void prepForSubmit();

private:
    class Heap : public GrRecycledResource {
    public:
        static sk_sp<Heap> Make(GrD3DGpu* gpu, D3D12_DESCRIPTOR_HEAP_TYPE type,
                                unsigned int numDescriptors);

        std::unique_ptr<GrD3DDescriptorTable> allocateTable(unsigned int count);
        bool canAllocate(unsigned int count) const {
            return (fDescriptorCount - fNextAvailable) >= count;
        }
        ID3D12DescriptorHeap* d3dDescriptorHeap() const { return fHeap->descriptorHeap(); }
        D3D12_DESCRIPTOR_HEAP_TYPE type() const { return fType; }
        unsigned int descriptorCount() { return fDescriptorCount; }

        void reset() {
            fNextAvailable = 0;
        }

    private:
        Heap(GrD3DGpu* gpu, std::unique_ptr<GrD3DDescriptorHeap>& heap,
             D3D12_DESCRIPTOR_HEAP_TYPE type, unsigned int descriptorCount)
            : INHERITED()
            , fGpu(gpu)
            , fHeap(std::move(heap))
            , fType(type)
            , fDescriptorCount(descriptorCount)
            , fNextAvailable(0) {
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
        unsigned int fDescriptorCount;
        unsigned int fNextAvailable;

        typedef GrRecycledResource INHERITED;
    };

    class HeapPool {
    public:
        HeapPool(GrD3DGpu*, D3D12_DESCRIPTOR_HEAP_TYPE);

        std::unique_ptr<GrD3DDescriptorTable> allocateTable(GrD3DGpu*, unsigned int count);
        void addResource(GrD3DCommandList*);
        void recycle(sk_sp<Heap>);
        ID3D12DescriptorHeap* currentD3DDescriptorHeap() const;
        //*** fix these betterer
        bool bindingNeedsUpdate() const { return fBindingNeedsUpdate; }
        void prepForSubmit() { fBindingNeedsUpdate = true; }

    private:
        static constexpr int kInitialHeapDescriptorCount = 256;

        std::vector<sk_sp<Heap>> fDescriptorHeaps;
        D3D12_DESCRIPTOR_HEAP_TYPE fHeapType;
        unsigned int fCurrentHeapDescriptorCount;
        bool fBindingNeedsUpdate;
    };

    void recycle(Heap*);

    HeapPool fCBVSRVDescriptorPool;
    HeapPool fSamplerDescriptorPool;
};

#endif
