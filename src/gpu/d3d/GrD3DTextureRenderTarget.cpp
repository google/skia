/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DTextureRenderTarget.h"

#include "src/gpu/GrTexture.h"
#include "src/gpu/d3d/GrD3DGpu.h"

GrD3DTextureRenderTarget::GrD3DTextureRenderTarget(
        GrD3DGpu* gpu,
        SkBudgeted budgeted,
        SkISize dimensions,
        const GrD3DTextureResourceInfo& info,
        sk_sp<GrD3DResourceState> state,
        const GrD3DDescriptorHeap::CPUHandle& shaderResourceView,
        const GrD3DTextureResourceInfo& msaaInfo,
        sk_sp<GrD3DResourceState> msaaState,
        const GrD3DDescriptorHeap::CPUHandle& colorRenderTargetView,
        const GrD3DDescriptorHeap::CPUHandle& resolveRenderTargetView,
        GrMipmapStatus mipmapStatus)
        : GrSurface(gpu, dimensions, info.fProtected)
        , GrD3DTextureResource(info, state)
        , GrD3DTexture(gpu, dimensions, info, state, shaderResourceView, mipmapStatus)
        , GrD3DRenderTarget(gpu,
                            dimensions,
                            info,
                            state,
                            msaaInfo,
                            std::move(msaaState),
                            colorRenderTargetView,
                            resolveRenderTargetView) {
    SkASSERT(info.fProtected == msaaInfo.fProtected);
    this->registerWithCache(budgeted);
}

GrD3DTextureRenderTarget::GrD3DTextureRenderTarget(
        GrD3DGpu* gpu,
        SkBudgeted budgeted,
        SkISize dimensions,
        const GrD3DTextureResourceInfo& info,
        sk_sp<GrD3DResourceState> state,
        const GrD3DDescriptorHeap::CPUHandle& shaderResourceView,
        const GrD3DDescriptorHeap::CPUHandle& renderTargetView,
        GrMipmapStatus mipmapStatus)
        : GrSurface(gpu, dimensions, info.fProtected)
        , GrD3DTextureResource(info, state)
        , GrD3DTexture(gpu, dimensions, info, state, shaderResourceView, mipmapStatus)
        , GrD3DRenderTarget(gpu, dimensions, info, state, renderTargetView) {
    this->registerWithCache(budgeted);
}

GrD3DTextureRenderTarget::GrD3DTextureRenderTarget(
        GrD3DGpu* gpu,
        SkISize dimensions,
        const GrD3DTextureResourceInfo& info,
        sk_sp<GrD3DResourceState> state,
        const GrD3DDescriptorHeap::CPUHandle& shaderResourceView,
        const GrD3DTextureResourceInfo& msaaInfo,
        sk_sp<GrD3DResourceState> msaaState,
        const GrD3DDescriptorHeap::CPUHandle& colorRenderTargetView,
        const GrD3DDescriptorHeap::CPUHandle& resolveRenderTargetView,
        GrMipmapStatus mipmapStatus,
        GrWrapCacheable cacheable)
        : GrSurface(gpu, dimensions, info.fProtected)
        , GrD3DTextureResource(info, state)
        , GrD3DTexture(gpu, dimensions, info, state, shaderResourceView, mipmapStatus)
        , GrD3DRenderTarget(gpu,
                            dimensions,
                            info,
                            state,
                            msaaInfo,
                            std::move(msaaState),
                            colorRenderTargetView,
                            resolveRenderTargetView) {
    SkASSERT(info.fProtected == msaaInfo.fProtected);
    this->registerWithCacheWrapped(cacheable);
}

GrD3DTextureRenderTarget::GrD3DTextureRenderTarget(
        GrD3DGpu* gpu,
        SkISize dimensions,
        const GrD3DTextureResourceInfo& info,
        sk_sp<GrD3DResourceState> state,
        const GrD3DDescriptorHeap::CPUHandle& shaderResourceView,
        const GrD3DDescriptorHeap::CPUHandle& renderTargetView,
        GrMipmapStatus mipmapStatus,
        GrWrapCacheable cacheable)
        : GrSurface(gpu, dimensions, info.fProtected)
        , GrD3DTextureResource(info, state)
        , GrD3DTexture(gpu, dimensions, info, state, shaderResourceView, mipmapStatus)
        , GrD3DRenderTarget(gpu, dimensions, info, state, renderTargetView) {
    this->registerWithCacheWrapped(cacheable);
}

