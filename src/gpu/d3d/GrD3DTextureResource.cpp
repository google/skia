/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrGpuResourcePriv.h"
#include "src/gpu/d3d/GrD3DGpu.h"
#include "src/gpu/d3d/GrD3DTextureResource.h"

void GrD3DTextureResource::setResourceState(const GrD3DGpu* gpu,
                                            D3D12_RESOURCE_STATES newResourceState) {
    SkASSERT(fStateExplicitlySet);
/* TODO
 * Something like:
    D3D12_RESOURCE_STATES currentState = this->currentState();
    gpu->addResourceTransitionBarrier(this->resource(), currentState, newResourceState);
*/
    this->updateResourceState(newResourceState, true);
}

bool GrD3DTextureResource::InitTextureResourceInfo(GrD3DGpu* gpu, const D3D12_RESOURCE_DESC& desc,
                                                   GrProtected isProtected,
                                                   GrD3DTextureResourceInfo* info) {
    if (0 == desc.Width || 0 == desc.Height) {
        return false;
    }
    // TODO: We don't support protected memory at the moment
    if (isProtected == GrProtected::kYes) {
        return false;
    }
    // If MipLevels is 0, for some formats the API will automatically calculate the maximum
    // number of levels supported and use that -- we don't support that.
    SkASSERT(desc.MipLevels > 0);

    ID3D12Resource* resource = nullptr;

    // TODO: incorporate D3Dx12.h and use CD3DX12_HEAP_PROPERTIES instead?
    D3D12_HEAP_PROPERTIES heapProperties = {
        D3D12_HEAP_TYPE_DEFAULT,
        D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
        D3D12_MEMORY_POOL_UNKNOWN,
        1, // CreationNodeMask
        1  // VisibleNodeMask
    };
    HRESULT hr = gpu->device()->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,  // TODO: might want to set pOptimizedClearValue for rendertarget and stencil
        IID_PPV_ARGS(&resource));
    if (!SUCCEEDED(hr)) {
        return false;
    }

    info->fResource.reset(resource);
    info->fResourceState = D3D12_RESOURCE_STATE_COMMON;
    info->fFormat = desc.Format;
    info->fLevelCount = desc.MipLevels;
    info->fProtected = isProtected;

    return true;
}

void GrD3DTextureResource::ReleaseTextureResourceInfo(GrD3DTextureResourceInfo* info) {
    info->fResource->Release();
}

GrD3DTextureResource::~GrD3DTextureResource() {
    // Should have been reset() before
    SkASSERT(!fResource);
}

void GrD3DTextureResource::releaseResource(GrD3DGpu* gpu) {
    // TODO: do we need to migrate resource state if we change queues?
    if (fResource) {
        fResource->removeOwningTexture();
        fResource.reset(nullptr);
    }
}

void GrD3DTextureResource::setResourceRelease(sk_sp<GrRefCntedCallback> releaseHelper) {
    SkASSERT(fResource);
    // Forward the release proc on to GrD3DTextureResource::Resource
    fResource->setRelease(std::move(releaseHelper));
}

void GrD3DTextureResource::Resource::freeGPUData() const {
    this->invokeReleaseProc();
    fResource.reset();  // Release our ref to the resource
}
