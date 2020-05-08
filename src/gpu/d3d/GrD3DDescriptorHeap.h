/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DDescriptorHeap_DEFINED
#define GrD3DDescriptorHeap_DEFINED

#include "include/gpu/d3d/GrD3DTypes.h"
#include "src/gpu/GrManagedResource.h"
#include "src/utils/SkBitSet.h"

class GrD3DGpu;

class GrD3DDescriptorHeap {
public:
    static std::unique_ptr<GrD3DDescriptorHeap> Make(GrD3DGpu* gpu, D3D12_DESCRIPTOR_HEAP_TYPE,
                                                     unsigned int numDescriptors,
                                                     D3D12_DESCRIPTOR_HEAP_FLAGS);

    ~GrD3DDescriptorHeap() = default;

    D3D12_CPU_DESCRIPTOR_HANDLE getCPUHandle(unsigned int index); // only if non-shader-visible
    D3D12_GPU_DESCRIPTOR_HANDLE getGPUHandle(unsigned int index);

    bool getIndex(D3D12_CPU_DESCRIPTOR_HANDLE handle, size_t* indexPtr) {
        if (handle.ptr < fCPUHeapStart.ptr) {
            return false;
        }
        size_t index = (handle.ptr - fCPUHeapStart.ptr) / fHandleIncrementSize;
        if (index >= fHeap->GetDesc().NumDescriptors) {
            return false;
        }
        SkASSERT(handle.ptr == fCPUHeapStart.ptr + index * fHandleIncrementSize);
        *indexPtr = index;
        return true;
    }

    bool getIndex(D3D12_GPU_DESCRIPTOR_HANDLE handle, size_t* indexPtr) {
        if (handle.ptr < fGPUHeapStart.ptr) {
            return false;
        }
        size_t index = (handle.ptr - fGPUHeapStart.ptr) / fHandleIncrementSize;
        if (index >= fHeap->GetDesc().NumDescriptors) {
            return false;
        }
        SkASSERT(handle.ptr == fGPUHeapStart.ptr + index * fHandleIncrementSize);
        *indexPtr = index;
        return true;
    }

protected:
    GrD3DDescriptorHeap(const gr_cp<ID3D12DescriptorHeap>&, unsigned int handleIncrementSize);

    gr_cp<ID3D12DescriptorHeap> fHeap;
    size_t fHandleIncrementSize;
    D3D12_CPU_DESCRIPTOR_HANDLE fCPUHeapStart;
    D3D12_GPU_DESCRIPTOR_HANDLE fGPUHeapStart;
};

#endif