sk_sp<GrD3DTextureRenderTarget> GrD3DTextureRenderTarget::MakeNewTextureRenderTarget(
        GrD3DGpu* gpu,
        SkBudgeted budgeted,
        SkISize dimensions,
        int sampleCnt,
        const D3D12_RESOURCE_DESC& resourceDesc,
        GrProtected isProtected,
        GrMipmapStatus mipmapStatus) {

    GrD3DTextureResourceInfo info;
    D3D12_RESOURCE_STATES initialState = sampleCnt > 1 ? D3D12_RESOURCE_STATE_RESOLVE_DEST
                                                       : D3D12_RESOURCE_STATE_RENDER_TARGET;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = resourceDesc.Format;
    clearValue.Color[0] = 0;
    clearValue.Color[1] = 0;
    clearValue.Color[2] = 0;
    clearValue.Color[3] = 0;

    if (!GrD3DTextureResource::InitTextureResourceInfo(gpu, resourceDesc, initialState,
                                                       isProtected, &clearValue, &info)) {
        return nullptr;
    }
    sk_sp<GrD3DResourceState> state(new GrD3DResourceState(
                                          static_cast<D3D12_RESOURCE_STATES>(info.fResourceState)));

    const GrD3DDescriptorHeap::CPUHandle shaderResourceView =
            gpu->resourceProvider().createShaderResourceView(info.fResource.get());

    const GrD3DDescriptorHeap::CPUHandle renderTargetView =
            gpu->resourceProvider().createRenderTargetView(info.fResource.get());

    if (sampleCnt > 1) {
        GrD3DTextureResourceInfo msInfo;
        sk_sp<GrD3DResourceState> msState;
        // created MSAA surface we assume will be used for masks, so clear to transparent black
        SkColor4f clearColor = { 0, 0, 0, 0 };
        std::tie(msInfo, msState) =
                GrD3DTextureResource::CreateMSAA(gpu, dimensions, sampleCnt, info, clearColor);

        const GrD3DDescriptorHeap::CPUHandle msaaRenderTargetView =
                gpu->resourceProvider().createRenderTargetView(msInfo.fResource.get());

        GrD3DTextureRenderTarget* trt = new GrD3DTextureRenderTarget(
                gpu, budgeted, dimensions, info, std::move(state), shaderResourceView, msInfo,
                std::move(msState), msaaRenderTargetView, renderTargetView, mipmapStatus);
        return sk_sp<GrD3DTextureRenderTarget>(trt);
    } else {
        GrD3DTextureRenderTarget* trt = new GrD3DTextureRenderTarget(
                gpu, budgeted, dimensions, info, std::move(state), shaderResourceView,
                renderTargetView, mipmapStatus);
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

    GrMipmapStatus mipmapStatus = info.fLevelCount > 1 ? GrMipmapStatus::kDirty
                                                       : GrMipmapStatus::kNotAllocated;

    const GrD3DDescriptorHeap::CPUHandle shaderResourceView =
            gpu->resourceProvider().createShaderResourceView(info.fResource.get());

    const GrD3DDescriptorHeap::CPUHandle renderTargetView =
            gpu->resourceProvider().createRenderTargetView(info.fResource.get());

    if (sampleCnt > 1) {
        GrD3DTextureResourceInfo msInfo;
        sk_sp<GrD3DResourceState> msState;
        // for wrapped MSAA surface we assume clear to white
        SkColor4f clearColor = { 1, 1, 1, 1 };
        std::tie(msInfo, msState) =
                GrD3DTextureResource::CreateMSAA(gpu, dimensions, sampleCnt, info, clearColor);

        const GrD3DDescriptorHeap::CPUHandle msaaRenderTargetView =
                gpu->resourceProvider().createRenderTargetView(msInfo.fResource.get());

        GrD3DTextureRenderTarget* trt = new GrD3DTextureRenderTarget(
                gpu, dimensions, info, std::move(state), shaderResourceView, msInfo,
                std::move(msState), msaaRenderTargetView, renderTargetView, mipmapStatus,
                cacheable);
        return sk_sp<GrD3DTextureRenderTarget>(trt);
    } else {
        return sk_sp<GrD3DTextureRenderTarget>(new GrD3DTextureRenderTarget(
                gpu, dimensions, info, std::move(state), shaderResourceView, renderTargetView,
                mipmapStatus, cacheable));
    }
}

size_t GrD3DTextureRenderTarget::onGpuMemorySize() const {
    int numColorSamples = this->numSamples();
    if (numColorSamples > 1) {
        // Add one to account for the resolve VkImage.
        ++numColorSamples;
    }
    return GrSurface::ComputeSize(this->backendFormat(), this->dimensions(),
                                  numColorSamples,  // TODO: this still correct?
                                  this->mipmapped());
}
