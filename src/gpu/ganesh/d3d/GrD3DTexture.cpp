/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/d3d/GrD3DTexture.h"

#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/d3d/GrD3DGpu.h"
#include "src/gpu/ganesh/d3d/GrD3DUtil.h"

#include "include/gpu/d3d/GrD3DTypes.h"

// Because this class is virtually derived from GrSurface we must explicitly call its constructor.
GrD3DTexture::GrD3DTexture(GrD3DGpu* gpu,
                           skgpu::Budgeted budgeted,
                           SkISize dimensions,
                           const GrD3DTextureResourceInfo& info,
                           sk_sp<GrD3DResourceState> state,
                           const GrD3DDescriptorHeap::CPUHandle& shaderResourceView,
                           GrMipmapStatus mipmapStatus,
                           std::string_view label)
        : GrSurface(gpu, dimensions, info.fProtected, label)
        , GrD3DTextureResource(info, std::move(state))
        , INHERITED(gpu, dimensions, info.fProtected, GrTextureType::k2D, mipmapStatus, label)
        , fShaderResourceView(shaderResourceView) {
    SkASSERT((GrMipmapStatus::kNotAllocated == mipmapStatus) == (1 == info.fLevelCount));
    this->registerWithCache(budgeted);
    if (GrDxgiFormatIsCompressed(info.fFormat)) {
        this->setReadOnly();
    }
}

GrD3DTexture::GrD3DTexture(GrD3DGpu* gpu, SkISize dimensions, const GrD3DTextureResourceInfo& info,
                           sk_sp<GrD3DResourceState> state,
                           const GrD3DDescriptorHeap::CPUHandle& shaderResourceView,
                           GrMipmapStatus mipmapStatus, GrWrapCacheable cacheable,
                           GrIOType ioType,
                           std::string_view label)
        : GrSurface(gpu, dimensions, info.fProtected, label)
        , GrD3DTextureResource(info, std::move(state))
        , INHERITED(gpu, dimensions, info.fProtected, GrTextureType::k2D, mipmapStatus, label)
        , fShaderResourceView(shaderResourceView) {
    SkASSERT((GrMipmapStatus::kNotAllocated == mipmapStatus) == (1 == info.fLevelCount));
    if (ioType == kRead_GrIOType) {
        this->setReadOnly();
    }
    this->registerWithCacheWrapped(cacheable);
}

// Because this class is virtually derived from GrSurface we must explicitly call its constructor.
GrD3DTexture::GrD3DTexture(GrD3DGpu* gpu,
                           SkISize dimensions,
                           const GrD3DTextureResourceInfo& info,
                           sk_sp<GrD3DResourceState> state,
                           const GrD3DDescriptorHeap::CPUHandle& shaderResourceView,
                           GrMipmapStatus mipmapStatus,
                           std::string_view label)
        : GrSurface(gpu, dimensions, info.fProtected, label)
        , GrD3DTextureResource(info, state)
        , INHERITED(gpu, dimensions, info.fProtected, GrTextureType::k2D, mipmapStatus, label)
        , fShaderResourceView(shaderResourceView) {
    SkASSERT((GrMipmapStatus::kNotAllocated == mipmapStatus) == (1 == info.fLevelCount));
}

sk_sp<GrD3DTexture> GrD3DTexture::MakeNewTexture(GrD3DGpu* gpu,
                                                 skgpu::Budgeted budgeted,
                                                 SkISize dimensions,
                                                 const D3D12_RESOURCE_DESC& desc,
                                                 GrProtected isProtected,
                                                 GrMipmapStatus mipmapStatus,
                                                 std::string_view label) {
    GrD3DTextureResourceInfo info;
    if (!GrD3DTextureResource::InitTextureResourceInfo(gpu, desc,
                                                       D3D12_RESOURCE_STATE_COPY_DEST,
                                                       isProtected, nullptr, &info)) {
        return nullptr;
    }

    sk_sp<GrD3DResourceState> state(
            new GrD3DResourceState(static_cast<D3D12_RESOURCE_STATES>(info.fResourceState)));

    GrD3DDescriptorHeap::CPUHandle shaderResourceView =
            gpu->resourceProvider().createShaderResourceView(info.fResource.get());

    GrD3DTexture* tex = new GrD3DTexture(gpu, budgeted, dimensions, info, std::move(state),
                                         shaderResourceView,
                                         mipmapStatus,
                                         label);

    return sk_sp<GrD3DTexture>(tex);
}

