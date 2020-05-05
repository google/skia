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

class GrD3DDescriptorHeap : public GrManagedResource {
public:
    static sk_sp<GrD3DDescriptorHeap> Make(GrD3DGpu* gpu, D3D12_DESCRIPTOR_HEAP_TYPE,
                                           unsigned int numDescriptors,
                                           D3D12_DESCRIPTOR_HEAP_FLAGS);

    ~GrD3DDescriptorHeap() override = default;

    D3D12_CPU_DESCRIPTOR_HANDLE allocateCPUHandle(); // valid only for non-shader-visible heaps
    D3D12_GPU_DESCRIPTOR_HANDLE allocateGPUHandle();
    void freeCPUHandle(D3D12_CPU_DESCRIPTOR_HANDLE*);
    void freeGPUHandle(D3D12_GPU_DESCRIPTOR_HANDLE*);

    bool canAllocate() { return fFreeCount > 0; }
    bool ownsHandle(D3D12_CPU_DESCRIPTOR_HANDLE handle) {
        if (handle.ptr < fCPUHeapStart.ptr) {
            return false;
        }
        SIZE_T index = (handle.ptr - fCPUHeapStart.ptr) / fHandleIncrementSize;
        return (index < fHeap->GetDesc().NumDescriptors);
    }
    bool ownsHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle) {
        if (handle.ptr < fGPUHeapStart.ptr) {
            return false;
        }
        SIZE_T index = (handle.ptr - fGPUHeapStart.ptr) / fHandleIncrementSize;
        return (index < fHeap->GetDesc().NumDescriptors);
    }

#ifdef SK_TRACE_MANAGED_RESOURCES
    /** Output a human-readable dump of this resource's information
     */
    void dumpInfo() const override;
#endif

    void freeGPUData() const override {}

private:
    GrD3DDescriptorHeap(const gr_cp<ID3D12DescriptorHeap>&, const D3D12_DESCRIPTOR_HEAP_DESC&,
                        unsigned int handleIncrementSize);

    gr_cp<ID3D12DescriptorHeap> fHeap;
    size_t fHandleIncrementSize;
    SkBitSet fFreeBlocks;
    int fFreeCount;
    D3D12_CPU_DESCRIPTOR_HANDLE fCPUHeapStart;
    D3D12_GPU_DESCRIPTOR_HANDLE fGPUHeapStart;
};

#endif
