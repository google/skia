/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DTextureRenderTarget.h"

#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/d3d/GrD3DGpu.h"

GrD3DTextureRenderTarget::GrD3DTextureRenderTarget(GrD3DGpu* gpu,
                                                   SkBudgeted budgeted,
                                                   SkISize dimensions,
                                                   int sampleCnt,
                                                   const GrD3DTextureResourceInfo& info,
                                                   sk_sp<GrD3DResourceState> state,
                                                   const GrD3DTextureResourceInfo& msaaInfo,
                                                   sk_sp<GrD3DResourceState> msaaState,
                                                   GrMipMapsStatus mipMapsStatus)
        : GrSurface(gpu, dimensions, info.fProtected)
        , GrD3DTextureResource(info, state)
        , GrD3DTexture(gpu, dimensions, info, state, mipMapsStatus)
        , GrD3DRenderTarget(gpu, dimensions, sampleCnt, info, state, msaaInfo,
                            std::move(msaaState)) {
    SkASSERT(info.fProtected == msaaInfo.fProtected);
    this->registerWithCache(budgeted);
}

GrD3DTextureRenderTarget::GrD3DTextureRenderTarget(GrD3DGpu* gpu,
                                                   SkBudgeted budgeted,
                                                   SkISize dimensions,
                                                   const GrD3DTextureResourceInfo& info,
                                                   sk_sp<GrD3DResourceState> state,
                                                   GrMipMapsStatus mipMapsStatus)
        : GrSurface(gpu, dimensions, info.fProtected)
        , GrD3DTextureResource(info, state)
        , GrD3DTexture(gpu, dimensions, info, state, mipMapsStatus)
        , GrD3DRenderTarget(gpu, dimensions, info, state) {
    this->registerWithCache(budgeted);
}

GrD3DTextureRenderTarget::GrD3DTextureRenderTarget(GrD3DGpu* gpu,
                                                   SkISize dimensions,
                                                   int sampleCnt,
                                                   const GrD3DTextureResourceInfo& info,
                                                   sk_sp<GrD3DResourceState> state,
                                                   const GrD3DTextureResourceInfo& msaaInfo,
                                                   sk_sp<GrD3DResourceState> msaaState,
                                                   GrMipMapsStatus mipMapsStatus,
                                                   GrWrapCacheable cacheable)
        : GrSurface(gpu, dimensions, info.fProtected)
        , GrD3DTextureResource(info, state)
        , GrD3DTexture(gpu, dimensions, info, state, mipMapsStatus)
        , GrD3DRenderTarget(gpu, dimensions, sampleCnt, info, state, msaaInfo,
                            std::move(msaaState)) {
    SkASSERT(info.fProtected == msaaInfo.fProtected);
    this->registerWithCacheWrapped(cacheable);
}

GrD3DTextureRenderTarget::GrD3DTextureRenderTarget(GrD3DGpu* gpu,
                                                   SkISize dimensions,
                                                   const GrD3DTextureResourceInfo& info,
                                                   sk_sp<GrD3DResourceState> state,
                                                   GrMipMapsStatus mipMapsStatus,
                                                   GrWrapCacheable cacheable)
        : GrSurface(gpu, dimensions, info.fProtected)
        , GrD3DTextureResource(info, state)
        , GrD3DTexture(gpu, dimensions, info, state, mipMapsStatus)
        , GrD3DRenderTarget(gpu, dimensions, info, state) {
    this->registerWithCacheWrapped(cacheable);
}

static std::pair<GrD3DTextureResourceInfo, sk_sp<GrD3DResourceState>> create_msaa_resource(
        GrD3DGpu* gpu, SkISize dimensions, int sampleCnt, const GrD3DTextureResourceInfo& info) {
    GrD3DTextureResourceInfo msInfo;
    sk_sp<GrD3DResourceState> msState;

    // create msaa surface
    D3D12_RESOURCE_DESC msTextureDesc;
    msTextureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    msTextureDesc.Alignment = 0;  // Default alignment (64KB)
    msTextureDesc.Width = dimensions.fWidth;
    msTextureDesc.Height = dimensions.fHeight;
    msTextureDesc.DepthOrArraySize = 1;
    msTextureDesc.MipLevels = 1;
    msTextureDesc.Format = info.fFormat;
    msTextureDesc.SampleDesc.Count = sampleCnt;
    msTextureDesc.SampleDesc.Quality = 0;  // TODO: only valid for tiled renderers
    msTextureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;  // Use default for dxgi format
    msTextureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    if (!GrD3DTextureResource::InitTextureResourceInfo(gpu, msTextureDesc, info.fProtected,
                                                       &msInfo)) {
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
    if (!GrD3DTextureResource::InitTextureResourceInfo(gpu, resourceDesc, isProtected, &info)) {
        return nullptr;
    }
    sk_sp<GrD3DResourceState> state(new GrD3DResourceState(
                                          static_cast<D3D12_RESOURCE_STATES>(info.fResourceState)));

    if (sampleCnt > 1) {
        GrD3DTextureResourceInfo msInfo;
        sk_sp<GrD3DResourceState> msState;

        std::tie(msInfo, msState) = create_msaa_resource(gpu, dimensions, sampleCnt, info);

        GrD3DTextureRenderTarget* trt = new GrD3DTextureRenderTarget(
                gpu, budgeted, dimensions, sampleCnt, info, std::move(state),
                msInfo, std::move(msState), mipMapsStatus);

        // The GrD3DTextureRenderTarget takes a ref on the textures so we need to release ours
        GrD3DTextureResource::ReleaseTextureResourceInfo(&msInfo);
        GrD3DTextureResource::ReleaseTextureResourceInfo(&info);

        return sk_sp<GrD3DTextureRenderTarget>(trt);
    } else {
        GrD3DTextureRenderTarget* trt = new GrD3DTextureRenderTarget(
                gpu, budgeted, dimensions, info, std::move(state), mipMapsStatus);
        // The GrD3DTextureRenderTarget takes a ref on the texture so we need to release ours
        GrD3DTextureResource::ReleaseTextureResourceInfo(&info);
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

    if (sampleCnt > 1) {
        GrD3DTextureResourceInfo msInfo;
        sk_sp<GrD3DResourceState> msState;

        std::tie(msInfo, msState) = create_msaa_resource(gpu, dimensions, sampleCnt, info);
        GrD3DTextureRenderTarget* trt = new GrD3DTextureRenderTarget(
                gpu, dimensions, sampleCnt, info, std::move(state), msInfo, std::move(msState),
                mipMapsStatus, cacheable);
        // The GrD3DTexture takes a ref on the msaa texture so we need to release ours
        GrD3DTextureResource::ReleaseTextureResourceInfo(&msInfo);

        return sk_sp<GrD3DTextureRenderTarget>(trt);
    } else {
        return sk_sp<GrD3DTextureRenderTarget>(new GrD3DTextureRenderTarget(
                gpu, dimensions, info, std::move(state), mipMapsStatus, cacheable));
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
