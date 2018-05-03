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
        return kTextureExternalSampler_GrSLType;
    } else if (idDesc.fInfo.fTarget == GR_GL_TEXTURE_RECTANGLE) {
        SkASSERT(gpu->glCaps().rectangleTextureSupport());
        return kTexture2DRectSampler_GrSLType;
    } else {
        SkASSERT(idDesc.fInfo.fTarget == GR_GL_TEXTURE_2D);
        return kTexture2DSampler_GrSLType;
    }
}

// This method parallels GrTextureProxy::highestFilterMode
static inline GrSamplerState::Filter highest_filter_mode(const GrGLTexture::IDDesc& idDesc,
                                                         GrPixelConfig config) {
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
    if (idDesc.fInfo.fTarget == GR_GL_TEXTURE_RECTANGLE ||
        idDesc.fInfo.fTarget == GR_GL_TEXTURE_EXTERNAL) {
        this->setIsGLTextureRectangleOrExternal();
    }
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

GrBackendTexture GrGLTexture::getBackendTexture() const {
    return GrBackendTexture(this->width(), this->height(), this->texturePriv().mipMapped(), fInfo);
}

sk_sp<GrGLTexture> GrGLTexture::MakeWrapped(GrGLGpu* gpu, const GrSurfaceDesc& desc,
                                            GrMipMapsStatus mipMapsStatus, const IDDesc& idDesc) {
    return sk_sp<GrGLTexture>(new GrGLTexture(gpu, kWrapped, desc, mipMapsStatus, idDesc));
}

bool GrGLTexture::onStealBackendTexture(GrBackendTexture* backendTexture,
                                        SkImage::BackendTextureReleaseProc* releaseProc) {
    *backendTexture = GrBackendTexture(this->width(), this->height(),
                                       this->texturePriv().mipMapped(), fInfo);
    // Set the release proc to a no-op function. GL doesn't require any special cleanup.
    *releaseProc = [](GrBackendTexture){};

    // It's important that we only abandon this texture's objects, not subclass objects such as
    // those held by GrGLTextureRenderTarget. Those objects are not being stolen and need to be
    // cleaned up by us.
    this->GrGLTexture::onAbandon();
    return true;
}

void GrGLTexture::dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const {
    // Don't check this->fRefsWrappedObjects, as we might be the base of a GrGLTextureRenderTarget
    // which is multiply inherited from both ourselves and a texture. In these cases, one part
    // (texture, rt) may be wrapped, while the other is owned by Skia.
    bool refsWrappedTextureObjects =
            this->fTextureIDOwnership == GrBackendObjectOwnership::kBorrowed;
    if (refsWrappedTextureObjects && !traceMemoryDump->shouldDumpWrappedObjects()) {
        return;
    }

    // Dump as skia/gpu_resources/resource_#/texture, to avoid conflicts in the
    // GrGLTextureRenderTarget case, where multiple things may dump to the same resource. This
    // has no downside in the normal case.
    SkString resourceName = this->getResourceName();
    resourceName.append("/texture");

    // As we are only dumping our texture memory (not any additional memory tracked by classes
    // which may inherit from us), specifically call GrGLTexture::gpuMemorySize to avoid
    // hitting an override.
    this->dumpMemoryStatisticsPriv(traceMemoryDump, resourceName, "Texture",
                                   GrGLTexture::gpuMemorySize());

    SkString texture_id;
    texture_id.appendU32(this->textureID());
    traceMemoryDump->setMemoryBacking(resourceName.c_str(), "gl_texture", texture_id.c_str());
}
