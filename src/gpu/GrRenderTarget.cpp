/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrRenderTarget.h"

#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrRenderTargetContext.h"
#include "GrGpu.h"
#include "GrRenderTargetOpList.h"
#include "GrRenderTargetPriv.h"
#include "GrStencilAttachment.h"
#include "GrStencilSettings.h"

GrRenderTarget::GrRenderTarget(GrGpu* gpu, const GrSurfaceDesc& desc,
                               GrRenderTargetFlags flags,
                               GrStencilAttachment* stencil)
        : INHERITED(gpu, desc)
        , fSampleCnt(desc.fSampleCnt)
        , fStencilAttachment(stencil)
        , fMultisampleSpecsID(0)
        , fFlags(flags) {
    SkASSERT(desc.fFlags & kRenderTarget_GrSurfaceFlag);
    SkASSERT(!(fFlags & GrRenderTargetFlags::kMixedSampled) || fSampleCnt > 0);
    SkASSERT(!(fFlags & GrRenderTargetFlags::kWindowRectsSupport) ||
             gpu->caps()->maxWindowRectangles() > 0);
    fResolveRect.setLargestInverted();
}

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
        fResolveRect.setLargestInverted();
    } else {
        if (!fResolveRect.intersect(0, 0, this->width(), this->height())) {
            fResolveRect.setLargestInverted();
        }
    }
}

void GrRenderTarget::onRelease() {
    SkSafeSetNull(fStencilAttachment);

    INHERITED::onRelease();
}

void GrRenderTarget::onAbandon() {
    SkSafeSetNull(fStencilAttachment);

    INHERITED::onAbandon();
}

///////////////////////////////////////////////////////////////////////////////

bool GrRenderTargetPriv::attachStencilAttachment(GrStencilAttachment* stencil) {
    if (!stencil && !fRenderTarget->fStencilAttachment) {
        // No need to do any work since we currently don't have a stencil attachment and
        // we're not actually adding one.
        return true;
    }
    fRenderTarget->fStencilAttachment = stencil;
    if (!fRenderTarget->completeStencilAttachment()) {
        SkSafeSetNull(fRenderTarget->fStencilAttachment);
        return false;
    }
    return true;
}

int GrRenderTargetPriv::numStencilBits() const {
    SkASSERT(this->getStencilAttachment());
    return this->getStencilAttachment()->bits();
}

const GrGpu::MultisampleSpecs&
GrRenderTargetPriv::getMultisampleSpecs(const GrPipeline& pipeline) const {
    SkASSERT(fRenderTarget == pipeline.getRenderTarget()); // TODO: remove RT from pipeline.
    GrGpu* gpu = fRenderTarget->getGpu();
    if (auto id = fRenderTarget->fMultisampleSpecsID) {
        SkASSERT(gpu->queryMultisampleSpecs(pipeline).fUniqueID == id);
        return gpu->getMultisampleSpecs(id);
    }
    const GrGpu::MultisampleSpecs& specs = gpu->queryMultisampleSpecs(pipeline);
    fRenderTarget->fMultisampleSpecsID = specs.fUniqueID;
    return specs;
}

