/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrRenderTarget.h"

#include "GrContext.h"
#include "GrDrawContext.h"
#include "GrDrawTarget.h"
#include "GrGpu.h"
#include "GrRenderTargetPriv.h"
#include "GrStencilAttachment.h"

GrRenderTarget::~GrRenderTarget() {
    if (fLastDrawTarget) {
        fLastDrawTarget->clearRT();
    }
    SkSafeUnref(fLastDrawTarget);
}

void GrRenderTarget::discard() {
    // go through context so that all necessary flushing occurs
    GrContext* context = this->getContext();
    if (!context) {
        return;
    }

    SkAutoTUnref<GrDrawContext> drawContext(context->drawContext(this));
    if (!drawContext) {
        return;
    }

    drawContext->discard();
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

    // The contents of this renderTarget are gone/invalid. It isn't useful to point back
    // the creating drawTarget.
    this->setLastDrawTarget(nullptr);

    INHERITED::onAbandon();
}

void GrRenderTarget::setLastDrawTarget(GrDrawTarget* dt) {
    if (fLastDrawTarget) {
        // The non-MDB world never closes so we can't check this condition
#ifdef ENABLE_MDB
        SkASSERT(fLastDrawTarget->isClosed());
#endif
        fLastDrawTarget->clearRT();
    }

    SkRefCnt_SafeAssign(fLastDrawTarget, dt);
}

///////////////////////////////////////////////////////////////////////////////

bool GrRenderTargetPriv::attachStencilAttachment(GrStencilAttachment* stencil) {
    if (!stencil && !fRenderTarget->fStencilAttachment) {
        // No need to do any work since we currently don't have a stencil attachment and
        // we're not acctually adding one.
        return true;
    }
    fRenderTarget->fStencilAttachment = stencil;
    if (!fRenderTarget->completeStencilAttachment()) {
        SkSafeSetNull(fRenderTarget->fStencilAttachment);
        return false;
    }
    return true;
}

const GrGpu::MultisampleSpecs&
GrRenderTargetPriv::getMultisampleSpecs(const GrStencilSettings& stencil) const {
    return fRenderTarget->getGpu()->getMultisampleSpecs(fRenderTarget, stencil);
}
