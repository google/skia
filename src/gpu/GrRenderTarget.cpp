
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrRenderTarget.h"

#include "GrContext.h"
#include "GrGpu.h"
#include "GrRenderTargetPriv.h"
#include "GrStencilBuffer.h"

void GrRenderTarget::discard() {
    // go through context so that all necessary flushing occurs
    GrContext* context = this->getContext();
    if (NULL == context) {
        return;
    }
    context->discardRenderTarget(this);
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
    this->renderTargetPriv().didAttachStencilBuffer(NULL);

    INHERITED::onRelease();
}

void GrRenderTarget::onAbandon() {
    this->renderTargetPriv().didAttachStencilBuffer(NULL);

    INHERITED::onAbandon();
}

///////////////////////////////////////////////////////////////////////////////

void GrRenderTargetPriv::didAttachStencilBuffer(GrStencilBuffer* stencilBuffer) {
    SkRefCnt_SafeAssign(fRenderTarget->fStencilBuffer, stencilBuffer);
}

GrStencilBuffer* GrRenderTargetPriv::attachStencilBuffer() const {
    if (fRenderTarget->fStencilBuffer) {
        return fRenderTarget->fStencilBuffer;
    }
    if (!fRenderTarget->wasDestroyed() && fRenderTarget->canAttemptStencilAttachment()) {
        fRenderTarget->getGpu()->attachStencilBufferToRenderTarget(fRenderTarget);
    }
    return fRenderTarget->fStencilBuffer;
}
