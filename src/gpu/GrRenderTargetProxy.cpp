/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrRenderTargetProxy.h"

#include "GrCaps.h"
#include "GrGpuResourcePriv.h"
#include "GrRenderTargetOpList.h"
#include "GrRenderTargetPriv.h"
#include "GrResourceProvider.h"
#include "GrTextureRenderTargetProxy.h"
#include "SkMathPriv.h"

// Deferred version
// TODO: we can probably munge the 'desc' in both the wrapped and deferred
// cases to make the sampleConfig/numSamples stuff more rational.
GrRenderTargetProxy::GrRenderTargetProxy(const GrCaps& caps, const GrSurfaceDesc& desc,
                                         SkBackingFit fit, SkBudgeted budgeted, uint32_t flags)
        : INHERITED(desc, fit, budgeted, flags)
        , fSampleCnt(desc.fSampleCnt)
        , fNeedsStencil(false)
        , fRenderTargetFlags(GrRenderTargetFlags::kNone) {
    // Since we know the newly created render target will be internal, we are able to precompute
    // what the flags will ultimately end up being.
    if (caps.usesMixedSamples() && fSampleCnt > 1) {
        fRenderTargetFlags |= GrRenderTargetFlags::kMixedSampled;
    }
    if (caps.maxWindowRectangles() > 0) {
        fRenderTargetFlags |= GrRenderTargetFlags::kWindowRectsSupport;
    }
}

// Lazy-callback version
GrRenderTargetProxy::GrRenderTargetProxy(LazyInstantiateCallback&& callback,
                                         LazyInstantiationType lazyType,
                                         const GrSurfaceDesc& desc,
                                         SkBackingFit fit, SkBudgeted budgeted,
                                         uint32_t flags,
                                         GrRenderTargetFlags renderTargetFlags)
        : INHERITED(std::move(callback), lazyType, desc, fit, budgeted, flags)
        , fSampleCnt(desc.fSampleCnt)
        , fNeedsStencil(false)
        , fRenderTargetFlags(renderTargetFlags) {
    SkASSERT(SkToBool(kRenderTarget_GrSurfaceFlag & desc.fFlags));
}

// Wrapped version
GrRenderTargetProxy::GrRenderTargetProxy(sk_sp<GrSurface> surf, GrSurfaceOrigin origin)
        : INHERITED(std::move(surf), origin, SkBackingFit::kExact)
        , fSampleCnt(fTarget->asRenderTarget()->numStencilSamples())
        , fNeedsStencil(false)
        , fRenderTargetFlags(fTarget->asRenderTarget()->renderTargetPriv().flags()) {
}

int GrRenderTargetProxy::maxWindowRectangles(const GrCaps& caps) const {
    return (fRenderTargetFlags & GrRenderTargetFlags::kWindowRectsSupport)
                   ? caps.maxWindowRectangles()
                   : 0;
}

bool GrRenderTargetProxy::instantiate(GrResourceProvider* resourceProvider) {
    if (LazyState::kNot != this->lazyInstantiationState()) {
        return false;
    }
    static constexpr GrSurfaceFlags kFlags = kRenderTarget_GrSurfaceFlag;

    if (!this->instantiateImpl(resourceProvider, fSampleCnt, fNeedsStencil, kFlags,
                               GrMipMapped::kNo, nullptr)) {
        return false;
    }
    SkASSERT(fTarget->asRenderTarget());
    SkASSERT(!fTarget->asTexture());
    // Check that our a priori computation matched the ultimate reality
    SkASSERT(fRenderTargetFlags == fTarget->asRenderTarget()->renderTargetPriv().flags());

    return true;
}

sk_sp<GrSurface> GrRenderTargetProxy::createSurface(GrResourceProvider* resourceProvider) const {
    static constexpr GrSurfaceFlags kFlags = kRenderTarget_GrSurfaceFlag;

    sk_sp<GrSurface> surface = this->createSurfaceImpl(resourceProvider, fSampleCnt, fNeedsStencil,
                                                       kFlags, GrMipMapped::kNo);
    if (!surface) {
        return nullptr;
    }
    SkASSERT(surface->asRenderTarget());
    SkASSERT(!surface->asTexture());
    // Check that our a priori computation matched the ultimate reality
    SkASSERT(fRenderTargetFlags == surface->asRenderTarget()->renderTargetPriv().flags());

    return surface;
}

size_t GrRenderTargetProxy::onUninstantiatedGpuMemorySize() const {
    int colorSamplesPerPixel = this->numColorSamples();
    if (colorSamplesPerPixel > 1) {
        // Add one for the resolve buffer.
        ++colorSamplesPerPixel;
    }

    // TODO: do we have enough information to improve this worst case estimate?
    return GrSurface::ComputeSize(this->config(), this->width(), this->height(),
                                  colorSamplesPerPixel, GrMipMapped::kNo, !this->priv().isExact());
}

bool GrRenderTargetProxy::refsWrappedObjects() const {
    if (!fTarget) {
        return false;
    }

    return fTarget->resourcePriv().refsWrappedObjects();
}

#ifdef SK_DEBUG
void GrRenderTargetProxy::validateLazySurface(const GrSurface* surface) {
    SkASSERT(!surface->asTexture());

    // Anything that is checked here should be duplicated in GrTextureRenderTargetProxy's version
    SkASSERT(surface->asRenderTarget());
    SkASSERT(surface->asRenderTarget()->numStencilSamples() == this->numStencilSamples());
}
#endif
