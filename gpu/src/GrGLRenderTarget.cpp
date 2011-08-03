
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrGLRenderTarget.h"

#include "GrGpuGL.h"

#define GPUGL static_cast<GrGpuGL*>(getGpu())

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
    : INHERITED(gpu, texture, viewport.fWidth,
                viewport.fHeight, desc.fConfig, desc.fSampleCnt) {
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
    : INHERITED(gpu, NULL, viewport.fWidth,
                viewport.fHeight, desc.fConfig, desc.fSampleCnt) {
    this->init(desc, viewport, NULL);
}

void GrGLRenderTarget::onRelease() {
    GPUGL->notifyRenderTargetDelete(this);
    if (fOwnIDs) {
        if (fTexFBOID) {
            GR_GL(DeleteFramebuffers(1, &fTexFBOID));
        }
        if (fRTFBOID && fRTFBOID != fTexFBOID) {
            GR_GL(DeleteFramebuffers(1, &fRTFBOID));
        }
        if (fMSColorRenderbufferID) {
            GR_GL(DeleteRenderbuffers(1, &fMSColorRenderbufferID));
        }
    }
    fRTFBOID                = 0;
    fTexFBOID               = 0;
    fMSColorRenderbufferID  = 0;
    GrSafeUnref(fTexIDObj);
    fTexIDObj = NULL;
    GrSafeSetNull(fStencilBuffer);
}

void GrGLRenderTarget::onAbandon() {
    fRTFBOID                = 0;
    fTexFBOID               = 0;
    fMSColorRenderbufferID  = 0;
    if (NULL != fTexIDObj) {
        fTexIDObj->abandon();
        fTexIDObj = NULL;
    }
    GrSafeSetNull(fStencilBuffer);
}

