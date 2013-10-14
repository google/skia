
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

SK_DEFINE_INST_COUNT(GrRenderTarget)

bool GrRenderTarget::readPixels(int left, int top, int width, int height,
                                GrPixelConfig config,
                                void* buffer,
                                size_t rowBytes,
                                uint32_t pixelOpsFlags) {
    // go through context so that all necessary flushing occurs
    GrContext* context = this->getContext();
    if (NULL == context) {
        return false;
    }
    return context->readRenderTargetPixels(this,
                                           left, top, width, height,
                                           config, buffer, rowBytes,
                                           pixelOpsFlags);
}

void GrRenderTarget::writePixels(int left, int top, int width, int height,
                                 GrPixelConfig config,
                                 const void* buffer,
                                 size_t rowBytes,
                                 uint32_t pixelOpsFlags) {
    // go through context so that all necessary flushing occurs
    GrContext* context = this->getContext();
    if (NULL == context) {
        return;
    }
    context->writeRenderTargetPixels(this,
                                     left, top, width, height,
                                     config, buffer, rowBytes,
                                     pixelOpsFlags);
}

void GrRenderTarget::resolve() {
    // go through context so that all necessary flushing occurs
    GrContext* context = this->getContext();
    if (NULL == context) {
        return;
    }
    context->resolveRenderTarget(this);
}

size_t GrRenderTarget::sizeInBytes() const {
    size_t colorBits;
    if (kUnknown_GrPixelConfig == fDesc.fConfig) {
        colorBits = 32; // don't know, make a guess
    } else {
        colorBits = GrBytesPerPixel(fDesc.fConfig);
    }
    uint64_t size = fDesc.fWidth;
    size *= fDesc.fHeight;
    size *= colorBits;
    size *= GrMax(1, fDesc.fSampleCnt);
    return (size_t)(size / 8);
}

void GrRenderTarget::flagAsNeedingResolve(const SkIRect* rect) {
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
