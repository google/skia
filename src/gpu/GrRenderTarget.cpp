
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

bool GrRenderTarget::readPixels(int left, int top, int width, int height,
                                GrPixelConfig config, void* buffer) {
    // go through context so that all necessary flushing occurs
    GrContext* context = this->getGpu()->getContext();
    GrAssert(NULL != context);
    return context->readRenderTargetPixels(this,
                                           left, top, 
                                           width, height,
                                           config, buffer);
}

size_t GrRenderTarget::sizeInBytes() const {
    int colorBits;
    if (kUnknown_GrPixelConfig == fConfig) {
        colorBits = 32; // don't know, make a guess
    } else {
        colorBits = GrBytesPerPixel(fConfig);
    }
    uint64_t size = fAllocatedWidth;
    size *= fAllocatedHeight;
    size *= colorBits;
    size *= GrMax(1,fSampleCnt);
    return (size_t)(size / 8);
}

void GrRenderTarget::flagAsNeedingResolve(const GrIRect* rect) {
    if (kCanResolve_ResolveType == getResolveType()) {
        if (NULL != rect) {
            fResolveRect.join(*rect);
            if (!fResolveRect.intersect(0, 0, this->width(), this->height())) {
                fResolveRect.setEmpty();
            }
        } else {
            fResolveRect.setLTRB(0, 0, this->width(), this->height());
        }
    }
}

void GrRenderTarget::overrideResolveRect(const GrIRect rect) {
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
    if (NULL != fStencilBuffer) {
        fStencilBuffer->wasDetachedFromRenderTarget(this);
        fStencilBuffer->unref();
    }
    fStencilBuffer = stencilBuffer;
    if (NULL != fStencilBuffer) {
        fStencilBuffer->wasAttachedToRenderTarget(this);
        fStencilBuffer->ref();
    }
}
