/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLRenderTarget.h"

#include "GrGLGpu.h"

void GrGLFBO::release(const GrGLInterface* gl) {
    SkASSERT(gl);
    if (this->isValid()) {
        GR_GL_CALL(gl, DeleteFramebuffers(1, &fID));
        fIsValid = false;
    }
}

void GrGLFBO::abandon() { fIsValid = false; }

//////////////////////////////////////////////////////////////////////////////

#define GLGPU static_cast<GrGLGpu*>(this->getGpu())
#define GL_CALL(X) GR_GL_CALL(GLGPU->glInterface(), X)

// Because this class is virtually derived from GrSurface we must explicitly call its constructor.
GrGLRenderTarget::GrGLRenderTarget(GrGLGpu* gpu, const GrSurfaceDesc& desc, const IDDesc& idDesc)
    : GrSurface(gpu, idDesc.fLifeCycle, desc)
    , INHERITED(gpu, idDesc.fLifeCycle, desc) {
    this->init(desc, idDesc);
    this->registerWithCache();
}

GrGLRenderTarget::GrGLRenderTarget(GrGLGpu* gpu, const GrSurfaceDesc& desc, const IDDesc& idDesc,
                                   Derived)
    : GrSurface(gpu, idDesc.fLifeCycle, desc)
    , INHERITED(gpu, idDesc.fLifeCycle, desc) {
    this->init(desc, idDesc);
}

void GrGLRenderTarget::init(const GrSurfaceDesc& desc, const IDDesc& idDesc) {
    fRenderFBO.reset(SkRef(idDesc.fRenderFBO.get()));
    fTextureFBO.reset(SkSafeRef(idDesc.fTextureFBO.get()));
    SkASSERT(fRenderFBO->isValid());
    SkASSERT(!fTextureFBO || fTextureFBO->isValid());
    fMSColorRenderbufferID  = idDesc.fMSColorRenderbufferID;
    fIsWrapped              = kWrapped_LifeCycle == idDesc.fLifeCycle;

    fViewport.fLeft   = 0;
    fViewport.fBottom = 0;
    fViewport.fWidth  = desc.fWidth;
    fViewport.fHeight = desc.fHeight;

    // We own one color value for each MSAA sample.
    fColorValuesPerPixel = SkTMax(1, fDesc.fSampleCnt);
    if (fTextureFBO && fTextureFBO != fRenderFBO) {
        // If we own the resolve buffer then that is one more sample per pixel.
        fColorValuesPerPixel += 1;
    } 
}

size_t GrGLRenderTarget::onGpuMemorySize() const {
    SkASSERT(kUnknown_GrPixelConfig != fDesc.fConfig);
    SkASSERT(!GrPixelConfigIsCompressed(fDesc.fConfig));
    size_t colorBytes = GrBytesPerPixel(fDesc.fConfig);
    SkASSERT(colorBytes > 0);
    return fColorValuesPerPixel * fDesc.fWidth * fDesc.fHeight * colorBytes;
}

void GrGLRenderTarget::onRelease() {
    if (!fIsWrapped) {
        const GrGLInterface* gl = GLGPU->glInterface();
        if (fRenderFBO) {
            fRenderFBO->release(gl);
            fRenderFBO.reset(NULL);
        }
        if (fTextureFBO) {
            fTextureFBO->release(gl);
            fTextureFBO.reset(NULL);
        }
        if (fMSColorRenderbufferID) {
            GL_CALL(DeleteRenderbuffers(1, &fMSColorRenderbufferID));
            fMSColorRenderbufferID = 0;
        }
    } else {
        if (fRenderFBO) {
            fRenderFBO->abandon();
            fRenderFBO.reset(NULL);
        }
        if (fTextureFBO) {
            fTextureFBO->abandon();
            fTextureFBO.reset(NULL);
        }
        fMSColorRenderbufferID  = 0;
    }
    INHERITED::onRelease();
}

void GrGLRenderTarget::onAbandon() {
    if (fRenderFBO) {
        fRenderFBO->abandon();
        fRenderFBO.reset(NULL);
    }
    if (fTextureFBO) {
        fTextureFBO->abandon();
        fTextureFBO.reset(NULL);
    }
    fMSColorRenderbufferID  = 0;
    INHERITED::onAbandon();
}
