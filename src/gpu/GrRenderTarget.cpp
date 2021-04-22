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
#include "src/gpu/GrStencilSettings.h"
#include "src/gpu/GrSurfaceDrawContext.h"

GrRenderTarget::GrRenderTarget(GrGpu* gpu,
                               const SkISize& dimensions,
                               int sampleCount,
                               GrProtected isProtected,
                               GrAttachment* stencil)
        : INHERITED(gpu, dimensions, isProtected)
        , fStencilAttachment(stencil)
        , fSampleCnt(sampleCount) {
}

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

