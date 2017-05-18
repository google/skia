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
        , fRenderTargetFlags(GrRenderTarget::Flags::kNone) {
    // Since we know the newly created render target will be internal, we are able to precompute
    // what the flags will ultimately end up being.
    if (caps.usesMixedSamples() && fSampleCnt > 0) {
        fRenderTargetFlags |= GrRenderTarget::Flags::kMixedSampled;
    }
    if (caps.maxWindowRectangles() > 0) {
        fRenderTargetFlags |= GrRenderTarget::Flags::kWindowRectsSupport;
    }
}

// Wrapped version
GrRenderTargetProxy::GrRenderTargetProxy(sk_sp<GrSurface> surf)
        : INHERITED(std::move(surf), SkBackingFit::kExact)
        , fSampleCnt(fTarget->asRenderTarget()->numStencilSamples())
        , fRenderTargetFlags(fTarget->asRenderTarget()->renderTargetPriv().flags()) {}

int GrRenderTargetProxy::maxWindowRectangles(const GrCaps& caps) const {
    return (fRenderTargetFlags & GrRenderTarget::Flags::kWindowRectsSupport)
                   ? caps.maxWindowRectangles()
                   : 0;
}

GrSurface* GrRenderTargetProxy::instantiate(GrResourceProvider* resourceProvider) {
    static constexpr GrSurfaceFlags kFlags = kRenderTarget_GrSurfaceFlag;

    GrSurface* surf = this->instantiateImpl(resourceProvider, fSampleCnt, kFlags,
                                            /* isMipped = */ false);
    if (!surf) {
        return nullptr;
    }
    SkASSERT(surf->asRenderTarget());
    // Check that our a priori computation matched the ultimate reality
    SkASSERT(fRenderTargetFlags == surf->asRenderTarget()->renderTargetPriv().flags());

    return surf;
}

int GrRenderTargetProxy::worstCaseWidth() const {
    if (fTarget) {
        return fTarget->width();
    }

    if (SkBackingFit::kExact == fFit) {
        return fWidth;
    }
    return SkTMax(GrResourceProvider::kMinScratchTextureSize, GrNextPow2(fWidth));
}

int GrRenderTargetProxy::worstCaseHeight() const {
    if (fTarget) {
        return fTarget->height();
    }

    if (SkBackingFit::kExact == fFit) {
        return fHeight;
    }
    return SkTMax(GrResourceProvider::kMinScratchTextureSize, GrNextPow2(fHeight));
}

size_t GrRenderTargetProxy::onUninstantiatedGpuMemorySize() const {
    int colorSamplesPerPixel = this->numColorSamples() + 1;
    // TODO: do we have enough information to improve this worst case estimate?
    return GrSurface::ComputeSize(fConfig, fWidth, fHeight, colorSamplesPerPixel, false,
                                  SkBackingFit::kApprox == fFit);
}

bool GrRenderTargetProxy::refsWrappedObjects() const {
    if (!fTarget) {
        return false;
    }

    return fTarget->resourcePriv().refsWrappedObjects();
}
