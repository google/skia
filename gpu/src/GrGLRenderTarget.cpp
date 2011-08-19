
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

void GrGLRenderTarget::init(const Desc& desc,
                            const GrGLIRect& viewport,
                            GrGLTexID* texID) {
    fRTFBOID                = desc.fRTFBOID;
    fTexFBOID               = desc.fTexFBOID;
    fMSColorRenderbufferID  = desc.fMSColorRenderbufferID;
    fViewport               = viewport;
    fOwnIDs                 = desc.fOwnIDs;
    fTexIDObj               = texID;
    GrSafeRef(fTexIDObj);
}

GrGLRenderTarget::GrGLRenderTarget(GrGpuGL* gpu,
                                   const Desc& desc,
                                   const GrGLIRect& viewport,
                                   GrGLTexID* texID,
                                   GrGLTexture* texture)
    : INHERITED(gpu,
                texture,
                viewport.fWidth,
                viewport.fHeight,
                texture->allocatedWidth(),
                texture->allocatedHeight(),
                desc.fConfig,
                desc.fSampleCnt) {
    GrAssert(NULL != texID);
    GrAssert(NULL != texture);
    // FBO 0 can't also be a texture, right?
    GrAssert(0 != desc.fRTFBOID);
    GrAssert(0 != desc.fTexFBOID);
    this->init(desc, viewport, texID);
}

GrGLRenderTarget::GrGLRenderTarget(GrGpuGL* gpu,
                                   const Desc& desc,
                                   const GrGLIRect& viewport)
    : INHERITED(gpu,
                NULL,
                viewport.fWidth,
                viewport.fHeight,
                viewport.fWidth,   // don't really need separate alloc w/h for
                viewport.fHeight,  // non-texture RTs, repeat viewport values
                desc.fConfig,
                desc.fSampleCnt) {
    this->init(desc, viewport, NULL);
}

void GrGLRenderTarget::onRelease() {
    GPUGL->notifyRenderTargetDelete(this);
    if (fOwnIDs) {
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
    GrSafeUnref(fTexIDObj);
    fTexIDObj = NULL;
    this->setStencilBuffer(NULL);
}

void GrGLRenderTarget::onAbandon() {
    fRTFBOID                = 0;
    fTexFBOID               = 0;
    fMSColorRenderbufferID  = 0;
    if (NULL != fTexIDObj) {
        fTexIDObj->abandon();
        fTexIDObj = NULL;
    }
    this->setStencilBuffer(NULL);
}

