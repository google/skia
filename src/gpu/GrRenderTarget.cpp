/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "include/gpu/GrRenderTarget.h"

#include "include/gpu/GrContext.h"
#include "src/core/SkRectPriv.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrRenderTargetOpList.h"
#include "src/gpu/GrRenderTargetPriv.h"
#include "src/gpu/GrSamplePatternDictionary.h"
#include "src/gpu/GrStencilAttachment.h"
#include "src/gpu/GrStencilSettings.h"

GrRenderTarget::GrRenderTarget(GrGpu* gpu, const GrSurfaceDesc& desc,
                               GrStencilAttachment* stencil)
        : INHERITED(gpu, desc)
        , fSampleCnt(desc.fSampleCnt)
        , fSamplePatternKey(GrSamplePatternDictionary::kInvalidSamplePatternKey)
        , fStencilAttachment(stencil) {
    SkASSERT(desc.fFlags & kRenderTarget_GrSurfaceFlag);
    SkASSERT(!this->hasMixedSamples() || fSampleCnt > 1);
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

void GrRenderTarget::overrideResolveRect(const SkIRect rect) {
    fResolveRect = rect;
    if (fResolveRect.isEmpty()) {
        fResolveRect = SkRectPriv::MakeILargestInverted();
    } else {
        if (!fResolveRect.intersect(0, 0, this->width(), this->height())) {
            fResolveRect = SkRectPriv::MakeILargestInverted();
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
    SkASSERT(fRenderTarget->fSampleCnt > 1);
    if (GrSamplePatternDictionary::kInvalidSamplePatternKey == fRenderTarget->fSamplePatternKey) {
        fRenderTarget->fSamplePatternKey =
                fRenderTarget->getGpu()->findOrAssignSamplePatternKey(fRenderTarget);
    }
    SkASSERT(GrSamplePatternDictionary::kInvalidSamplePatternKey
                     != fRenderTarget->fSamplePatternKey);
    return fRenderTarget->fSamplePatternKey;
}
