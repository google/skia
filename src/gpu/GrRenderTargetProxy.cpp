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
    , fTarget(nullptr)
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
    : INHERITED(rt->desc(), SkBackingFit::kExact,
                rt->resourcePriv().isBudgeted(), rt->uniqueID())
    , fTarget(std::move(rt))
    , fFlags(fTarget->renderTargetPriv().flags())
    , fLastDrawTarget(nullptr) {
}

GrRenderTargetProxy::~GrRenderTargetProxy() {
    if (fLastDrawTarget) {
        fLastDrawTarget->clearRT();
    }
    SkSafeUnref(fLastDrawTarget);
}

GrRenderTarget* GrRenderTargetProxy::instantiate(GrTextureProvider* texProvider) {
    if (fTarget) {
        return fTarget.get();
    }

    // TODO: it would be nice to not have to copy the desc here
    GrSurfaceDesc desc = fDesc;
    desc.fFlags |= GrSurfaceFlags::kRenderTarget_GrSurfaceFlag;

    sk_sp<GrTexture> tex;
    if (SkBackingFit::kApprox == fFit) {
        tex.reset(texProvider->createApproxTexture(desc));
    } else {
        tex.reset(texProvider->createTexture(desc, fBudgeted));
    }
    if (!tex || !tex->asRenderTarget()) {
        return nullptr;
    }

    fTarget = sk_ref_sp(tex->asRenderTarget());

    // Check that our a priori computation matched the ultimate reality
    SkASSERT(fFlags == fTarget->renderTargetPriv().flags());

    return fTarget.get();
}

void GrRenderTargetProxy::setLastDrawTarget(GrDrawTarget* dt) {
    if (fLastDrawTarget) {
        // The non-MDB world never closes so we can't check this condition
#ifdef ENABLE_MDB
        SkASSERT(fLastDrawTarget->isClosed());
#endif
        fLastDrawTarget->clearRT();
    }

    SkRefCnt_SafeAssign(fLastDrawTarget, dt);
}

sk_sp<GrRenderTargetProxy> GrRenderTargetProxy::Make(const GrCaps& caps,
                                                     const GrSurfaceDesc& desc,
                                                     SkBackingFit fit,
                                                     SkBudgeted budgeted) {
    return sk_sp<GrRenderTargetProxy>(new GrRenderTargetProxy(caps, desc, fit, budgeted));
}

sk_sp<GrRenderTargetProxy> GrRenderTargetProxy::Make(const GrCaps& caps, sk_sp<GrRenderTarget> rt) {
    return sk_sp<GrRenderTargetProxy>(new GrRenderTargetProxy(caps, rt));
}

