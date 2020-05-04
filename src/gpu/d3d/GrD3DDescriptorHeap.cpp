/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DDescriptorHeap.h"
#include "src/gpu/d3d/GrD3DGpu.h"

sk_sp<GrD3DDescriptorHeap> GrD3DDescriptorHeap::Make(GrD3DGpu* gpu, D3D12_DESCRIPTOR_HEAP_TYPE type,
                                                     unsigned int numDescriptors,
                                                     D3D12_DESCRIPTOR_HEAP_FLAGS flags) {
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.Type = type;
    heapDesc.NumDescriptors = numDescriptors;
    heapDesc.Flags = flags;

    ID3D12DescriptorHeap* heap;
    gpu->device()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heap));

    return sk_sp<GrD3DDescriptorHeap>(
            new GrD3DDescriptorHeap(std::move(gr_cp<ID3D12DescriptorHeap>(heap)), heapDesc,
                                    gpu->device()->GetDescriptorHandleIncrementSize(type)));
}

GrD3DDescriptorHeap::GrD3DDescriptorHeap(const gr_cp<ID3D12DescriptorHeap>& heap,
                                         const D3D12_DESCRIPTOR_HEAP_DESC& descriptor,
                                         unsigned int handleIncrementSize)
    : fHeap(heap)
    , fHandleIncrementSize(handleIncrementSize)
    , fFreeBlocks(descriptor.NumDescriptors) {
    // set all the bits in freeBlocks
    for (UINT i = 0; i < descriptor.NumDescriptors; ++i) {
        fFreeBlocks.set(i);
    }
}

D3D12_CPU_DESCRIPTOR_HANDLE GrD3DDescriptorHeap::allocateCPUHandle() {
    // valid only for non-shader-visible heaps
    SkASSERT(!SkToBool(fHeap->GetDesc().Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE));
    D3D12_CPU_DESCRIPTOR_HANDLE handle = fHeap->GetCPUDescriptorHandleForHeapStart();
    SkBitSet::OptionalIndex freeBlock = fFreeBlocks.findFirst();
    SkASSERT(freeBlock);
    handle.ptr += *freeBlock * fHandleIncrementSize;
    fFreeBlocks.reset(*freeBlock);

    return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE GrD3DDescriptorHeap::allocateGPUHandle() {
    D3D12_GPU_DESCRIPTOR_HANDLE handle = fHeap->GetGPUDescriptorHandleForHeapStart();
    SkBitSet::OptionalIndex freeBlock = fFreeBlocks.findFirst();
    SkASSERT(freeBlock);
    handle.ptr += *freeBlock * fHandleIncrementSize;
    fFreeBlocks.reset(*freeBlock);

    return handle;
}

void GrD3DDescriptorHeap::freeCPUHandle(D3D12_CPU_DESCRIPTOR_HANDLE* handle) {
    // valid only for non-shader-visible heaps
    SkASSERT(!SkToBool(fHeap->GetDesc().Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE));
    D3D12_CPU_DESCRIPTOR_HANDLE heapStart = fHeap->GetCPUDescriptorHandleForHeapStart();
    // Make sure this handle belongs to this heap
    SkASSERT(handle->ptr >= heapStart.ptr);
    SIZE_T index = (handle->ptr - heapStart.ptr) / fHandleIncrementSize;
    SkASSERT(index < fHeap->GetDesc().NumDescriptors);
    fFreeBlocks.set(index);
    handle->ptr = 0;
}

void GrD3DDescriptorHeap::freeGPUHandle(D3D12_GPU_DESCRIPTOR_HANDLE* handle) {
    D3D12_GPU_DESCRIPTOR_HANDLE heapStart = fHeap->GetGPUDescriptorHandleForHeapStart();
    // Make sure this handle belongs to this heap
    SkASSERT(handle->ptr >= heapStart.ptr);
    SIZE_T index = (handle->ptr - heapStart.ptr) / fHandleIncrementSize;
    SkASSERT(index < fHeap->GetDesc().NumDescriptors);
    fFreeBlocks.set(index);
    handle->ptr = 0;
}

#ifdef SK_TRACE_MANAGED_RESOURCES
    /** Output a human-readable dump of this resource's information
     */
void GrD3DDescriptorHeap::dumpInfo() const {
    SkDebugf("GrD3DDescriptorHeap: %d (%d refs)\n", fHeap.get(), this->getRefCnt());
}
#endif


