/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrGpuResourcePriv.h"
#include "src/gpu/d3d/GrD3DGpu.h"
#include "src/gpu/d3d/GrD3DResource.h"

void GrD3DResource::setResourceState(const GrD3DGpu* gpu,
                                     D3D12_RESOURCE_STATES newResourceState) {
    SkASSERT(fStateExplicitlySet);
/* TODO
 * Something like:
    D3D12_RESOURCE_STATES currentState = this->currentState();
    gpu->addResourceTransitionBarrier(this->resource(), currentState, newResourceState);
*/
    this->updateResourceState(newResourceState, true);
}

ID3D12Resource* GrD3DResource::CreateResource(GrD3DGpu* gpu, const D3D12_RESOURCE_DESC& desc,
                                              D3D12_RESOURCE_STATES resourceState) {
    ID3D12Resource* resource = nullptr;

    // TODO: incorporate D3Dx12.h and use CD3DX12_HEAP_PROPERTIES instead?
    D3D12_HEAP_PROPERTIES heapProperties = {
        D3D12_HEAP_TYPE_DEFAULT,
        D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
        D3D12_MEMORY_POOL_UNKNOWN,
        1, // CreationNodeMask
        1  // VisibleNodeMask
    };
    SkDEBUGCODE(HRESULT hr =) gpu->device()->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        resourceState,
        nullptr,
        IID_PPV_ARGS(&resource));
    SkASSERT(SUCCEEDED(hr));

    return resource;
}

GrD3DResource::~GrD3DResource() {
    // should have been released first
    SkASSERT(!fResource);
}

void GrD3DResource::releaseResource() {
    if (fResource) {
        fResource->Release();
        fResource = nullptr;
    }
}
