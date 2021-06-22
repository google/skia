/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DCpuDescriptorManager.h"

#include "src/gpu/d3d/GrD3DGpu.h"

GrD3DCpuDescriptorManager::GrD3DCpuDescriptorManager(GrD3DGpu* gpu)
    : fRTVDescriptorPool(gpu, D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
    , fDSVDescriptorPool(gpu, D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
    , fShaderViewDescriptorPool(gpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
    , fSamplerDescriptorPool(gpu, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER) {}

GrD3DDescriptorHeap::CPUHandle GrD3DCpuDescriptorManager::createRenderTargetView(
        GrD3DGpu* gpu, ID3D12Resource* textureResource) {
    const GrD3DDescriptorHeap::CPUHandle& descriptor = fRTVDescriptorPool.allocateHandle(gpu);
    gpu->device()->CreateRenderTargetView(textureResource, nullptr, descriptor.fHandle);
    return descriptor;
}

void GrD3DCpuDescriptorManager::recycleRenderTargetView(
        const GrD3DDescriptorHeap::CPUHandle& rtvDescriptor) {
    fRTVDescriptorPool.releaseHandle(rtvDescriptor);
}

GrD3DDescriptorHeap::CPUHandle GrD3DCpuDescriptorManager::createDepthStencilView(
        GrD3DGpu* gpu, ID3D12Resource* textureResource) {
    const GrD3DDescriptorHeap::CPUHandle& descriptor = fDSVDescriptorPool.allocateHandle(gpu);
    gpu->device()->CreateDepthStencilView(textureResource, nullptr, descriptor.fHandle);
    return descriptor;
}

void GrD3DCpuDescriptorManager::recycleDepthStencilView(
        const GrD3DDescriptorHeap::CPUHandle& dsvDescriptor) {
    fDSVDescriptorPool.releaseHandle(dsvDescriptor);
}

GrD3DDescriptorHeap::CPUHandle GrD3DCpuDescriptorManager::createConstantBufferView(
        GrD3DGpu* gpu, ID3D12Resource* bufferResource, size_t offset, size_t size) {
    const GrD3DDescriptorHeap::CPUHandle& descriptor =
            fShaderViewDescriptorPool.allocateHandle(gpu);
    D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
    desc.BufferLocation = bufferResource->GetGPUVirtualAddress() + offset;
    desc.SizeInBytes = size;
    gpu->device()->CreateConstantBufferView(&desc, descriptor.fHandle);
    return descriptor;
}

GrD3DDescriptorHeap::CPUHandle GrD3DCpuDescriptorManager::createShaderResourceView(
        GrD3DGpu* gpu, ID3D12Resource* resource,
        unsigned int mostDetailedMip, unsigned int mipLevels) {
    const GrD3DDescriptorHeap::CPUHandle& descriptor =
            fShaderViewDescriptorPool.allocateHandle(gpu);
    // TODO: for 4:2:0 YUV formats we'll need to map two different views, one for Y and one for UV.
    // For now map the entire resource.
    D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
    desc.Format = resource->GetDesc().Format;
    desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    desc.Texture2D.MostDetailedMip = mostDetailedMip;
    desc.Texture2D.MipLevels = mipLevels;
    desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    gpu->device()->CreateShaderResourceView(resource, &desc, descriptor.fHandle);
    return descriptor;
}

GrD3DDescriptorHeap::CPUHandle GrD3DCpuDescriptorManager::createUnorderedAccessView(
        GrD3DGpu* gpu, ID3D12Resource* resource, unsigned int mipSlice) {
    const GrD3DDescriptorHeap::CPUHandle& descriptor =
            fShaderViewDescriptorPool.allocateHandle(gpu);
    if (resource->GetDesc().Dimension == D3D12_RESOURCE_DIMENSION_BUFFER) {
        // TODO: figure out buffer setup
        gpu->device()->CreateUnorderedAccessView(resource, nullptr, nullptr, descriptor.fHandle);
    } else {
        D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
        desc.Format = resource->GetDesc().Format;
        desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MipSlice = mipSlice;
        gpu->device()->CreateUnorderedAccessView(resource, nullptr, &desc, descriptor.fHandle);
    }
    return descriptor;
}

void GrD3DCpuDescriptorManager::recycleShaderView(
        const GrD3DDescriptorHeap::CPUHandle& view) {
    fShaderViewDescriptorPool.releaseHandle(view);
}

GrD3DDescriptorHeap::CPUHandle GrD3DCpuDescriptorManager::createSampler(
        GrD3DGpu* gpu,
        D3D12_FILTER filter,
        float maxLOD,
        D3D12_TEXTURE_ADDRESS_MODE addressModeU,
        D3D12_TEXTURE_ADDRESS_MODE addressModeV) {
    const GrD3DDescriptorHeap::CPUHandle& descriptor = fSamplerDescriptorPool.allocateHandle(gpu);
    D3D12_SAMPLER_DESC desc = {};
    desc.Filter = filter;
    desc.AddressU = addressModeU;
    desc.AddressV = addressModeV;
    desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    desc.MipLODBias = 0;
    desc.MaxAnisotropy = 1;
    desc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    // desc.BorderColor initialized to { 0, 0, 0, 0 } by default initializer, above.
    desc.MinLOD = 0;
    desc.MaxLOD = maxLOD;

    gpu->device()->CreateSampler(&desc, descriptor.fHandle);
    return descriptor;
}

void GrD3DCpuDescriptorManager::recycleSampler(
        const GrD3DDescriptorHeap::CPUHandle& samplerDescriptor) {
    fSamplerDescriptorPool.releaseHandle(samplerDescriptor);
}

////////////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<GrD3DCpuDescriptorManager::Heap> GrD3DCpuDescriptorManager::Heap::Make(
        GrD3DGpu* gpu, D3D12_DESCRIPTOR_HEAP_TYPE type, unsigned int numDescriptors) {
    std::unique_ptr<GrD3DDescriptorHeap> heap =
            GrD3DDescriptorHeap::Make(gpu, type, numDescriptors, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
    if (!heap) {
        return nullptr;
    }

    return std::unique_ptr<Heap>(new Heap(heap, numDescriptors));
}

GrD3DDescriptorHeap::CPUHandle GrD3DCpuDescriptorManager::Heap::allocateCPUHandle() {
    SkBitSet::OptionalIndex freeBlock = fFreeBlocks.findFirst();
    SkASSERT(freeBlock.has_value());
    fFreeBlocks.reset(*freeBlock);
    --fFreeCount;
    return fHeap->getCPUHandle(*freeBlock);
}

void GrD3DCpuDescriptorManager::Heap::freeCPUHandle(const GrD3DDescriptorHeap::CPUHandle& handle) {
    SkASSERT(this->ownsHandle(handle));
    size_t index = fHeap->getIndex(handle);
    fFreeBlocks.set(index);
    ++fFreeCount;
}

////////////////////////////////////////////////////////////////////////////////////////////////

GrD3DCpuDescriptorManager::HeapPool::HeapPool(GrD3DGpu* gpu, D3D12_DESCRIPTOR_HEAP_TYPE heapType)
    : fMaxAvailableDescriptors(32)
    , fHeapType(heapType) {
    std::unique_ptr<GrD3DCpuDescriptorManager::Heap> heap =
            GrD3DCpuDescriptorManager::Heap::Make(gpu, fHeapType, fMaxAvailableDescriptors);
    fDescriptorHeaps.push_back(std::move(heap));
}

GrD3DDescriptorHeap::CPUHandle GrD3DCpuDescriptorManager::HeapPool::allocateHandle(
        GrD3DGpu* gpu) {
    for (unsigned int i = 0; i < fDescriptorHeaps.size(); ++i) {
        if (fDescriptorHeaps[i]->canAllocate()) {
            GrD3DDescriptorHeap::CPUHandle handle = fDescriptorHeaps[i]->allocateCPUHandle();
            return handle;
        }
    }

    // need to allocate more space
    std::unique_ptr<GrD3DCpuDescriptorManager::Heap> heap =
            GrD3DCpuDescriptorManager::Heap::Make(gpu, fHeapType, fMaxAvailableDescriptors);
    // TODO: handle failed heap creation and/or memory restrictions better
    // skbug.com/11959
    SkASSERT(heap);

    fDescriptorHeaps.push_back(std::move(heap));
    fMaxAvailableDescriptors *= 2;
    GrD3DDescriptorHeap::CPUHandle handle =
            fDescriptorHeaps[fDescriptorHeaps.size() - 1]->allocateCPUHandle();
    return handle;
}

void GrD3DCpuDescriptorManager::HeapPool::releaseHandle(
        const GrD3DDescriptorHeap::CPUHandle& dsvDescriptor) {
    for (unsigned int i = 0; i < fDescriptorHeaps.size(); ++i) {
        if (fDescriptorHeaps[i]->ownsHandle(dsvDescriptor)) {
            fDescriptorHeaps[i]->freeCPUHandle(dsvDescriptor);
            return;
        }
    }
    SkASSERT(false);
}
