/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "src/gpu/GrRenderTarget.h"

#include "include/gpu/GrContext.h"
#include "src/core/SkRectPriv.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrRenderTargetPriv.h"
#include "src/gpu/GrSamplePatternDictionary.h"
#include "src/gpu/GrStencilAttachment.h"
#include "src/gpu/GrStencilSettings.h"

GrRenderTarget::GrRenderTarget(GrGpu* gpu, const SkISize& size, GrPixelConfig config,
                               int sampleCount, GrProtected isProtected,
                               GrStencilAttachment* stencil)
        : INHERITED(gpu, size, config, isProtected)
        , fSampleCnt(sampleCount)
        , fSamplePatternKey(GrSamplePatternDictionary::kInvalidSamplePatternKey)
        , fStencilAttachment(stencil) {
    fResolveRect = SkRectPriv::MakeILargestInverted();
}

GrRenderTarget::~GrRenderTarget() = default;

void GrRenderTarget::flagAsNeedingResolve(const SkIRect* rect) {
    if (kCanResolve_ResolveType == getResolveType()) {
        if (rect) {
            fResolveRect.join(*rect);
            if (!fResolveRect.intersect(0, 0, this->width(), this->height())) {
                fResolveRect.setEmpty();
            }
        } else {
            fResolveRect.setLTRB(0, 0, this->width(), this->height());
        }
    }
}

void GrRenderTarget::flagAsResolved() {
    fResolveRect = SkRectPriv::MakeILargestInverted();
}

void GrRenderTarget::onRelease() {
    fStencilAttachment = nullptr;

    INHERITED::onRelease();
}

void GrRenderTarget::onAbandon() {
    fStencilAttachment = nullptr;

    INHERITED::onAbandon();
}

///////////////////////////////////////////////////////////////////////////////

void GrRenderTargetPriv::attachStencilAttachment(sk_sp<GrStencilAttachment> stencil) {
#ifdef SK_DEBUG
    if (1 == fRenderTarget->fSampleCnt) {
        // TODO: We don't expect a mixed sampled render target to ever change its stencil buffer
        // right now. But if it does swap in a stencil buffer with a different number of samples,
        // and if we have a valid fSamplePatternKey, we will need to invalidate fSamplePatternKey
        // here and add tests to make sure we it properly.
        SkASSERT(GrSamplePatternDictionary::kInvalidSamplePatternKey ==
                 fRenderTarget->fSamplePatternKey);
    } else {
        // Render targets with >1 color sample should never use mixed samples. (This would lead to
        // different sample patterns, depending on stencil state.)
        SkASSERT(!stencil || stencil->numSamples() == fRenderTarget->fSampleCnt);
    }
#endif

    if (!stencil && !fRenderTarget->fStencilAttachment) {
        // No need to do any work since we currently don't have a stencil attachment and
        // we're not actually adding one.
        return;
    }

    fRenderTarget->fStencilAttachment = std::move(stencil);
    if (!fRenderTarget->completeStencilAttachment()) {
        fRenderTarget->fStencilAttachment = nullptr;
    }
}

int GrRenderTargetPriv::numStencilBits() const {
    SkASSERT(this->getStencilAttachment());
    return this->getStencilAttachment()->bits();
}

int GrRenderTargetPriv::getSamplePatternKey() const {
#ifdef SK_DEBUG
    GrStencilAttachment* stencil = fRenderTarget->fStencilAttachment.get();
    if (fRenderTarget->fSampleCnt <= 1) {
        // If the color buffer is not multisampled, the sample pattern better come from the stencil
        // buffer (mixed samples).
        SkASSERT(stencil && stencil->numSamples() > 1);
    } else {
        // The color sample count and stencil count cannot both be unequal and both greater than
        // one. If this were the case, there would be more than one sample pattern associated with
        // the render target.
        SkASSERT(!stencil || stencil->numSamples() == fRenderTarget->fSampleCnt);
    }
#endif
    if (GrSamplePatternDictionary::kInvalidSamplePatternKey == fRenderTarget->fSamplePatternKey) {
        fRenderTarget->fSamplePatternKey =
                fRenderTarget->getGpu()->findOrAssignSamplePatternKey(fRenderTarget);
    }
    SkASSERT(GrSamplePatternDictionary::kInvalidSamplePatternKey
                     != fRenderTarget->fSamplePatternKey);
    return fRenderTarget->fSamplePatternKey;
}
