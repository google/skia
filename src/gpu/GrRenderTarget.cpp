/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "src/gpu/GrRenderTarget.h"

#include "src/core/SkRectPriv.h"
#include "src/gpu/GrAttachment.h"
#include "src/gpu/GrBackendUtils.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrSamplePatternDictionary.h"
#include "src/gpu/GrStencilSettings.h"
#include "src/gpu/GrSurfaceDrawContext.h"

GrRenderTarget::GrRenderTarget(GrGpu* gpu,
                               const SkISize& dimensions,
                               int sampleCount,
                               GrProtected isProtected,
                               GrAttachment* stencil)
        : INHERITED(gpu, dimensions, isProtected)
        , fStencilAttachment(stencil)
        , fSampleCnt(sampleCount)
        , fSamplePatternKey(GrSamplePatternDictionary::kInvalidSamplePatternKey) {}

GrRenderTarget::~GrRenderTarget() = default;

void GrRenderTarget::onRelease() {
    fStencilAttachment = nullptr;

    INHERITED::onRelease();
}

void GrRenderTarget::onAbandon() {
    fStencilAttachment = nullptr;

    INHERITED::onAbandon();
}

void GrRenderTarget::attachStencilAttachment(sk_sp<GrAttachment> stencil) {
#ifdef SK_DEBUG
    if (fSampleCnt == 1) {
        // TODO: We don't expect a mixed sampled render target to ever change its stencil buffer
        // right now. But if it does swap in a stencil buffer with a different number of samples,
        // and if we have a valid fSamplePatternKey, we will need to invalidate fSamplePatternKey
        // here and add tests to make sure we it properly.
        SkASSERT(GrSamplePatternDictionary::kInvalidSamplePatternKey == fSamplePatternKey);
    } else {
        // Render targets with >1 color sample should never use mixed samples. (This would lead to
        // different sample patterns, depending on stencil state.)
        SkASSERT(!stencil || stencil->numSamples() == fSampleCnt);
    }
#endif

    if (!stencil && !fStencilAttachment) {
        // No need to do any work since we currently don't have a stencil attachment and
        // we're not actually adding one.
        return;
    }

    fStencilAttachment = std::move(stencil);
    if (!this->completeStencilAttachment()) {
        fStencilAttachment = nullptr;
    }
}

int GrRenderTarget::numStencilBits() const {
    SkASSERT(this->getStencilAttachment());
    return GrBackendFormatStencilBits(this->getStencilAttachment()->backendFormat());
}

int GrRenderTarget::getSamplePatternKey() {
#ifdef SK_DEBUG
    if (fSampleCnt <= 1) {
        // If the color buffer is not multisampled, the sample pattern better come from the stencil
        // buffer (mixed samples).
        SkASSERT(fStencilAttachment && fStencilAttachment->numSamples() > 1);
    } else {
        // The color sample count and stencil count cannot both be unequal and both greater than
        // one. If this were the case, there would be more than one sample pattern associated with
        // the render target.
        SkASSERT(!fStencilAttachment || fStencilAttachment->numSamples() == fSampleCnt);
    }
#endif
    if (GrSamplePatternDictionary::kInvalidSamplePatternKey == fSamplePatternKey) {
        fSamplePatternKey = this->getGpu()->findOrAssignSamplePatternKey(this);
    }
    SkASSERT(fSamplePatternKey != GrSamplePatternDictionary::kInvalidSamplePatternKey);
    return fSamplePatternKey;
}

const SkTArray<SkPoint>& GrRenderTarget::getSampleLocations() {
    int samplePatternKey = this->getSamplePatternKey();
    return this->getGpu()->retrieveSampleLocations(samplePatternKey);
}
