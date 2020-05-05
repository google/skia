/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DAttachmentViewManager.h"

#include "src/gpu/d3d/GrD3DGpu.h"

GrD3DAttachmentViewManager::GrD3DAttachmentViewManager(GrD3DGpu* gpu) {
    fMaxAvailableRTVDescriptors = 32;
    sk_sp<GrD3DDescriptorHeap> heap = GrD3DDescriptorHeap::Make(gpu, D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
                                                                fMaxAvailableRTVDescriptors,
                                                                D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
    fRTVDescriptorHeaps.push_back(std::move(heap));

    fMaxAvailableDSVDescriptors = 32;
    heap = GrD3DDescriptorHeap::Make(gpu, D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
                                     fMaxAvailableDSVDescriptors,
                                     D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
    fDSVDescriptorHeaps.push_back(std::move(heap));
}

D3D12_CPU_DESCRIPTOR_HANDLE GrD3DAttachmentViewManager::createRenderTargetView(
        GrD3DGpu* gpu, ID3D12Resource* textureResource) {
    for (unsigned int i = 0; i < fRTVDescriptorHeaps.size(); ++i) {
        if (fRTVDescriptorHeaps[i]->canAllocate()) {
            D3D12_CPU_DESCRIPTOR_HANDLE descriptor = fRTVDescriptorHeaps[i]->allocateCPUHandle();
            gpu->device()->CreateRenderTargetView(textureResource, nullptr, descriptor);
            return descriptor;
        }
    }

    // need to allocate more space
    sk_sp<GrD3DDescriptorHeap> heap = GrD3DDescriptorHeap::Make(gpu,
                                                                D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
                                                                fMaxAvailableRTVDescriptors,
                                                                D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
    fRTVDescriptorHeaps.push_back(std::move(heap));
    fMaxAvailableRTVDescriptors *= 2;
    D3D12_CPU_DESCRIPTOR_HANDLE descriptor =
            fRTVDescriptorHeaps[fRTVDescriptorHeaps.size()-1]->allocateCPUHandle();
    gpu->device()->CreateRenderTargetView(textureResource, nullptr, descriptor);
    return descriptor;
}

void GrD3DAttachmentViewManager::recycleRenderTargetView(
        D3D12_CPU_DESCRIPTOR_HANDLE* rtvDescriptor) {
    for (unsigned int i = 0; i < fRTVDescriptorHeaps.size(); ++i) {
        if (fRTVDescriptorHeaps[i]->ownsHandle(*rtvDescriptor)) {
            fRTVDescriptorHeaps[i]->freeCPUHandle(rtvDescriptor);
            return;
        }
    }
    SkASSERT(false);
}

D3D12_CPU_DESCRIPTOR_HANDLE GrD3DAttachmentViewManager::createDepthStencilView(
        GrD3DGpu* gpu, ID3D12Resource* textureResource) {
    for (unsigned int i = 0; i < fDSVDescriptorHeaps.size(); ++i) {
        if (fDSVDescriptorHeaps[i]->canAllocate()) {
            D3D12_CPU_DESCRIPTOR_HANDLE descriptor = fDSVDescriptorHeaps[i]->allocateCPUHandle();
            gpu->device()->CreateRenderTargetView(textureResource, nullptr, descriptor);
            return descriptor;
        }
    }

    // need to allocate more space
    sk_sp<GrD3DDescriptorHeap> heap = GrD3DDescriptorHeap::Make(gpu,
                                                                D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
                                                                fMaxAvailableDSVDescriptors,
                                                                D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
    fDSVDescriptorHeaps.push_back(std::move(heap));
    fMaxAvailableDSVDescriptors *= 2;
    D3D12_CPU_DESCRIPTOR_HANDLE descriptor =
        fDSVDescriptorHeaps[fDSVDescriptorHeaps.size() - 1]->allocateCPUHandle();
    gpu->device()->CreateDepthStencilView(textureResource, nullptr, descriptor);
    return descriptor;
}

void GrD3DAttachmentViewManager::recycleDepthStencilView(
        D3D12_CPU_DESCRIPTOR_HANDLE* dsvDescriptor) {
    for (unsigned int i = 0; i < fDSVDescriptorHeaps.size(); ++i) {
        if (fDSVDescriptorHeaps[i]->ownsHandle(*dsvDescriptor)) {
            fDSVDescriptorHeaps[i]->freeCPUHandle(dsvDescriptor);
            return;
        }
    }
    SkASSERT(false);
}

