/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLRenderTarget.h"

#include "GrGLGpu.h"

#define GPUGL static_cast<GrGLGpu*>(this->getGpu())
#define GL_CALL(X) GR_GL_CALL(GPUGL->glInterface(), X)

// Because this class is virtually derived from GrSurface we must explicitly call its constructor.
GrGLRenderTarget::GrGLRenderTarget(GrGLGpu* gpu, const GrSurfaceDesc& desc, const IDDesc& idDesc)
    : GrSurface(gpu, idDesc.fLifeCycle, desc)
    , INHERITED(gpu, idDesc.fLifeCycle, desc, idDesc.fSampleConfig) {
    this->init(desc, idDesc);
    this->registerWithCache();
}

GrGLRenderTarget::GrGLRenderTarget(GrGLGpu* gpu, const GrSurfaceDesc& desc, const IDDesc& idDesc,
                                   Derived)
    : GrSurface(gpu, idDesc.fLifeCycle, desc)
    , INHERITED(gpu, idDesc.fLifeCycle, desc, idDesc.fSampleConfig) {
    this->init(desc, idDesc);
}

void GrGLRenderTarget::init(const GrSurfaceDesc& desc, const IDDesc& idDesc) {
    fRTFBOID                = idDesc.fRTFBOID;
    fTexFBOID               = idDesc.fTexFBOID;
    fMSColorRenderbufferID  = idDesc.fMSColorRenderbufferID;
    fRTLifecycle            = idDesc.fLifeCycle;

    fViewport.fLeft   = 0;
    fViewport.fBottom = 0;
    fViewport.fWidth  = desc.fWidth;
    fViewport.fHeight = desc.fHeight;

    // We own one color value for each MSAA sample.
    int colorValuesPerPixel = SkTMax(1, fDesc.fSampleCnt);
    if (fTexFBOID != fRTFBOID) {
        // If we own the resolve buffer then that is one more sample per pixel.
        colorValuesPerPixel += 1;
    } else if (fTexFBOID != 0) {
        // For auto-resolving FBOs, the MSAA buffer is free.
        colorValuesPerPixel = 1;
    }
    SkASSERT(kUnknown_GrPixelConfig != fDesc.fConfig);
    SkASSERT(!GrPixelConfigIsCompressed(fDesc.fConfig));
    size_t colorBytes = GrBytesPerPixel(fDesc.fConfig);
    SkASSERT(colorBytes > 0);
    fGpuMemorySize = colorValuesPerPixel * fDesc.fWidth * fDesc.fHeight * colorBytes;
}

size_t GrGLRenderTarget::onGpuMemorySize() const {
    return fGpuMemorySize;
}

void GrGLRenderTarget::onRelease() {
    if (kBorrowed_LifeCycle != fRTLifecycle) {
        if (fTexFBOID) {
            GL_CALL(DeleteFramebuffers(1, &fTexFBOID));
        }
        if (fRTFBOID && fRTFBOID != fTexFBOID) {
            GL_CALL(DeleteFramebuffers(1, &fRTFBOID));
        }
        if (fMSColorRenderbufferID) {
            GL_CALL(DeleteRenderbuffers(1, &fMSColorRenderbufferID));
        }
    }
    fRTFBOID                = 0;
    fTexFBOID               = 0;
    fMSColorRenderbufferID  = 0;
    INHERITED::onRelease();
}

void GrGLRenderTarget::onAbandon() {
    fRTFBOID                = 0;
    fTexFBOID               = 0;
    fMSColorRenderbufferID  = 0;
    INHERITED::onAbandon();
}
