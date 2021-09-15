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

class GrD3DDescriptorTable : public SkRefCnt {
public:
    GrD3DDescriptorTable(D3D12_CPU_DESCRIPTOR_HANDLE baseCPU, D3D12_GPU_DESCRIPTOR_HANDLE baseGPU,
                         D3D12_DESCRIPTOR_HEAP_TYPE type)
        : fDescriptorTableCpuStart(baseCPU)
        , fDescriptorTableGpuStart(baseGPU)
        , fType(type) {}

    const D3D12_CPU_DESCRIPTOR_HANDLE* baseCpuDescriptorPtr() {
        return &fDescriptorTableCpuStart;
    }

    const D3D12_GPU_DESCRIPTOR_HANDLE baseGpuDescriptor() {
        return fDescriptorTableGpuStart;
    }

    D3D12_DESCRIPTOR_HEAP_TYPE type() const { return fType; }

private:
    D3D12_CPU_DESCRIPTOR_HANDLE fDescriptorTableCpuStart;
    D3D12_GPU_DESCRIPTOR_HANDLE fDescriptorTableGpuStart;
    D3D12_DESCRIPTOR_HEAP_TYPE fType;
};

class GrD3DDescriptorTableManager {
public:
    GrD3DDescriptorTableManager(GrD3DGpu*);

    sk_sp<GrD3DDescriptorTable> createShaderViewTable(GrD3DGpu*, unsigned int count);
    sk_sp<GrD3DDescriptorTable> createSamplerTable(GrD3DGpu*, unsigned int count);

    void prepForSubmit(GrD3DGpu* gpu);

private:
    class Heap : public GrRecycledResource {
    public:
        static sk_sp<Heap> Make(GrD3DGpu* gpu, D3D12_DESCRIPTOR_HEAP_TYPE type,
                                unsigned int numDescriptors);

        sk_sp<GrD3DDescriptorTable> allocateTable(unsigned int count);
        bool canAllocate(unsigned int count) const {
            return (fDescriptorCount - fNextAvailable) >= count;
        }
        ID3D12DescriptorHeap* d3dDescriptorHeap() const { return fHeap->descriptorHeap(); }
        D3D12_DESCRIPTOR_HEAP_TYPE type() const { return fType; }
        unsigned int descriptorCount() { return fDescriptorCount; }
        bool used() { return fNextAvailable > 0; }

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
            SkDebugf("GrD3DDescriptorTable::Heap: %p (%d refs)\n", fHeap.get(), this->getRefCnt());
        }
#endif

        GrD3DGpu* fGpu;
        std::unique_ptr<GrD3DDescriptorHeap> fHeap;
        D3D12_DESCRIPTOR_HEAP_TYPE fType;
        unsigned int fDescriptorCount;
        unsigned int fNextAvailable;

        using INHERITED = GrRecycledResource;
    };

    class HeapPool {
    public:
        HeapPool(GrD3DGpu*, D3D12_DESCRIPTOR_HEAP_TYPE);

        sk_sp<GrD3DDescriptorTable> allocateTable(GrD3DGpu*, unsigned int count);
        void recycle(sk_sp<Heap>);
        sk_sp<Heap>& currentDescriptorHeap();
        void prepForSubmit(GrD3DGpu* gpu);

    private:
        static constexpr int kInitialHeapDescriptorCount = 256;

        std::vector<sk_sp<Heap>> fDescriptorHeaps;
        D3D12_DESCRIPTOR_HEAP_TYPE fHeapType;
        unsigned int fCurrentHeapDescriptorCount;
    };

    void setHeaps(GrD3DGpu*);
    void recycle(Heap*);

    HeapPool fShaderViewDescriptorPool;
    HeapPool fSamplerDescriptorPool;
};

#endif