sk_sp<GrD3DTexture> GrD3DTexture::MakeWrappedTexture(GrD3DGpu* gpu,
                                                     SkISize dimensions,
                                                     GrWrapCacheable cacheable,
                                                     GrIOType ioType,
                                                     const GrD3DTextureResourceInfo& info,
                                                     sk_sp<GrD3DResourceState> state) {
    // TODO: If a client uses their own heap to allocate, how do we manage that?
    // Adopted textures require both image and allocation because we're responsible for freeing
    //SkASSERT(info.fTexture &&
    //         (kBorrow_GrWrapOwnership == wrapOwnership || VK_NULL_HANDLE != info.fAlloc.fMemory));

    GrMipmapStatus mipmapStatus = info.fLevelCount > 1 ? GrMipmapStatus::kValid
                                                       : GrMipmapStatus::kNotAllocated;

    GrD3DDescriptorHeap::CPUHandle shaderResourceView =
            gpu->resourceProvider().createShaderResourceView(info.fResource.get());

    return sk_sp<GrD3DTexture>(new GrD3DTexture(gpu, dimensions, info, std::move(state),
                                                shaderResourceView, mipmapStatus, cacheable,
                                                ioType,
                                                /*label=*/"D3DWrappedTexture"));
}

sk_sp<GrD3DTexture> GrD3DTexture::MakeAliasingTexture(GrD3DGpu* gpu,
                                                      sk_sp<GrD3DTexture> originalTexture,
                                                      const D3D12_RESOURCE_DESC& newDesc,
                                                      D3D12_RESOURCE_STATES resourceState) {
    GrD3DTextureResourceInfo info = originalTexture->fInfo;
    info.fResource = gpu->memoryAllocator()->createAliasingResource(info.fAlloc, 0, &newDesc,
                                                                    resourceState, nullptr);
    if (!info.fResource) {
        return nullptr;
    }
    info.fResourceState = resourceState;

    sk_sp<GrD3DResourceState> state(
        new GrD3DResourceState(static_cast<D3D12_RESOURCE_STATES>(resourceState)));

    GrD3DDescriptorHeap::CPUHandle shaderResourceView =
        gpu->resourceProvider().createShaderResourceView(info.fResource.get());

    GrD3DTexture* tex = new GrD3DTexture(gpu,
                                         skgpu::Budgeted::kNo,
                                         originalTexture->dimensions(),
                                         info,
                                         std::move(state),
                                         shaderResourceView,
                                         originalTexture->mipmapStatus(),
                                         /*label=*/"AliasingTexture");
    return sk_sp<GrD3DTexture>(tex);
}

void GrD3DTexture::onRelease() {
    GrD3DGpu* gpu = this->getD3DGpu();
    gpu->resourceProvider().recycleShaderView(fShaderResourceView);
    this->releaseResource(gpu);

    INHERITED::onRelease();
}

void GrD3DTexture::onAbandon() {
    GrD3DGpu* gpu = this->getD3DGpu();
    gpu->resourceProvider().recycleShaderView(fShaderResourceView);
    this->releaseResource(gpu);
    INHERITED::onAbandon();
}

GrBackendTexture GrD3DTexture::getBackendTexture() const {
    return GrBackendTexture(this->width(), this->height(), fInfo, this->grD3DResourceState());
}

GrD3DGpu* GrD3DTexture::getD3DGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrD3DGpu*>(this->getGpu());
}

void GrD3DTexture::onSetLabel() {
    SkASSERT(this->d3dResource());
    if (!this->getLabel().empty()) {
        const std::wstring label = L"_Skia_" + GrD3DMultiByteToWide(this->getLabel());
        this->d3dResource()->SetName(label.c_str());
    }
}
