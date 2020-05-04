/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DTextureRenderTarget.h"

#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/d3d/GrD3DGpu.h"

GrD3DTextureRenderTarget::GrD3DTextureRenderTarget(
        GrD3DGpu* gpu, SkBudgeted budgeted, SkISize dimensions, int sampleCnt,
        const GrD3DTextureResourceInfo& info, sk_sp<GrD3DResourceState> state,
        const GrD3DTextureResourceInfo& msaaInfo, sk_sp<GrD3DResourceState> msaaState,
        const D3D12_CPU_DESCRIPTOR_HANDLE& colorRenderTargetView,
        const D3D12_CPU_DESCRIPTOR_HANDLE& resolveRenderTargetView,
        GrMipMapsStatus mipMapsStatus)
    : GrSurface(gpu, dimensions, info.fProtected)
    , GrD3DTextureResource(info, state)
    , GrD3DTexture(gpu, dimensions, info, state, mipMapsStatus)
    , GrD3DRenderTarget(gpu, dimensions, sampleCnt, info, state, msaaInfo,
                        std::move(msaaState), colorRenderTargetView, resolveRenderTargetView) {
    SkASSERT(info.fProtected == msaaInfo.fProtected);
    this->registerWithCache(budgeted);
}

GrD3DTextureRenderTarget::GrD3DTextureRenderTarget(
        GrD3DGpu* gpu, SkBudgeted budgeted, SkISize dimensions,
        const GrD3DTextureResourceInfo& info, sk_sp<GrD3DResourceState> state,
        const D3D12_CPU_DESCRIPTOR_HANDLE& renderTargetView,
        GrMipMapsStatus mipMapsStatus)
    : GrSurface(gpu, dimensions, info.fProtected)
    , GrD3DTextureResource(info, state)
    , GrD3DTexture(gpu, dimensions, info, state, mipMapsStatus)
    , GrD3DRenderTarget(gpu, dimensions, info, state, renderTargetView) {
    this->registerWithCache(budgeted);
}

GrD3DTextureRenderTarget::GrD3DTextureRenderTarget(
        GrD3DGpu* gpu, SkISize dimensions, int sampleCnt,
        const GrD3DTextureResourceInfo& info, sk_sp<GrD3DResourceState> state,
        const GrD3DTextureResourceInfo& msaaInfo, sk_sp<GrD3DResourceState> msaaState,
        const D3D12_CPU_DESCRIPTOR_HANDLE& colorRenderTargetView,
        const D3D12_CPU_DESCRIPTOR_HANDLE& resolveRenderTargetView,
        GrMipMapsStatus mipMapsStatus,
        GrWrapCacheable cacheable)
    : GrSurface(gpu, dimensions, info.fProtected)
    , GrD3DTextureResource(info, state)
    , GrD3DTexture(gpu, dimensions, info, state, mipMapsStatus)
    , GrD3DRenderTarget(gpu, dimensions, sampleCnt, info, state, msaaInfo,
                        std::move(msaaState), colorRenderTargetView, resolveRenderTargetView) {
    SkASSERT(info.fProtected == msaaInfo.fProtected);
    this->registerWithCacheWrapped(cacheable);
}

GrD3DTextureRenderTarget::GrD3DTextureRenderTarget(
        GrD3DGpu* gpu, SkISize dimensions,
        const GrD3DTextureResourceInfo& info, sk_sp<GrD3DResourceState> state,
        const D3D12_CPU_DESCRIPTOR_HANDLE& renderTargetView,
        GrMipMapsStatus mipMapsStatus,
        GrWrapCacheable cacheable)
    : GrSurface(gpu, dimensions, info.fProtected)
    , GrD3DTextureResource(info, state)
    , GrD3DTexture(gpu, dimensions, info, state, mipMapsStatus)
    , GrD3DRenderTarget(gpu, dimensions, info, state, renderTargetView) {
    this->registerWithCacheWrapped(cacheable);
}

static std::pair<GrD3DTextureResourceInfo, sk_sp<GrD3DResourceState>> create_msaa_resource(
        GrD3DGpu* gpu, SkISize dimensions, int sampleCnt, const GrD3DTextureResourceInfo& info) {
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
    // quality levels are only supported for tiled resources so ignore for now
    msTextureDesc.SampleDesc.Quality = GrD3DTextureResource::kDefaultQualityLevel;
    msTextureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;  // Use default for dxgi format
    msTextureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = info.fFormat;
    clearValue.Color[0] = 1;
    clearValue.Color[1] = 1;
    clearValue.Color[2] = 1;
    clearValue.Color[3] = 1;

    if (!GrD3DTextureResource::InitTextureResourceInfo(gpu, msTextureDesc,
                                                       D3D12_RESOURCE_STATE_RENDER_TARGET,
                                                       info.fProtected, &clearValue, &msInfo)) {
        return {};
    }

    msState.reset(new GrD3DResourceState(
                              static_cast<D3D12_RESOURCE_STATES>(msInfo.fResourceState)));

    return std::make_pair(msInfo, msState);
}

