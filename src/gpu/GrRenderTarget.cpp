
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrRenderTarget.h"

#include "GrContext.h"
#include "GrGpu.h"
#include "GrStencilBuffer.h"

void GrRenderTarget::resolve() {
    // go through context so that all necessary flushing occurs
    GrContext* context = this->getContext();
    if (NULL == context) {
        return;
    }
    context->resolveRenderTarget(this);
}

void GrRenderTarget::discard() {
    // go through context so that all necessary flushing occurs
    GrContext* context = this->getContext();
    if (NULL == context) {
        return;
    }
    context->discardRenderTarget(this);
}

size_t GrRenderTarget::gpuMemorySize() const {
    size_t colorBits;
    if (kUnknown_GrPixelConfig == fDesc.fConfig) {
        colorBits = 32; // don't know, make a guess
    } else {
        colorBits = GrBytesPerPixel(fDesc.fConfig) * 8;
    }
    uint64_t size = fDesc.fWidth;
    size *= fDesc.fHeight;
    size *= colorBits;
    size *= SkTMax(1, fDesc.fSampleCnt);
    return (size_t)(size / 8);
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

void GrRenderTarget::setStencilBuffer(GrStencilBuffer* stencilBuffer) {
    SkRefCnt_SafeAssign(fStencilBuffer, stencilBuffer);
}

void GrRenderTarget::onRelease() {
    this->setStencilBuffer(NULL);

    INHERITED::onRelease();
}

void GrRenderTarget::onAbandon() {
    this->setStencilBuffer(NULL);

    INHERITED::onAbandon();
}
