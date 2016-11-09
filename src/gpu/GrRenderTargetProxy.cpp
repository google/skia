/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrRenderTargetProxy.h"

#include "GrCaps.h"
#include "GrRenderTargetOpList.h"
#include "GrRenderTargetPriv.h"
#include "GrTextureProvider.h"
#include "GrTextureRenderTargetProxy.h"

// Deferred version
// TODO: we can probably munge the 'desc' in both the wrapped and deferred
// cases to make the sampleConfig/numSamples stuff more rational.
GrRenderTargetProxy::GrRenderTargetProxy(const GrCaps& caps, const GrSurfaceDesc& desc,
                                         SkBackingFit fit, SkBudgeted budgeted)
    : INHERITED(desc, fit, budgeted)
    , fFlags(GrRenderTarget::Flags::kNone) {
    // Since we know the newly created render target will be internal, we are able to precompute
    // what the flags will ultimately end up being.
    if (caps.usesMixedSamples() && fDesc.fSampleCnt > 0) {
        fFlags |= GrRenderTarget::Flags::kMixedSampled;
    }
    if (caps.maxWindowRectangles() > 0) {
        fFlags |= GrRenderTarget::Flags::kWindowRectsSupport;
    }
}

// Wrapped version
GrRenderTargetProxy::GrRenderTargetProxy(sk_sp<GrSurface> surf)
    : INHERITED(std::move(surf), SkBackingFit::kExact)
    , fFlags(fTarget->asRenderTarget()->renderTargetPriv().flags()) {
}

int GrRenderTargetProxy::maxWindowRectangles(const GrCaps& caps) const {
    return (fFlags & GrRenderTarget::Flags::kWindowRectsSupport) ? caps.maxWindowRectangles() : 0;
}

GrRenderTarget* GrRenderTargetProxy::instantiate(GrTextureProvider* texProvider) {
    SkASSERT(fDesc.fFlags & GrSurfaceFlags::kRenderTarget_GrSurfaceFlag);

    GrSurface* surf = INHERITED::instantiate(texProvider);
    if (!surf || !surf->asRenderTarget()) {
        return nullptr;
    }

    // Check that our a priori computation matched the ultimate reality
    SkASSERT(fFlags == surf->asRenderTarget()->renderTargetPriv().flags());

    return surf->asRenderTarget();
}


#ifdef SK_DEBUG
void GrRenderTargetProxy::validate(GrContext* context) const {
    if (fTarget) {
        SkASSERT(fTarget->getContext() == context);
    }

    INHERITED::validate();
}
#endif

size_t GrRenderTargetProxy::onGpuMemorySize() const {
    if (fTarget) {
        return fTarget->gpuMemorySize();
    }

    // TODO: do we have enough information to improve this worst case estimate?
    return GrSurface::ComputeSize(fDesc, fDesc.fSampleCnt+1, false);
}

