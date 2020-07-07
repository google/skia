/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DDescriptorHeap.h"
#include "src/gpu/d3d/GrD3DGpu.h"

std::unique_ptr<GrD3DDescriptorHeap> GrD3DDescriptorHeap::Make(GrD3DGpu* gpu,
                                                               D3D12_DESCRIPTOR_HEAP_TYPE type,
                                                               unsigned int numDescriptors,
                                                               D3D12_DESCRIPTOR_HEAP_FLAGS flags) {
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.Type = type;
    heapDesc.NumDescriptors = numDescriptors;
    heapDesc.Flags = flags;

    ID3D12DescriptorHeap* heap;
    gpu->device()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heap));

    return std::unique_ptr<GrD3DDescriptorHeap>(
            new GrD3DDescriptorHeap(std::move(gr_cp<ID3D12DescriptorHeap>(heap)),
                                    gpu->device()->GetDescriptorHandleIncrementSize(type)));
}

GrD3DDescriptorHeap::GrD3DDescriptorHeap(const gr_cp<ID3D12DescriptorHeap>& heap,
                                         unsigned int handleIncrementSize)
    : fHeap(heap)
    , fHandleIncrementSize(handleIncrementSize)
    , fUniqueID(GenID()) {
    fCPUHeapStart = fHeap->GetCPUDescriptorHandleForHeapStart();
    fGPUHeapStart = fHeap->GetGPUDescriptorHandleForHeapStart();
}

GrD3DDescriptorHeap::CPUHandle GrD3DDescriptorHeap::getCPUHandle(unsigned int index) {
    SkASSERT(index < fHeap->GetDesc().NumDescriptors);
    D3D12_CPU_DESCRIPTOR_HANDLE handle = fCPUHeapStart;
    handle.ptr += index * fHandleIncrementSize;
    return {handle, fUniqueID};
}

GrD3DDescriptorHeap::GPUHandle GrD3DDescriptorHeap::getGPUHandle(unsigned int index) {
    SkASSERT(index < fHeap->GetDesc().NumDescriptors);
    D3D12_GPU_DESCRIPTOR_HANDLE handle = fGPUHeapStart;
    handle.ptr += index * fHandleIncrementSize;
    return {handle, fUniqueID};
}



