/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrRenderTargetProxy.h"

#include "GrCaps.h"
#include "GrDrawTarget.h"
#include "GrGpuResourcePriv.h"

// Deferred version
// TODO: we can probably munge the 'desc' in both the wrapped and deferred
// cases to make the sampleConfig/numSamples stuff more rational.
GrRenderTargetProxy::GrRenderTargetProxy(const GrCaps& caps, const GrSurfaceDesc& desc,
                                         SkBackingFit fit, SkBudgeted budgeted)
    : INHERITED(desc, fit, budgeted)
    , fFlags(GrRenderTargetPriv::Flags::kNone)
    , fLastDrawTarget(nullptr) {
    // Since we know the newly created render target will be internal, we are able to precompute
    // what the flags will ultimately end up being.
    if (caps.usesMixedSamples() && fDesc.fSampleCnt > 0) {
        fFlags |= GrRenderTargetPriv::Flags::kMixedSampled;
    }
    if (caps.maxWindowRectangles() > 0) {
        fFlags |= GrRenderTargetPriv::Flags::kWindowRectsSupport;
    }
}

// Wrapped version
GrRenderTargetProxy::GrRenderTargetProxy(const GrCaps& caps, sk_sp<GrRenderTarget> rt)
    : INHERITED(std::move(rt), SkBackingFit::kExact,
                rt->resourcePriv().isBudgeted(), rt->uniqueID())
    , fFlags(fTarget->asRenderTarget()->renderTargetPriv().flags())
    , fLastDrawTarget(nullptr) {
}

GrRenderTargetProxy::~GrRenderTargetProxy() {
    if (fLastDrawTarget) {
        fLastDrawTarget->clearRTP();
    }
    SkSafeUnref(fLastDrawTarget);
}

GrRenderTarget* GrRenderTargetProxy::instantiate(GrTextureProvider* texProvider) {
    if (fTarget) {
        return fTarget->asRenderTarget();
    }

    // TODO: it would be nice to not have to copy the desc here
    GrSurfaceDesc desc = fDesc;
    desc.fFlags |= GrSurfaceFlags::kRenderTarget_GrSurfaceFlag;

    if (SkBackingFit::kApprox == fFit) {
        fTarget = texProvider->createApproxTexture(desc);
    } else {
        fTarget = texProvider->createTexture(desc, fBudgeted);
    }
    if (!fTarget) {
        return nullptr;
    }

    // Check that our a priori computation matched the ultimate reality
    SkASSERT(fFlags == fTarget->asRenderTarget()->renderTargetPriv().flags());

    return fTarget->asRenderTarget();
}

void GrRenderTargetProxy::setLastDrawTarget(GrDrawTarget* dt) {
    if (fLastDrawTarget) {
        // The non-MDB world never closes so we can't check this condition
#ifdef ENABLE_MDB
        SkASSERT(fLastDrawTarget->isClosed());
#endif
        fLastDrawTarget->clearRTP();
    }

    SkRefCnt_SafeAssign(fLastDrawTarget, dt);
}

#ifdef SK_DEBUG
void GrRenderTargetProxy::validate(GrContext* context) const {
    if (fTarget) {
        SkASSERT(fTarget->getContext() == context);
    }
}
#endif

sk_sp<GrRenderTargetProxy> GrRenderTargetProxy::Make(const GrCaps& caps,
                                                     const GrSurfaceDesc& desc,
                                                     SkBackingFit fit,
                                                     SkBudgeted budgeted) {
    return sk_sp<GrRenderTargetProxy>(new GrRenderTargetProxy(caps, desc, fit, budgeted));
}

sk_sp<GrRenderTargetProxy> GrRenderTargetProxy::Make(const GrCaps& caps, sk_sp<GrRenderTarget> rt) {
    return sk_sp<GrRenderTargetProxy>(new GrRenderTargetProxy(caps, rt));
}

