/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLTexture.h"
#include "GrGpuGL.h"

#define GPUGL static_cast<GrGpuGL*>(getGpu())

#define GL_CALL(X) GR_GL_CALL(GPUGL->glInterface(), X)

void GrGLTexture::init(GrGpuGL* gpu,
                       const GrSurfaceDesc& desc,
                       const IDDesc& idDesc,
                       const GrGLRenderTarget::IDDesc* rtIDDesc) {

    SkASSERT(0 != idDesc.fTextureID);

    fTexParams.invalidate();
    fTexParamsTimestamp = GrGpu::kExpiredTimestamp;
    fTexIDObj.reset(SkNEW_ARGS(GrGLTexID, (GPUGL->glInterface(),
                                           idDesc.fTextureID,
                                           idDesc.fIsWrapped)));

    if (rtIDDesc) {
        GrGLIRect vp;
        vp.fLeft   = 0;
        vp.fWidth  = desc.fWidth;
        vp.fBottom = 0;
        vp.fHeight = desc.fHeight;

        fRenderTarget.reset(SkNEW_ARGS(GrGLRenderTarget, (gpu, *rtIDDesc, vp, fTexIDObj, this)));
    }
    this->registerWithCache();
}

GrGLTexture::GrGLTexture(GrGpuGL* gpu, const GrSurfaceDesc& desc, const IDDesc& idDesc)
    : INHERITED(gpu, idDesc.fIsWrapped, desc) {
    this->init(gpu, desc, idDesc, NULL);
}

GrGLTexture::GrGLTexture(GrGpuGL* gpu,
                         const GrSurfaceDesc& desc,
                         const IDDesc& idDesc,
                         const GrGLRenderTarget::IDDesc& rtIDDesc)
    : INHERITED(gpu, idDesc.fIsWrapped, desc) {
    this->init(gpu, desc, idDesc, &rtIDDesc);
}

void GrGLTexture::onRelease() {
    fTexIDObj.reset(NULL);
    INHERITED::onRelease();
}

void GrGLTexture::onAbandon() {
    if (fTexIDObj.get()) {
        fTexIDObj->abandon();
        fTexIDObj.reset(NULL);
    }

    INHERITED::onAbandon();
}

GrBackendObject GrGLTexture::getTextureHandle() const {
    return static_cast<GrBackendObject>(this->textureID());
}
