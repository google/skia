/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DRenderTarget.h"

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/d3d/GrD3DTypes.h"
#include "src/gpu/GrRenderTargetPriv.h"
#include "src/gpu/d3d/GrD3DGpu.h"
#include "src/gpu/d3d/GrD3DResourceProvider.h"
#include "src/gpu/d3d/GrD3DTextureResource.h"
#include "src/gpu/d3d/GrD3DUtil.h"

// We're virtually derived from GrSurface (via GrRenderTarget) so its
// constructor must be explicitly called.
GrD3DRenderTarget::GrD3DRenderTarget(GrD3DGpu* gpu,
                                     SkISize dimensions,
                                     int sampleCnt,
                                     const GrD3DTextureResourceInfo& info,
                                     sk_sp<GrD3DResourceState> state,
                                     const GrD3DTextureResourceInfo& msaaInfo,
                                     sk_sp<GrD3DResourceState> msaaState,
                                     const D3D12_CPU_DESCRIPTOR_HANDLE& colorRenderTargetView,
                                     const D3D12_CPU_DESCRIPTOR_HANDLE& resolveRenderTargetView,
                                     Wrapped)
        : GrSurface(gpu, dimensions, info.fProtected)
        , GrD3DTextureResource(info, std::move(state))
        // for the moment we only support 1:1 color to stencil
        , GrRenderTarget(gpu, dimensions, sampleCnt, info.fProtected)
        , fMSAATextureResource(new GrD3DTextureResource(msaaInfo, std::move(msaaState)))
        , fColorRenderTargetView(colorRenderTargetView)
        , fResolveRenderTargetView(resolveRenderTargetView) {
    SkASSERT(info.fProtected == msaaInfo.fProtected);
    SkASSERT(sampleCnt > 1);
    this->registerWithCacheWrapped(GrWrapCacheable::kNo);
}

// We're virtually derived from GrSurface (via GrRenderTarget) so its
// constructor must be explicitly called.
GrD3DRenderTarget::GrD3DRenderTarget(GrD3DGpu* gpu,
                                     SkISize dimensions,
                                     int sampleCnt,
                                     const GrD3DTextureResourceInfo& info,
                                     sk_sp<GrD3DResourceState> state,
                                     const GrD3DTextureResourceInfo& msaaInfo,
                                     sk_sp<GrD3DResourceState> msaaState,
                                     const D3D12_CPU_DESCRIPTOR_HANDLE& colorRenderTargetView,
                                     const D3D12_CPU_DESCRIPTOR_HANDLE& resolveRenderTargetView)
        : GrSurface(gpu, dimensions, info.fProtected)
        , GrD3DTextureResource(info, std::move(state))
        // for the moment we only support 1:1 color to stencil
        , GrRenderTarget(gpu, dimensions, sampleCnt, info.fProtected)
        , fMSAATextureResource(new GrD3DTextureResource(msaaInfo, std::move(msaaState)))
        , fColorRenderTargetView(colorRenderTargetView)
        , fResolveRenderTargetView(resolveRenderTargetView) {
    SkASSERT(info.fProtected == msaaInfo.fProtected);
    SkASSERT(sampleCnt > 1);
}

// We're virtually derived from GrSurface (via GrRenderTarget) so its
// constructor must be explicitly called.
GrD3DRenderTarget::GrD3DRenderTarget(GrD3DGpu* gpu,
                                     SkISize dimensions,
                                     const GrD3DTextureResourceInfo& info,
                                     sk_sp<GrD3DResourceState> state,
                                     const D3D12_CPU_DESCRIPTOR_HANDLE& renderTargetView,
                                     Wrapped)
        : GrSurface(gpu, dimensions, info.fProtected)
        , GrD3DTextureResource(info, std::move(state))
        , GrRenderTarget(gpu, dimensions, 1, info.fProtected)
        , fMSAATextureResource(nullptr)
        , fColorRenderTargetView(renderTargetView) {
    this->registerWithCacheWrapped(GrWrapCacheable::kNo);
}

// We're virtually derived from GrSurface (via GrRenderTarget) so its
// constructor must be explicitly called.
GrD3DRenderTarget::GrD3DRenderTarget(GrD3DGpu* gpu,
                                     SkISize dimensions,
                                     const GrD3DTextureResourceInfo& info,
                                     sk_sp<GrD3DResourceState> state,
                                     const D3D12_CPU_DESCRIPTOR_HANDLE& renderTargetView)
        : GrSurface(gpu, dimensions, info.fProtected)
        , GrD3DTextureResource(info, std::move(state))
        , GrRenderTarget(gpu, dimensions, 1, info.fProtected)
        , fMSAATextureResource(nullptr)
        , fColorRenderTargetView(renderTargetView) {}

