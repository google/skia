/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLRenderTarget.h"

#include "GrGpuGL.h"

#define GPUGL static_cast<GrGpuGL*>(getGpu())

#define GL_CALL(X) GR_GL_CALL(GPUGL->glInterface(), X)

void GrGLRenderTarget::init(const GrSurfaceDesc& desc,
                            const IDDesc& idDesc,
                            const GrGLIRect& viewport,
                            GrGLTexID* texID) {
    fRTFBOID                = idDesc.fRTFBOID;
    fTexFBOID               = idDesc.fTexFBOID;
    fMSColorRenderbufferID  = idDesc.fMSColorRenderbufferID;
    fViewport               = viewport;
    fTexIDObj.reset(SkSafeRef(texID));
    this->registerWithCache();
}

GrGLRenderTarget::GrGLRenderTarget(GrGpuGL* gpu,
                                   const IDDesc& idDesc,
                                   const GrGLIRect& viewport,
                                   GrGLTexID* texID,
                                   GrGLTexture* texture)
    : INHERITED(gpu, idDesc.fIsWrapped, texture, texture->desc()) {
    SkASSERT(texID);
    SkASSERT(texture);
    // FBO 0 can't also be a texture, right?
    SkASSERT(0 != idDesc.fRTFBOID);
    SkASSERT(0 != idDesc.fTexFBOID);

    // we assume this is true, TODO: get rid of viewport as a param.
    SkASSERT(viewport.fWidth == texture->width());
    SkASSERT(viewport.fHeight == texture->height());

    this->init(texture->desc(), idDesc, viewport, texID);
}

GrGLRenderTarget::GrGLRenderTarget(GrGpuGL* gpu,
                                   const GrSurfaceDesc& desc,
                                   const IDDesc& idDesc,
                                   const GrGLIRect& viewport)
    : INHERITED(gpu, idDesc.fIsWrapped, NULL, desc) {
    this->init(desc, idDesc, viewport, NULL);
}

void GrGLRenderTarget::onRelease() {
    if (!this->isWrapped()) {
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
    fTexIDObj.reset(NULL);
    INHERITED::onRelease();
}

void GrGLRenderTarget::onAbandon() {
    fRTFBOID                = 0;
    fTexFBOID               = 0;
    fMSColorRenderbufferID  = 0;
    if (fTexIDObj.get()) {
        fTexIDObj->abandon();
        fTexIDObj.reset(NULL);
    }
    INHERITED::onAbandon();
}