sk_sp<GrD3DTextureRenderTarget> GrD3DTextureRenderTarget::MakeNewTextureRenderTarget(
        GrD3DGpu* gpu,
        SkBudgeted budgeted,
        SkISize dimensions,
        int sampleCnt,
        const D3D12_RESOURCE_DESC& resourceDesc,
        GrProtected isProtected,
        GrMipMapsStatus mipMapsStatus) {

    GrD3DTextureResourceInfo info;
    D3D12_RESOURCE_STATES initialState = sampleCnt > 1 ? D3D12_RESOURCE_STATE_RESOLVE_DEST
                                                       : D3D12_RESOURCE_STATE_RENDER_TARGET;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = resourceDesc.Format;
    clearValue.Color[0] = 1;
    clearValue.Color[1] = 1;
    clearValue.Color[2] = 1;
    clearValue.Color[3] = 1;

    if (!GrD3DTextureResource::InitTextureResourceInfo(gpu, resourceDesc, initialState,
                                                       isProtected, &clearValue, &info)) {
        return nullptr;
    }
    sk_sp<GrD3DResourceState> state(new GrD3DResourceState(
                                          static_cast<D3D12_RESOURCE_STATES>(info.fResourceState)));

    const D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView =
        gpu->resourceProvider().createRenderTargetView(info.fResource.get());

    if (sampleCnt > 1) {
        GrD3DTextureResourceInfo msInfo;
        sk_sp<GrD3DResourceState> msState;

        std::tie(msInfo, msState) = create_msaa_resource(gpu, dimensions, sampleCnt, info);

        const D3D12_CPU_DESCRIPTOR_HANDLE msaaRenderTargetView =
            gpu->resourceProvider().createRenderTargetView(msInfo.fResource.get());

        GrD3DTextureRenderTarget* trt = new GrD3DTextureRenderTarget(
                gpu, budgeted, dimensions, sampleCnt, info, std::move(state),
                msInfo, std::move(msState), msaaRenderTargetView, renderTargetView, mipMapsStatus);
        return sk_sp<GrD3DTextureRenderTarget>(trt);
    } else {
        GrD3DTextureRenderTarget* trt = new GrD3DTextureRenderTarget(
                gpu, budgeted, dimensions, info, std::move(state), renderTargetView, mipMapsStatus);
        return sk_sp<GrD3DTextureRenderTarget>(trt);
    }
}

sk_sp<GrD3DTextureRenderTarget> GrD3DTextureRenderTarget::MakeWrappedTextureRenderTarget(
        GrD3DGpu* gpu,
        SkISize dimensions,
        int sampleCnt,
        GrWrapCacheable cacheable,
        const GrD3DTextureResourceInfo& info,
        sk_sp<GrD3DResourceState> state) {
    // TODO: If a client uses their own heap to allocate, how do we manage that?
    // Adopted textures require both image and allocation because we're responsible for freeing
    //SkASSERT(VK_NULL_HANDLE != info.fImage &&
    //         (kBorrow_GrWrapOwnership == wrapOwnership || VK_NULL_HANDLE != info.fAlloc.fMemory));

    GrMipMapsStatus mipMapsStatus = info.fLevelCount > 1 ? GrMipMapsStatus::kDirty
                                                         : GrMipMapsStatus::kNotAllocated;

    const D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView =
        gpu->resourceProvider().createRenderTargetView(info.fResource.get());

    if (sampleCnt > 1) {
        GrD3DTextureResourceInfo msInfo;
        sk_sp<GrD3DResourceState> msState;

        std::tie(msInfo, msState) = create_msaa_resource(gpu, dimensions, sampleCnt, info);
        const D3D12_CPU_DESCRIPTOR_HANDLE msaaRenderTargetView =
                gpu->resourceProvider().createRenderTargetView(msInfo.fResource.get());

        GrD3DTextureRenderTarget* trt = new GrD3DTextureRenderTarget(
                gpu, dimensions, sampleCnt, info, std::move(state), msInfo, std::move(msState),
                msaaRenderTargetView, renderTargetView, mipMapsStatus, cacheable);
        return sk_sp<GrD3DTextureRenderTarget>(trt);
    } else {
        return sk_sp<GrD3DTextureRenderTarget>(new GrD3DTextureRenderTarget(
                gpu, dimensions, info, std::move(state), renderTargetView, mipMapsStatus,
                cacheable));
    }
}

size_t GrD3DTextureRenderTarget::onGpuMemorySize() const {
    int numColorSamples = this->numSamples();
    if (numColorSamples > 1) {
        // Add one to account for the resolve VkImage.
        ++numColorSamples;
    }
    const GrCaps& caps = *this->getGpu()->caps();
    return GrSurface::ComputeSize(caps, this->backendFormat(), this->dimensions(),
                                  numColorSamples,  // TODO: this still correct?
                                  this->texturePriv().mipMapped());
}
