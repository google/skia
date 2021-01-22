/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DRenderTarget.h"

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/d3d/GrD3DTypes.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/d3d/GrD3DGpu.h"
#include "src/gpu/d3d/GrD3DResourceProvider.h"
#include "src/gpu/d3d/GrD3DTextureResource.h"
#include "src/gpu/d3d/GrD3DUtil.h"

// We're virtually derived from GrSurface (via GrRenderTarget) so its
// constructor must be explicitly called.
GrD3DRenderTarget::GrD3DRenderTarget(GrD3DGpu* gpu,
                                     SkISize dimensions,
                                     const GrD3DTextureResourceInfo& info,
                                     sk_sp<GrD3DResourceState> state,
                                     const GrD3DTextureResourceInfo& msaaInfo,
                                     sk_sp<GrD3DResourceState> msaaState,
                                     const GrD3DDescriptorHeap::CPUHandle& colorRenderTargetView,
                                     const GrD3DDescriptorHeap::CPUHandle& resolveRenderTargetView,
                                     Wrapped)
        : GrSurface(gpu, dimensions, info.fProtected)
        , GrD3DTextureResource(info, std::move(state))
        // for the moment we only support 1:1 color to stencil
        , GrRenderTarget(gpu, dimensions, msaaInfo.fSampleCount, info.fProtected)
        , fMSAATextureResource(new GrD3DTextureResource(msaaInfo, std::move(msaaState)))
        , fColorRenderTargetView(colorRenderTargetView)
        , fResolveRenderTargetView(resolveRenderTargetView) {
    SkASSERT(info.fProtected == msaaInfo.fProtected);
    SkASSERT(msaaInfo.fSampleCount > 1);
    this->registerWithCacheWrapped(GrWrapCacheable::kNo);
}

// We're virtually derived from GrSurface (via GrRenderTarget) so its
// constructor must be explicitly called.
GrD3DRenderTarget::GrD3DRenderTarget(GrD3DGpu* gpu,
                                     SkISize dimensions,
                                     const GrD3DTextureResourceInfo& info,
                                     sk_sp<GrD3DResourceState> state,
                                     const GrD3DTextureResourceInfo& msaaInfo,
                                     sk_sp<GrD3DResourceState> msaaState,
                                     const GrD3DDescriptorHeap::CPUHandle& colorRenderTargetView,
                                     const GrD3DDescriptorHeap::CPUHandle& resolveRenderTargetView)
        : GrSurface(gpu, dimensions, info.fProtected)
        , GrD3DTextureResource(info, std::move(state))
        // for the moment we only support 1:1 color to stencil
        , GrRenderTarget(gpu, dimensions, msaaInfo.fSampleCount, info.fProtected)
        , fMSAATextureResource(new GrD3DTextureResource(msaaInfo, std::move(msaaState)))
        , fColorRenderTargetView(colorRenderTargetView)
        , fResolveRenderTargetView(resolveRenderTargetView) {
    SkASSERT(info.fProtected == msaaInfo.fProtected);
    SkASSERT(msaaInfo.fSampleCount > 1);
}

// We're virtually derived from GrSurface (via GrRenderTarget) so its
// constructor must be explicitly called.
GrD3DRenderTarget::GrD3DRenderTarget(GrD3DGpu* gpu,
                                     SkISize dimensions,
                                     const GrD3DTextureResourceInfo& info,
                                     sk_sp<GrD3DResourceState> state,
                                     const GrD3DDescriptorHeap::CPUHandle& renderTargetView,
                                     Wrapped)
        : GrSurface(gpu, dimensions, info.fProtected)
        , GrD3DTextureResource(info, std::move(state))
        , GrRenderTarget(gpu, dimensions, info.fSampleCount, info.fProtected)
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
                                     const GrD3DDescriptorHeap::CPUHandle& renderTargetView)
        : GrSurface(gpu, dimensions, info.fProtected)
        , GrD3DTextureResource(info, std::move(state))
        , GrRenderTarget(gpu, dimensions, info.fSampleCount, info.fProtected)
        , fMSAATextureResource(nullptr)
        , fColorRenderTargetView(renderTargetView) {}

