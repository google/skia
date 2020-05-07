/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DAttachmentViewManager.h"

#include "src/gpu/d3d/GrD3DGpu.h"

GrD3DAttachmentViewManager::GrD3DAttachmentViewManager(GrD3DGpu* gpu)
    : fRTVDescriptorPool(gpu, D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
    , fDSVDescriptorPool(gpu, D3D12_DESCRIPTOR_HEAP_TYPE_DSV) {}

D3D12_CPU_DESCRIPTOR_HANDLE GrD3DAttachmentViewManager::createRenderTargetView(
        GrD3DGpu* gpu, ID3D12Resource* textureResource) {
    D3D12_CPU_DESCRIPTOR_HANDLE descriptor = fRTVDescriptorPool.allocateHandle(gpu);
    gpu->device()->CreateRenderTargetView(textureResource, nullptr, descriptor);
    return descriptor;
}

void GrD3DAttachmentViewManager::recycleRenderTargetView(
        D3D12_CPU_DESCRIPTOR_HANDLE* rtvDescriptor) {
    fRTVDescriptorPool.releaseHandle(rtvDescriptor);
}

D3D12_CPU_DESCRIPTOR_HANDLE GrD3DAttachmentViewManager::createDepthStencilView(
        GrD3DGpu* gpu, ID3D12Resource* textureResource) {
    D3D12_CPU_DESCRIPTOR_HANDLE descriptor = fDSVDescriptorPool.allocateHandle(gpu);
    gpu->device()->CreateDepthStencilView(textureResource, nullptr, descriptor);
    return descriptor;
}

void GrD3DAttachmentViewManager::recycleDepthStencilView(
        D3D12_CPU_DESCRIPTOR_HANDLE* dsvDescriptor) {
    fDSVDescriptorPool.releaseHandle(dsvDescriptor);
}

////////////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<GrD3DAttachmentViewManager::Heap> GrD3DAttachmentViewManager::Heap::Make(
        GrD3DGpu* gpu, D3D12_DESCRIPTOR_HEAP_TYPE type, unsigned int numDescriptors) {
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.Type = type;
    heapDesc.NumDescriptors = numDescriptors;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    ID3D12DescriptorHeap* heap;
    gpu->device()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heap));

    return std::unique_ptr<GrD3DAttachmentViewManager::Heap>(
        new GrD3DAttachmentViewManager::Heap(
                std::move(gr_cp<ID3D12DescriptorHeap>(heap)),
                          gpu->device()->GetDescriptorHandleIncrementSize(type),
                          heapDesc.NumDescriptors));
}

D3D12_CPU_DESCRIPTOR_HANDLE GrD3DAttachmentViewManager::Heap::allocateCPUHandle() {
    SkBitSet::OptionalIndex freeBlock = fFreeBlocks.findFirst();
    SkASSERT(freeBlock);
    fFreeBlocks.reset(*freeBlock);
    --fFreeCount;
    return this->getCPUHandle(*freeBlock);
}

void GrD3DAttachmentViewManager::Heap::freeCPUHandle(D3D12_CPU_DESCRIPTOR_HANDLE* handle) {
    // Make sure this handle belongs to this heap
    SkASSERT(handle->ptr >= fCPUHeapStart.ptr);
    SIZE_T index = (handle->ptr - fCPUHeapStart.ptr) / fHandleIncrementSize;
    SkASSERT(index < fHeap->GetDesc().NumDescriptors);
    fFreeBlocks.set(index);
    ++fFreeCount;
    handle->ptr = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////

GrD3DAttachmentViewManager::HeapPool::HeapPool(GrD3DGpu* gpu, D3D12_DESCRIPTOR_HEAP_TYPE heapType)
    : fMaxAvailableDescriptors(32)
    , fHeapType(heapType) {
    std::unique_ptr<GrD3DAttachmentViewManager::Heap> heap =
            GrD3DAttachmentViewManager::Heap::Make(gpu, fHeapType, fMaxAvailableDescriptors);
    fDescriptorHeaps.push_back(std::move(heap));
}

D3D12_CPU_DESCRIPTOR_HANDLE GrD3DAttachmentViewManager::HeapPool::allocateHandle(GrD3DGpu* gpu) {
    for (unsigned int i = 0; i < fDescriptorHeaps.size(); ++i) {
        if (fDescriptorHeaps[i]->canAllocate()) {
            D3D12_CPU_DESCRIPTOR_HANDLE handle = fDescriptorHeaps[i]->allocateCPUHandle();
            return handle;
        }
    }

    // need to allocate more space
    std::unique_ptr<GrD3DAttachmentViewManager::Heap> heap =
            GrD3DAttachmentViewManager::Heap::Make(gpu, fHeapType,
                                                   fMaxAvailableDescriptors);
    fDescriptorHeaps.push_back(std::move(heap));
    fMaxAvailableDescriptors *= 2;
    D3D12_CPU_DESCRIPTOR_HANDLE handle =
            fDescriptorHeaps[fDescriptorHeaps.size() - 1]->allocateCPUHandle();
    return handle;
}

void GrD3DAttachmentViewManager::HeapPool::releaseHandle(
        D3D12_CPU_DESCRIPTOR_HANDLE* dsvDescriptor) {
    for (unsigned int i = 0; i < fDescriptorHeaps.size(); ++i) {
        if (fDescriptorHeaps[i]->ownsHandle(*dsvDescriptor)) {
            fDescriptorHeaps[i]->freeCPUHandle(dsvDescriptor);
            return;
        }
    }
    SkASSERT(false);
}
