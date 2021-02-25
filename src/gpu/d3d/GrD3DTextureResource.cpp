/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrGpuResourcePriv.h"
#include "src/gpu/d3d/GrD3DAMDMemoryAllocator.h"
#include "src/gpu/d3d/GrD3DGpu.h"
#include "src/gpu/d3d/GrD3DTextureResource.h"

void GrD3DTextureResource::setResourceState(const GrD3DGpu* gpu,
                                            D3D12_RESOURCE_STATES newResourceState) {
    D3D12_RESOURCE_STATES currentResourceState = this->currentState();
    if (newResourceState == currentResourceState) {
        return;
    }

    D3D12_RESOURCE_TRANSITION_BARRIER barrier;
    barrier.pResource = this->d3dResource();
    barrier.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.StateBefore = currentResourceState;
    barrier.StateAfter = newResourceState;
    gpu->addResourceBarriers(this->resource(), 1, &barrier);

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

    info->fResource = gpu->memoryAllocator()->createResource(
            D3D12_HEAP_TYPE_DEFAULT, &desc, initialState, &info->fAlloc, clearValue);
    if (!info->fResource) {
        return false;
    }

    info->fResourceState = initialState;
    info->fFormat = desc.Format;
    info->fLevelCount = desc.MipLevels;
    info->fSampleCount = desc.SampleDesc.Count;
    info->fSampleQualityPattern = desc.SampleDesc.Quality;
    info->fProtected = isProtected;

    return true;
}

std::pair<GrD3DTextureResourceInfo, sk_sp<GrD3DResourceState>> GrD3DTextureResource::CreateMSAA(
        GrD3DGpu* gpu, SkISize dimensions, int sampleCnt, const GrD3DTextureResourceInfo& info,
        SkColor4f clearColor) {
    GrD3DTextureResourceInfo msInfo;
    sk_sp<GrD3DResourceState> msState;

    // create msaa surface
    D3D12_RESOURCE_DESC msTextureDesc = {};
    msTextureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    msTextureDesc.Alignment = 0;  // Default alignment (64KB)
    msTextureDesc.Width = dimensions.fWidth;
    msTextureDesc.Height = dimensions.fHeight;
    msTextureDesc.DepthOrArraySize = 1;
    msTextureDesc.MipLevels = 1;
    msTextureDesc.Format = info.fFormat;
    msTextureDesc.SampleDesc.Count = sampleCnt;
    msTextureDesc.SampleDesc.Quality = DXGI_STANDARD_MULTISAMPLE_QUALITY_PATTERN;
    msTextureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;  // Use default for dxgi format
    msTextureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = info.fFormat;
    clearValue.Color[0] = clearColor.fR;
    clearValue.Color[1] = clearColor.fG;
    clearValue.Color[2] = clearColor.fB;
    clearValue.Color[3] = clearColor.fA;

    if (!InitTextureResourceInfo(gpu, msTextureDesc, D3D12_RESOURCE_STATE_RENDER_TARGET,
                                 info.fProtected, &clearValue, &msInfo)) {
        return {};
    }

    msState.reset(new GrD3DResourceState(
            static_cast<D3D12_RESOURCE_STATES>(msInfo.fResourceState)));

    return std::make_pair(msInfo, msState);
}

GrD3DTextureResource::~GrD3DTextureResource() {
    // Should have been reset() before
    SkASSERT(!fResource);
    SkASSERT(!fInfo.fResource);
}

void GrD3DTextureResource::prepareForPresent(GrD3DGpu* gpu) {
    this->setResourceState(gpu, D3D12_RESOURCE_STATE_PRESENT);
}

void GrD3DTextureResource::releaseResource(GrD3DGpu* gpu) {
    // TODO: do we need to migrate resource state if we change queues?
    if (fResource) {
        fResource.reset();
    }
    fInfo.fResource.reset();
    fInfo.fAlloc.reset();
}

void GrD3DTextureResource::setResourceRelease(sk_sp<GrRefCntedCallback> releaseHelper) {
    SkASSERT(fResource);
    // Forward the release proc on to GrD3DTextureResource::Resource
    fResource->setRelease(std::move(releaseHelper));
}

void GrD3DTextureResource::Resource::freeGPUData() const {
    this->invokeReleaseProc();
    fResource.reset();  // Release our ref to the resource
    fAlloc.reset(); // Release our ref to the allocation
}