sk_sp<GrD3DRenderTarget> GrD3DRenderTarget::MakeWrappedRenderTarget(
            GrD3DGpu* gpu, SkISize dimensions, int sampleCnt, const GrD3DTextureResourceInfo& info,
            sk_sp<GrD3DResourceState> state) {
    SkASSERT(info.fResource.get());

    SkASSERT(1 == info.fLevelCount);
    DXGI_FORMAT dxgiFormat = info.fFormat;

    D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView =
            gpu->resourceProvider().createRenderTargetView(info.fResource.get());

    // create msaa surface if necessary
    GrD3DRenderTarget* d3dRT;
    if (sampleCnt > 1) {
        D3D12_RESOURCE_DESC msTextureDesc = {};
        msTextureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        msTextureDesc.Alignment = 0;  // Default alignment (64KB)
        msTextureDesc.Width = dimensions.fWidth;
        msTextureDesc.Height = dimensions.fHeight;
        msTextureDesc.DepthOrArraySize = 1;
        msTextureDesc.MipLevels = 1;
        msTextureDesc.Format = dxgiFormat;
        msTextureDesc.SampleDesc.Count = sampleCnt;
        // quality levels are only supported for tiled resources so ignore for now
        msTextureDesc.SampleDesc.Quality = GrD3DTextureResource::kDefaultQualityLevel;
        msTextureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;  // Use default for dxgi format
        msTextureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

        D3D12_CLEAR_VALUE clearValue = {};
        clearValue.Format = dxgiFormat;
        // For wrapped rendertargets we assume a clear to white
        clearValue.Color[0] = 1;
        clearValue.Color[1] = 1;
        clearValue.Color[2] = 1;
        clearValue.Color[3] = 1;

        GrD3DTextureResourceInfo msInfo;
        sk_sp<GrD3DResourceState> msState;
        if (!GrD3DTextureResource::InitTextureResourceInfo(gpu, msTextureDesc,
                                                           D3D12_RESOURCE_STATE_RENDER_TARGET,
                                                           info.fProtected, &clearValue, &msInfo)) {
            return nullptr;
        }

        msState.reset(new GrD3DResourceState(
                                  static_cast<D3D12_RESOURCE_STATES>(msInfo.fResourceState)));

        D3D12_CPU_DESCRIPTOR_HANDLE msaaRenderTargetView =
                gpu->resourceProvider().createRenderTargetView(msInfo.fResource.get());

        d3dRT = new GrD3DRenderTarget(gpu, dimensions, sampleCnt, info, std::move(state), msInfo,
                                      std::move(msState), msaaRenderTargetView,
                                      renderTargetView, kWrapped);
    } else {
        d3dRT = new GrD3DRenderTarget(gpu, dimensions, info, std::move(state), renderTargetView,
                                      kWrapped);
    }

    return sk_sp<GrD3DRenderTarget>(d3dRT);
}

GrD3DRenderTarget::~GrD3DRenderTarget() {
    // either release or abandon should have been called by the owner of this object.
    SkASSERT(!fMSAATextureResource);
}

void GrD3DRenderTarget::releaseInternalObjects() {
    GrD3DGpu* gpu = this->getD3DGpu();

    if (fMSAATextureResource) {
        fMSAATextureResource->releaseResource(gpu);
        fMSAATextureResource.reset();
        gpu->resourceProvider().recycleRenderTargetView(&fResolveRenderTargetView);
    }

    gpu->resourceProvider().recycleRenderTargetView(&fColorRenderTargetView);
}

void GrD3DRenderTarget::onRelease() {
    this->releaseInternalObjects();
    this->releaseResource(this->getD3DGpu());
    GrRenderTarget::onRelease();
}

void GrD3DRenderTarget::onAbandon() {
    this->releaseInternalObjects();
    this->releaseResource(this->getD3DGpu());
    GrRenderTarget::onAbandon();
}

GrBackendRenderTarget GrD3DRenderTarget::getBackendRenderTarget() const {
    return GrBackendRenderTarget(this->width(), this->height(), this->numSamples(), fInfo,
                                 this->grD3DResourceState());
}

GrD3DGpu* GrD3DRenderTarget::getD3DGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrD3DGpu*>(this->getGpu());
}

DXGI_FORMAT GrD3DRenderTarget::stencilDxgiFormat() const {
    if (auto stencil = this->renderTargetPriv().getStencilAttachment()) {
        auto d3dStencil = static_cast<GrD3DStencilAttachment*>(stencil);
        return d3dStencil->dxgiFormat();
    }
    return DXGI_FORMAT_UNKNOWN;
}

void GrD3DRenderTarget::genKey(GrProcessorKeyBuilder* b) const {
    b->add32(this->dxgiFormat());
    b->add32(this->numSamples());
    b->add32(this->stencilDxgiFormat());
#ifdef SK_DEBUG
    if (const GrStencilAttachment* stencil = this->renderTargetPriv().getStencilAttachment()) {
        SkASSERT(stencil->numSamples() == this->numSamples());
    }
#endif
    b->add32(this->sampleQualityLevel());
}