sk_sp<GrD3DRenderTarget> GrD3DRenderTarget::MakeWrappedRenderTarget(
            GrD3DGpu* gpu, SkISize dimensions, int sampleCnt, const GrD3DTextureResourceInfo& info,
            sk_sp<GrD3DResourceState> state) {
    SkASSERT(info.fResource.get());
    SkASSERT(info.fLevelCount == 1);
    SkASSERT(sampleCnt >= 1 && info.fSampleCount >= 1);

    int wrappedTextureSampleCnt = static_cast<int>(info.fSampleCount);
    if (sampleCnt != wrappedTextureSampleCnt && wrappedTextureSampleCnt != 1) {
        return nullptr;
    }

    GrD3DDescriptorHeap::CPUHandle renderTargetView =
            gpu->resourceProvider().createRenderTargetView(info.fResource.get());

    // create msaa surface if necessary
    GrD3DRenderTarget* d3dRT;
    if (sampleCnt != wrappedTextureSampleCnt) {
        GrD3DTextureResourceInfo msInfo;
        sk_sp<GrD3DResourceState> msState;
        // for wrapped MSAA surface we assume clear to white
        SkColor4f clearColor = { 1, 1, 1, 1 };
        std::tie(msInfo, msState) =
                GrD3DTextureResource::CreateMSAA(gpu, dimensions, sampleCnt, info, clearColor);

        GrD3DDescriptorHeap::CPUHandle msaaRenderTargetView =
                gpu->resourceProvider().createRenderTargetView(msInfo.fResource.get());

        d3dRT = new GrD3DRenderTarget(gpu, dimensions, info, std::move(state), msInfo,
                                      std::move(msState), msaaRenderTargetView, renderTargetView,
                                      kWrapped);
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

const GrD3DTextureResource* GrD3DRenderTarget::msaaTextureResource() const {
    if (this->numSamples() == 1) {
        SkASSERT(!fMSAATextureResource);
        return nullptr;
    }
    if (fMSAATextureResource) {
        return fMSAATextureResource.get();
    }
    SkASSERT(!fMSAATextureResource);
    return this;
}

GrD3DTextureResource* GrD3DRenderTarget::msaaTextureResource() {
    auto* constThis = const_cast<const GrD3DRenderTarget*>(this);
    return const_cast<GrD3DTextureResource*>(constThis->msaaTextureResource());
}

void GrD3DRenderTarget::releaseInternalObjects() {
    GrD3DGpu* gpu = this->getD3DGpu();

    if (fMSAATextureResource) {
        fMSAATextureResource->releaseResource(gpu);
        fMSAATextureResource.reset();
        gpu->resourceProvider().recycleRenderTargetView(fResolveRenderTargetView);
    }

    gpu->resourceProvider().recycleRenderTargetView(fColorRenderTargetView);
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
    return GrBackendRenderTarget(this->width(), this->height(), fInfo, this->grD3DResourceState());
}

GrD3DGpu* GrD3DRenderTarget::getD3DGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrD3DGpu*>(this->getGpu());
}

DXGI_FORMAT GrD3DRenderTarget::stencilDxgiFormat() const {
    if (auto stencil = this->getStencilAttachment()) {
        auto d3dStencil = static_cast<GrD3DAttachment*>(stencil);
        return d3dStencil->dxgiFormat();
    }
    return DXGI_FORMAT_UNKNOWN;
}

void GrD3DRenderTarget::genKey(GrProcessorKeyBuilder* b) const {
    b->add32(this->dxgiFormat());
    b->add32(this->numSamples());
    b->add32(this->stencilDxgiFormat());
#ifdef SK_DEBUG
    if (const GrAttachment* stencil = this->getStencilAttachment()) {
        SkASSERT(stencil->numSamples() == this->numSamples());
    }
#endif
    b->add32(this->sampleQualityPattern());
}
