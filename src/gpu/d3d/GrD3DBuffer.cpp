/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DBuffer.h"
#include "src/gpu/d3d/GrD3DGpu.h"
#include "src/gpu/d3d/GrD3DUtil.h"

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
            // Includes VERTEX_AND_CONSTANT_BUFFER, INDEX_BUFFER, and COPY_SOURCE
            *resourceState = D3D12_RESOURCE_STATE_GENERIC_READ;
        }
    }

    ID3D12Resource* resource;
    D3D12_HEAP_PROPERTIES heapProperties = {
        heapType,
        D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
        D3D12_MEMORY_POOL_UNKNOWN,
        1, // CreationNodeMask
        1, // VisibleNodeMask
    };
    D3D12_RESOURCE_DESC bufferDesc = {
        D3D12_RESOURCE_DIMENSION_BUFFER,
        0, // Alignment
        size,
        1, // Height
        1, // DepthOrArraySize
        1, // MipLevels
        DXGI_FORMAT_UNKNOWN,
        {1, 0}, // SampleDesc
        D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
        D3D12_RESOURCE_FLAG_NONE,
    };
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
}

void GrD3DBuffer::onRelease() {
    if (!this->wasDestroyed()) {
        fResource.reset();
    }
    INHERITED::onRelease();
}

void GrD3DBuffer::onAbandon() {
    if (!this->wasDestroyed()) {
        fResource.reset();
    }
    INHERITED::onAbandon();
}
