/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DBuffer.h"
#include "src/gpu/d3d/GrD3DGpu.h"
#include "src/gpu/d3d/GrD3DUtil.h"

#ifdef SK_DEBUG
#define VALIDATE() this->validate()
#else
#define VALIDATE() do {} while(false)
#endif

sk_sp<GrD3DBuffer::Resource> GrD3DBuffer::Resource::Make(GrD3DGpu* gpu, size_t size,
                                                         GrGpuBufferType intendedType,
                                                         GrAccessPattern accessPattern,
                                                         D3D12_RESOURCE_STATES* resourceState) {
    D3D12_HEAP_TYPE heapType;
    if (accessPattern == kStatic_GrAccessPattern) {
        SkASSERT(intendedType != GrGpuBufferType::kXferCpuToGpu &&
                 intendedType != GrGpuBufferType::kXferGpuToCpu);
        heapType = D3D12_HEAP_TYPE_DEFAULT;
        // Can be transitioned to different states
        *resourceState = D3D12_RESOURCE_STATE_COMMON;
    } else {
        if (intendedType == GrGpuBufferType::kXferGpuToCpu) {
            heapType = D3D12_HEAP_TYPE_READBACK;
            // Cannot be changed
            *resourceState = D3D12_RESOURCE_STATE_COPY_DEST;
        } else {
            heapType = D3D12_HEAP_TYPE_UPLOAD;
            // Cannot be changed
            // Includes VERTEX_AND_CONSTANT_BUFFER, INDEX_BUFFER, INDIRECT_ARGUMENT, and COPY_SOURCE
            *resourceState = D3D12_RESOURCE_STATE_GENERIC_READ;
        }
    }

    D3D12_HEAP_PROPERTIES heapProperties = {};
    heapProperties.Type = heapType;
    heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProperties.CreationNodeMask = 1;
    heapProperties.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC bufferDesc = {};
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Alignment = 0;  // default alignment
    bufferDesc.Width = size;
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels = 1;
    bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.SampleDesc.Quality = 0; // Doesn't apply to buffers
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    ID3D12Resource* resource;
    HRESULT hr = gpu->device()->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            *resourceState,
            nullptr,
            IID_PPV_ARGS(&resource));
    if (!SUCCEEDED(hr)) {
        return nullptr;
    }

    return sk_sp<Resource>(new GrD3DBuffer::Resource(std::move(gr_cp<ID3D12Resource>(resource))));
}

sk_sp<GrD3DBuffer> GrD3DBuffer::Make(GrD3DGpu* gpu, size_t size, GrGpuBufferType intendedType,
                                     GrAccessPattern accessPattern) {
    SkASSERT(!gpu->protectedContext() || (accessPattern != kStatic_GrAccessPattern));
    D3D12_RESOURCE_STATES resourceState;
    sk_sp<Resource> resource = Resource::Make(gpu, size, intendedType, accessPattern,
                                              &resourceState);
    if (!resource) {
        return nullptr;
    }

    return sk_sp<GrD3DBuffer>(new GrD3DBuffer(gpu, size, intendedType, accessPattern,
                                              std::move(resource), resourceState));
}

GrD3DBuffer::GrD3DBuffer(GrD3DGpu* gpu, size_t size, GrGpuBufferType intendedType,
                         GrAccessPattern accessPattern, const sk_sp<Resource>& bufferResource,
                         D3D12_RESOURCE_STATES resourceState)
    : INHERITED(gpu, size, intendedType, accessPattern)
    , fResourceState(resourceState)
    , fResource(bufferResource) {
    this->registerWithCache(SkBudgeted::kYes);

    // TODO: persistently map UPLOAD resources?

    VALIDATE();
}

void GrD3DBuffer::onRelease() {
    if (!this->wasDestroyed()) {
        VALIDATE();
        fResource.reset();
        fMappedResource.reset();
        fMapPtr = nullptr;
        VALIDATE();
    }
    INHERITED::onRelease();
}

void GrD3DBuffer::onAbandon() {
    if (!this->wasDestroyed()) {
        VALIDATE();
        fResource.reset();
        fMappedResource.reset();
        fMapPtr = nullptr;
        VALIDATE();
    }
    INHERITED::onAbandon();
}

void GrD3DBuffer::onMap() {
    this->internalMap(this->size());
}

void GrD3DBuffer::onUnmap() {
    this->internalUnmap(this->size());
}

