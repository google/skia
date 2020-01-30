/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTraceMemoryDump.h"
#include "src/gpu/GrSemaphore.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/gl/GrGLGpu.h"
#include "src/gpu/gl/GrGLTexture.h"

#define GPUGL static_cast<GrGLGpu*>(this->getGpu())
#define GL_CALL(X) GR_GL_CALL(GPUGL->glInterface(), X)

GrTextureType GrGLTexture::TextureTypeFromTarget(GrGLenum target) {
    switch (target) {
        case GR_GL_TEXTURE_2D:
            return GrTextureType::k2D;
        case GR_GL_TEXTURE_RECTANGLE:
            return GrTextureType::kRectangle;
        case GR_GL_TEXTURE_EXTERNAL:
            return GrTextureType::kExternal;
    }
    SK_ABORT("Unexpected texture target");
}

static inline GrGLenum target_from_texture_type(GrTextureType type) {
    switch (type) {
        case GrTextureType::k2D:
            return GR_GL_TEXTURE_2D;
        case GrTextureType::kRectangle:
            return GR_GL_TEXTURE_RECTANGLE;
        case GrTextureType::kExternal:
            return GR_GL_TEXTURE_EXTERNAL;
        default:
            SK_ABORT("Unexpected texture target");
    }
    SK_ABORT("Unexpected texture type");
}

// Because this class is virtually derived from GrSurface we must explicitly call its constructor.
GrGLTexture::GrGLTexture(GrGLGpu* gpu, SkBudgeted budgeted, const Desc& desc,
                         GrMipMapsStatus mipMapsStatus)
        : GrSurface(gpu, desc.fSize, GrProtected::kNo)
        , INHERITED(gpu, desc.fSize, GrProtected::kNo,
                    TextureTypeFromTarget(desc.fTarget), mipMapsStatus)
        , fParameters(sk_make_sp<GrGLTextureParameters>()) {
    this->init(desc);
    this->registerWithCache(budgeted);
    if (GrGLFormatIsCompressed(desc.fFormat)) {
        this->setReadOnly();
    }
}

GrGLTexture::GrGLTexture(GrGLGpu* gpu, const Desc& desc, GrMipMapsStatus mipMapsStatus,
                         sk_sp<GrGLTextureParameters> parameters, GrWrapCacheable cacheable,
                         GrIOType ioType)
        : GrSurface(gpu, desc.fSize, GrProtected::kNo)
        , INHERITED(gpu, desc.fSize, GrProtected::kNo,
                    TextureTypeFromTarget(desc.fTarget), mipMapsStatus)
        , fParameters(std::move(parameters)) {
    SkASSERT(fParameters);
    this->init(desc);
    this->registerWithCacheWrapped(cacheable);
    if (ioType == kRead_GrIOType) {
        this->setReadOnly();
    }
}

GrGLTexture::GrGLTexture(GrGLGpu* gpu, const Desc& desc, sk_sp<GrGLTextureParameters> parameters,
                         GrMipMapsStatus mipMapsStatus)
        : GrSurface(gpu, desc.fSize, GrProtected::kNo)
        , INHERITED(gpu, desc.fSize, GrProtected::kNo,
                    TextureTypeFromTarget(desc.fTarget), mipMapsStatus) {
    SkASSERT(parameters || desc.fOwnership == GrBackendObjectOwnership::kOwned);
    fParameters = parameters ? std::move(parameters) : sk_make_sp<GrGLTextureParameters>();
    this->init(desc);
}

void GrGLTexture::init(const Desc& desc) {
    SkASSERT(0 != desc.fID);
    SkASSERT(GrGLFormat::kUnknown != desc.fFormat);
    fID = desc.fID;
    fFormat = desc.fFormat;
    fTextureIDOwnership = desc.fOwnership;
}

GrGLenum GrGLTexture::target() const {
    return target_from_texture_type(this->texturePriv().textureType());
}

void GrGLTexture::onRelease() {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);

    if (fID) {
        if (GrBackendObjectOwnership::kBorrowed != fTextureIDOwnership) {
            GL_CALL(DeleteTextures(1, &fID));
        }
        fID = 0;
    }
    INHERITED::onRelease();
}

void GrGLTexture::onAbandon() {
    fID = 0;
    INHERITED::onAbandon();
}

GrBackendTexture GrGLTexture::getBackendTexture() const {
    GrGLTextureInfo info;
    info.fTarget = target_from_texture_type(this->texturePriv().textureType());
    info.fID = fID;
    info.fFormat = GrGLFormatToEnum(fFormat);
    return GrBackendTexture(this->width(), this->height(), this->texturePriv().mipMapped(), info,
                            fParameters);
}

GrBackendFormat GrGLTexture::backendFormat() const {
    return GrBackendFormat::MakeGL(GrGLFormatToEnum(fFormat),
                                   target_from_texture_type(this->texturePriv().textureType()));
}

sk_sp<GrGLTexture> GrGLTexture::MakeWrapped(GrGLGpu* gpu,
                                            GrMipMapsStatus mipMapsStatus,
                                            const Desc& desc,
                                            sk_sp<GrGLTextureParameters> parameters,
                                            GrWrapCacheable cacheable,
                                            GrIOType ioType) {
    return sk_sp<GrGLTexture>(
            new GrGLTexture(gpu, desc, mipMapsStatus, std::move(parameters), cacheable, ioType));
}

bool GrGLTexture::onStealBackendTexture(GrBackendTexture* backendTexture,
                                        SkImage::BackendTextureReleaseProc* releaseProc) {
    *backendTexture = this->getBackendTexture();
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
