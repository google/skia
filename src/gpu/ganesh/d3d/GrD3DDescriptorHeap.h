/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DDescriptorHeap_DEFINED
#define GrD3DDescriptorHeap_DEFINED

#include "include/gpu/d3d/GrD3DTypes.h"
#include "src/gpu/ganesh/GrManagedResource.h"
#include "src/utils/SkBitSet.h"

class GrD3DGpu;

class GrD3DDescriptorHeap {
public:
    static std::unique_ptr<GrD3DDescriptorHeap> Make(GrD3DGpu* gpu, D3D12_DESCRIPTOR_HEAP_TYPE,
                                                     unsigned int numDescriptors,
                                                     D3D12_DESCRIPTOR_HEAP_FLAGS);

    ~GrD3DDescriptorHeap() = default;

    uint32_t uniqueID() const { return fUniqueID; }

    struct CPUHandle {
        D3D12_CPU_DESCRIPTOR_HANDLE fHandle;
        uint32_t fHeapID;
    };

    struct GPUHandle {
        D3D12_GPU_DESCRIPTOR_HANDLE fHandle;
        uint32_t fHeapID;
    };

    CPUHandle getCPUHandle(unsigned int index); // write-only if shader-visible
    GPUHandle getGPUHandle(unsigned int index);
    ID3D12DescriptorHeap* descriptorHeap() const { return fHeap.get(); }
    size_t handleIncrementSize() { return fHandleIncrementSize; }

    size_t getIndex(const CPUHandle& handle) {
        SkASSERT(handle.fHeapID == fUniqueID);
        size_t index = (handle.fHandle.ptr - fCPUHeapStart.ptr) / fHandleIncrementSize;
        SkASSERT(index < fHeap->GetDesc().NumDescriptors);
        SkASSERT(handle.fHandle.ptr == fCPUHeapStart.ptr + index * fHandleIncrementSize);
        return index;
    }

    size_t getIndex(const GPUHandle& handle) {
        SkASSERT(handle.fHeapID == fUniqueID);
        size_t index = (handle.fHandle.ptr - fCPUHeapStart.ptr) / fHandleIncrementSize;
        SkASSERT(index < fHeap->GetDesc().NumDescriptors);
        SkASSERT(handle.fHandle.ptr == fCPUHeapStart.ptr + index * fHandleIncrementSize);
        return index;
    }

protected:
    GrD3DDescriptorHeap(const gr_cp<ID3D12DescriptorHeap>&, unsigned int handleIncrementSize);

    static uint32_t GenID() {
        static std::atomic<uint32_t> nextID{1};
        uint32_t id;
        do {
            id = nextID++;
        } while (id == SK_InvalidUniqueID);
        return id;
    }

    gr_cp<ID3D12DescriptorHeap> fHeap;
    size_t fHandleIncrementSize;
    D3D12_CPU_DESCRIPTOR_HANDLE fCPUHeapStart;
    D3D12_GPU_DESCRIPTOR_HANDLE fGPUHeapStart;
    uint32_t fUniqueID;
};

#endif