bool GrD3DBuffer::onUpdateData(const void* src, size_t size) {
    SkASSERT(src);
    if (size > this->size()) {
        return false;
    }
    if (!fResource) {
        return false;
    }

    this->internalMap(size);
    if (!fMapPtr) {
        return false;
    }
    SkASSERT(fMappedResource);
    if (this->accessPattern() == kStatic_GrAccessPattern) {
        // We should never call this method on static buffers in protected contexts.
        SkASSERT(!this->getD3DGpu()->protectedContext());
        //*** any alignment restrictions?
    }
    memcpy(fMapPtr, src, size);
    this->internalUnmap(size);

    return true;
}

void GrD3DBuffer::internalMap(size_t size) {
    // TODO: if UPLOAD heap type, could be persistently mapped (i.e., this would be a no-op)
    if (this->wasDestroyed()) {
        return;
    }
    SkASSERT(fResource);
    SkASSERT(!fMappedResource);
    SkASSERT(!this->isMapped());
    SkASSERT(fResource->size() >= size);

    VALIDATE();

    if (this->accessPattern() == kStatic_GrAccessPattern) {
#ifdef SK_BUILD_FOR_MAC
        // Mac requires 4-byte alignment for copies so we pad this out
        sizeInBytes = SkAlign4(sizeInBytes);
#endif
        // TODO: should use a slice of a previously allocated UPLOAD buffer
        D3D12_RESOURCE_STATES resourceState; // not used, just to pass to make
        fMappedResource = Resource::Make(this->getD3DGpu(), size, GrGpuBufferType::kXferCpuToGpu,
                                         GrAccessPattern::kDynamic_GrAccessPattern,
                                         &resourceState);
        SkASSERT(resourceState == D3D12_RESOURCE_STATE_GENERIC_READ);
        D3D12_RANGE range;
        range.Begin = 0;
        range.End = size;
        fMappedResource->fD3DResource->Map(0, &range, &fMapPtr);
    } else {
        fMappedResource = fResource;
        D3D12_RANGE range;
        range.Begin = 0;
        range.End = size;
        fMappedResource->fD3DResource->Map(0, &range, &fMapPtr);
    }

    VALIDATE();
}

void GrD3DBuffer::internalUnmap(size_t size) {
    // TODO: if UPLOAD heap type, could be persistently mapped (i.e., this would be a no-op)
    if (this->wasDestroyed()) {
        return;
    }
    SkASSERT(fResource);
    SkASSERT(fMappedResource);
    SkASSERT(this->isMapped());
    SkASSERT(fMappedResource->size() >= size);
    VALIDATE();

#ifdef SK_BUILD_FOR_MAC
    // In both cases the size needs to be 4-byte aligned on Mac
    sizeInBytes = SkAlign4(sizeInBytes);
#endif
    if (this->accessPattern() == kStatic_GrAccessPattern) {
        // TODO: if using a slice of a persistently mapped UPLOAD buffer don't unmap here
        D3D12_RANGE range;
        range.Begin = 0;
        range.End = size;
        fMappedResource->fD3DResource->Unmap(0, &range);
        // TODO
        // let the D3DGpu make this decision?
        //if (size == this->size()) {
        //    this->getD3DGpu()->copyResource(this->d3dResource(), fMappedResource->fD3DResource);

        //} else {
        //    this->getD3DGpu()->copyBufferRegion(this->d3dResource(), 0,
        //                                        fMappedResource->fD3DResource, 0, size);
        //}
    } else {
        D3D12_RANGE range;
        range.Begin = 0;
        // For READBACK heaps, unmap requires an empty range
        range.End = fResourceState == D3D12_RESOURCE_STATE_COPY_DEST ? 0 : size;
        fMappedResource->fD3DResource->Unmap(0, &range);
    }

    fMappedResource.reset(nullptr);
    fMapPtr = nullptr;

    VALIDATE();
}

#ifdef SK_DEBUG
void GrD3DBuffer::validate() const {
    SkASSERT(!fResource ||
             this->intendedType() == GrGpuBufferType::kVertex ||
             this->intendedType() == GrGpuBufferType::kIndex ||
             //this->intendedType() == GrGpuBufferType::kDrawIndirect ||
             this->intendedType() == GrGpuBufferType::kXferCpuToGpu ||
             this->intendedType() == GrGpuBufferType::kXferGpuToCpu);
    SkASSERT(!fMappedResource || !fResource ||
             (fResource->size() == this->size() &&
              fMappedResource->size() <= fResource->size()));
}
#endif
