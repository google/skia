/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLTexture.h"
#include "GrGLGpu.h"
#include "GrSemaphore.h"
#include "GrShaderCaps.h"
#include "GrTexturePriv.h"
#include "SkTraceMemoryDump.h"

#define GPUGL static_cast<GrGLGpu*>(this->getGpu())
#define GL_CALL(X) GR_GL_CALL(GPUGL->glInterface(), X)

static inline GrSLType sampler_type(const GrGLTexture::IDDesc& idDesc, GrPixelConfig config,
                                    const GrGLGpu* gpu) {
    if (idDesc.fInfo.fTarget == GR_GL_TEXTURE_EXTERNAL) {
        SkASSERT(gpu->caps()->shaderCaps()->externalTextureSupport());
        SkASSERT(!GrPixelConfigIsSint(config));
        return kTextureExternalSampler_GrSLType;
    } else if (idDesc.fInfo.fTarget == GR_GL_TEXTURE_RECTANGLE) {
        SkASSERT(gpu->glCaps().rectangleTextureSupport());
        SkASSERT(!GrPixelConfigIsSint(config));
        return kTexture2DRectSampler_GrSLType;
    } else if (GrPixelConfigIsSint(config)) {
        return kITexture2DSampler_GrSLType;
    } else {
        SkASSERT(idDesc.fInfo.fTarget == GR_GL_TEXTURE_2D);
        return kTexture2DSampler_GrSLType;
    }
}

// This method parallels GrTextureProxy::highestFilterMode
static inline GrSamplerState::Filter highest_filter_mode(const GrGLTexture::IDDesc& idDesc,
                                                         GrPixelConfig config) {
    if (GrPixelConfigIsSint(config)) {
        // Integer textures in GL can use GL_NEAREST_MIPMAP_NEAREST. This is a mode we don't support
        // and don't currently have a use for.
        return GrSamplerState::Filter::kNearest;
    }
    if (idDesc.fInfo.fTarget == GR_GL_TEXTURE_RECTANGLE ||
        idDesc.fInfo.fTarget == GR_GL_TEXTURE_EXTERNAL) {
        return GrSamplerState::Filter::kBilerp;
    }
    return GrSamplerState::Filter::kMipMap;
}

// Because this class is virtually derived from GrSurface we must explicitly call its constructor.
GrGLTexture::GrGLTexture(GrGLGpu* gpu, SkBudgeted budgeted, const GrSurfaceDesc& desc,
                         const IDDesc& idDesc, GrMipMapsStatus mipMapsStatus)
    : GrSurface(gpu, desc)
    , INHERITED(gpu, desc, sampler_type(idDesc, desc.fConfig, gpu),
                highest_filter_mode(idDesc, desc.fConfig), mipMapsStatus) {
    this->init(desc, idDesc);
    this->registerWithCache(budgeted);
}

GrGLTexture::GrGLTexture(GrGLGpu* gpu, Wrapped, const GrSurfaceDesc& desc,
                         GrMipMapsStatus mipMapsStatus, const IDDesc& idDesc)
    : GrSurface(gpu, desc)
    , INHERITED(gpu, desc, sampler_type(idDesc, desc.fConfig, gpu),
                highest_filter_mode(idDesc, desc.fConfig), mipMapsStatus) {
    this->init(desc, idDesc);
    this->registerWithCacheWrapped();
}

GrGLTexture::GrGLTexture(GrGLGpu* gpu, const GrSurfaceDesc& desc, const IDDesc& idDesc,
                         GrMipMapsStatus mipMapsStatus)
    : GrSurface(gpu, desc)
    , INHERITED(gpu, desc, sampler_type(idDesc, desc.fConfig, gpu),
                highest_filter_mode(idDesc, desc.fConfig), mipMapsStatus) {
    this->init(desc, idDesc);
}

void GrGLTexture::init(const GrSurfaceDesc& desc, const IDDesc& idDesc) {
    SkASSERT(0 != idDesc.fInfo.fID);
    SkASSERT(0 != idDesc.fInfo.fFormat);
    fTexParams.invalidate();
    fTexParamsTimestamp = GrGpu::kExpiredTimestamp;
    fInfo = idDesc.fInfo;
    fTextureIDOwnership = idDesc.fOwnership;
}

void GrGLTexture::onRelease() {
    if (fInfo.fID) {
        if (GrBackendObjectOwnership::kBorrowed != fTextureIDOwnership) {
            GL_CALL(DeleteTextures(1, &fInfo.fID));
        }
        fInfo.fID = 0;
    }
    this->invokeReleaseProc();
    INHERITED::onRelease();
}

void GrGLTexture::onAbandon() {
    fInfo.fTarget = 0;
    fInfo.fID = 0;
    this->invokeReleaseProc();
    INHERITED::onAbandon();
}

GrBackendObject GrGLTexture::getTextureHandle() const {
    return reinterpret_cast<GrBackendObject>(&fInfo);
}

GrBackendTexture GrGLTexture::getBackendTexture() const {
    return GrBackendTexture(this->width(), this->height(), this->texturePriv().mipMapped(), fInfo);
}

void GrGLTexture::setMemoryBacking(SkTraceMemoryDump* traceMemoryDump,
                                   const SkString& dumpName) const {
    SkString texture_id;
    texture_id.appendU32(this->textureID());
    traceMemoryDump->setMemoryBacking(dumpName.c_str(), "gl_texture",
                                      texture_id.c_str());
}

sk_sp<GrGLTexture> GrGLTexture::MakeWrapped(GrGLGpu* gpu, const GrSurfaceDesc& desc,
                                            GrMipMapsStatus mipMapsStatus, const IDDesc& idDesc) {
    return sk_sp<GrGLTexture>(new GrGLTexture(gpu, kWrapped, desc, mipMapsStatus, idDesc));
}

bool GrGLTexture::onStealBackendTexture(GrBackendTexture* backendTexture,
                                        SkImage::BackendTextureReleaseProc* releaseProc) {
    *backendTexture = GrBackendTexture(width(), height(), config(), fInfo);
    // Set the release proc to a no-op function. GL doesn't require any special cleanup.
    *releaseProc = [](GrBackendTexture){};

    // It's important that we only abandon this texture's objects, not subclass objects such as
    // those held by GrGLTextureRenderTarget. Those objects are not being stolen and need to be
    // cleaned up by us.
    this->GrGLTexture::onAbandon();
    return true;
}
