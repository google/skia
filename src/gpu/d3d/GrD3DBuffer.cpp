/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DBuffer.h"
#include "src/gpu/d3d/GrD3DGpu.h"
#include "src/gpu/d3d/GrD3DUtil.h"

#include "include/third_party/d3d/d3dx12.h"

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
    HRESULT hr = gpu->device()->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(heapType),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(size),
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
    , fResource(bufferResource)
    , fResourceState(resourceState) {
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
