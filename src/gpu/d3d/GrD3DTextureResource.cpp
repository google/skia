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
    D3D12_RESOURCE_STATES currentResourceState = this->currentState();
    if (newResourceState == currentResourceState) {
        return;
    }

    SkAutoTMalloc<D3D12_RESOURCE_TRANSITION_BARRIER> barriers(fInfo.fLevelCount);
    for (uint32_t mipLevel = 0; mipLevel < fInfo.fLevelCount; ++mipLevel) {
        barriers[mipLevel].pResource = this->d3dResource();
        barriers[mipLevel].Subresource = mipLevel;
        barriers[mipLevel].StateBefore = currentResourceState;
        barriers[mipLevel].StateAfter = newResourceState;
    }
    gpu->addResourceBarriers(this->resource(), fInfo.fLevelCount, barriers.get());

    this->updateResourceState(newResourceState);
}

bool GrD3DTextureResource::InitTextureResourceInfo(GrD3DGpu* gpu, const D3D12_RESOURCE_DESC& desc,
                                                   D3D12_RESOURCE_STATES initialState,
                                                   GrProtected isProtected,
                                                   D3D12_CLEAR_VALUE* clearValue,
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

    D3D12_HEAP_PROPERTIES heapProperties = {};
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
    heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProperties.CreationNodeMask = 1;
    heapProperties.VisibleNodeMask = 1;
    HRESULT hr = gpu->device()->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        initialState,
        clearValue,
        IID_PPV_ARGS(&resource));
    if (!SUCCEEDED(hr)) {
        return false;
    }

    info->fResource.reset(resource);
    info->fResourceState = initialState;
    info->fFormat = desc.Format;
    info->fLevelCount = desc.MipLevels;
    info->fSampleQualityLevel = desc.SampleDesc.Quality;
    info->fProtected = isProtected;

    return true;
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
