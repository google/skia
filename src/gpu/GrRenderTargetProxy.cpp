/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrRenderTargetProxy.h"

#include "GrDrawTarget.h"
#include "GrGpuResourcePriv.h"
#include "GrRenderTargetPriv.h"

GrRenderTargetProxy::GrRenderTargetProxy(sk_sp<GrRenderTarget> rt)
    : INHERITED(rt->desc(), SkBackingFit::kExact, rt->resourcePriv().isBudgeted())
    , fTarget(std::move(rt))
    , fSampleConfig(fTarget->renderTargetPriv().sampleConfig())
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
    SkASSERT(fSampleConfig == fTarget->renderTargetPriv().sampleConfig());

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

sk_sp<GrRenderTargetProxy> GrRenderTargetProxy::Make(sk_sp<GrRenderTarget> rt) {
    return sk_sp<GrRenderTargetProxy>(new GrRenderTargetProxy(rt));
}

