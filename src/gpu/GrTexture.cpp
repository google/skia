
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrTexture.h"

#include "GrContext.h"
#include "GrGpu.h"
#include "GrRenderTarget.h"

bool GrTexture::readPixels(int left, int top, int width, int height,
                           GrPixelConfig config, void* buffer,
                           size_t rowBytes) {
    // go through context so that all necessary flushing occurs
    GrContext* context = this->getContext();
    if (NULL == context) {
        return false;
    }
    return context->readTexturePixels(this,
                                      left, top,
                                      width, height,
                                      config, buffer, rowBytes);
}

void GrTexture::writePixels(int left, int top, int width, int height,
                            GrPixelConfig config, const void* buffer,
                            size_t rowBytes) {
    // go through context so that all necessary flushing occurs
    GrContext* context = this->getContext();
    if (NULL == context) {
        return;
    }
    context->writeTexturePixels(this,
                                left, top,
                                width, height,
                                config, buffer, rowBytes);
}

void GrTexture::releaseRenderTarget() {
    if (NULL != fRenderTarget) {
        GrAssert(fRenderTarget->asTexture() == this);
        GrAssert(fDesc.fFlags & kRenderTarget_GrTextureFlagBit);

        fRenderTarget->onTextureReleaseRenderTarget();
        fRenderTarget->unref();
        fRenderTarget = NULL;

        fDesc.fFlags = fDesc.fFlags &
            ~(kRenderTarget_GrTextureFlagBit|kNoStencil_GrTextureFlagBit);
        fDesc.fSampleCnt = 0;
    }
}

void GrTexture::onAbandon() {
    if (NULL != fRenderTarget) {
        fRenderTarget->abandon();
    }
}

void GrTexture::validateDesc() const {
    if (NULL != this->asRenderTarget()) {
        // This texture has a render target
        GrAssert(0 != (fDesc.fFlags & kRenderTarget_GrTextureFlagBit));

        if (NULL != this->asRenderTarget()->getStencilBuffer()) {
            GrAssert(0 != (fDesc.fFlags & kNoStencil_GrTextureFlagBit));
        } else {
            GrAssert(0 == (fDesc.fFlags & kNoStencil_GrTextureFlagBit));
        }

        GrAssert(fDesc.fSampleCnt == this->asRenderTarget()->numSamples());
    } else {
        GrAssert(0 == (fDesc.fFlags & kRenderTarget_GrTextureFlagBit));
        GrAssert(0 == (fDesc.fFlags & kNoStencil_GrTextureFlagBit));
        GrAssert(0 == fDesc.fSampleCnt);
    }
}
