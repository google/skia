/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLTexture.h"
#include "GrGpuGL.h"

#define GPUGL static_cast<GrGpuGL*>(this->getGpu())
#define GL_CALL(X) GR_GL_CALL(GPUGL->glInterface(), X)

// Because this class is virtually derived from GrSurface we must explicitly call its constructor.
GrGLTexture::GrGLTexture(GrGpuGL* gpu, const GrSurfaceDesc& desc, const IDDesc& idDesc)
    : GrSurface(gpu, idDesc.fIsWrapped, desc)
    , INHERITED(gpu, idDesc.fIsWrapped, desc) {
    this->init(desc, idDesc);
    this->registerWithCache();
}

GrGLTexture::GrGLTexture(GrGpuGL* gpu, const GrSurfaceDesc& desc, const IDDesc& idDesc, Derived)
    : GrSurface(gpu, idDesc.fIsWrapped, desc)
    , INHERITED(gpu, idDesc.fIsWrapped, desc) {
    this->init(desc, idDesc);
}

void GrGLTexture::init(const GrSurfaceDesc& desc, const IDDesc& idDesc) {
    SkASSERT(0 != idDesc.fTextureID);
    fTexParams.invalidate();
    fTexParamsTimestamp = GrGpu::kExpiredTimestamp;
    fTexIDObj.reset(SkNEW_ARGS(GrGLTexID, (GPUGL->glInterface(),
                                           idDesc.fTextureID,
                                           idDesc.fIsWrapped)));
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
