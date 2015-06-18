/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLTexture.h"
#include "GrGLGpu.h"

#define GPUGL static_cast<GrGLGpu*>(this->getGpu())
#define GL_CALL(X) GR_GL_CALL(GPUGL->glInterface(), X)

// Because this class is virtually derived from GrSurface we must explicitly call its constructor.
GrGLTexture::GrGLTexture(GrGLGpu* gpu, const GrSurfaceDesc& desc, const IDDesc& idDesc)
    : GrSurface(gpu, idDesc.fLifeCycle, desc)
    , INHERITED(gpu, idDesc.fLifeCycle, desc) {
    this->init(desc, idDesc);
    this->registerWithCache();
}

GrGLTexture::GrGLTexture(GrGLGpu* gpu, const GrSurfaceDesc& desc, const IDDesc& idDesc, Derived)
    : GrSurface(gpu, idDesc.fLifeCycle, desc)
    , INHERITED(gpu, idDesc.fLifeCycle, desc) {
    this->init(desc, idDesc);
}

void GrGLTexture::init(const GrSurfaceDesc& desc, const IDDesc& idDesc) {
    SkASSERT(0 != idDesc.fTextureID);
    fTexParams.invalidate();
    fTexParamsTimestamp = GrGpu::kExpiredTimestamp;
    fTextureID = idDesc.fTextureID;
    fTextureIDLifecycle = idDesc.fLifeCycle;
}

void GrGLTexture::onRelease() {
    if (fTextureID) {
        if (GrGpuResource::kBorrowed_LifeCycle != fTextureIDLifecycle) {
            GL_CALL(DeleteTextures(1, &fTextureID));
        }
        fTextureID = 0;
    }
    INHERITED::onRelease();
}

void GrGLTexture::onAbandon() {
    fTextureID = 0;
    INHERITED::onAbandon();
}

GrBackendObject GrGLTexture::getTextureHandle() const {
    return static_cast<GrBackendObject>(this->textureID());
}
