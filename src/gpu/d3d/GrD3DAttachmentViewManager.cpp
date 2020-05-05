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

GrD3DAttachmentViewManager::DescriptorPool::DescriptorPool(GrD3DGpu* gpu,
                                                           D3D12_DESCRIPTOR_HEAP_TYPE heapType)
    : fMaxAvailableDescriptors(32)
    , fHeapType(heapType) {
    sk_sp<GrD3DDescriptorHeap> heap = GrD3DDescriptorHeap::Make(gpu, fHeapType,
                                                                fMaxAvailableDescriptors,
                                                                D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
    fDescriptorHeaps.push_back(std::move(heap));
}

D3D12_CPU_DESCRIPTOR_HANDLE GrD3DAttachmentViewManager::DescriptorPool::allocateHandle(
        GrD3DGpu* gpu) {
    for (unsigned int i = 0; i < fDescriptorHeaps.size(); ++i) {
        if (fDescriptorHeaps[i]->canAllocate()) {
            D3D12_CPU_DESCRIPTOR_HANDLE handle = fDescriptorHeaps[i]->allocateCPUHandle();
            return handle;
        }
    }

    // need to allocate more space
    sk_sp<GrD3DDescriptorHeap> heap = GrD3DDescriptorHeap::Make(gpu,
                                                                fHeapType,
                                                                fMaxAvailableDescriptors,
                                                                D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
    fDescriptorHeaps.push_back(std::move(heap));
    fMaxAvailableDescriptors *= 2;
    D3D12_CPU_DESCRIPTOR_HANDLE handle =
            fDescriptorHeaps[fDescriptorHeaps.size() - 1]->allocateCPUHandle();
    return handle;
}

void GrD3DAttachmentViewManager::DescriptorPool::releaseHandle(
        D3D12_CPU_DESCRIPTOR_HANDLE* dsvDescriptor) {
    for (unsigned int i = 0; i < fDescriptorHeaps.size(); ++i) {
        if (fDescriptorHeaps[i]->ownsHandle(*dsvDescriptor)) {
            fDescriptorHeaps[i]->freeCPUHandle(dsvDescriptor);
            return;
        }
    }
    SkASSERT(false);
}
