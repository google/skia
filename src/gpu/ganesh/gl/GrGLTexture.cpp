/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/ganesh/gl/GrGLTexture.h"

#include "include/core/SkString.h"
#include "include/core/SkTraceMemoryDump.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/gpu/ganesh/gl/GrGLFunctions.h"
#include "include/gpu/ganesh/gl/GrGLInterface.h"
#include "include/private/base/SkAssert.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/ganesh/GrSurface.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/gl/GrGLBackendSurfacePriv.h"
#include "src/gpu/ganesh/gl/GrGLCaps.h"
#include "src/gpu/ganesh/gl/GrGLDefines.h"
#include "src/gpu/ganesh/gl/GrGLGpu.h"
#include "src/gpu/ganesh/gl/GrGLUtil.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <utility>

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
GrGLTexture::GrGLTexture(GrGLGpu* gpu,
                         skgpu::Budgeted budgeted,
                         const Desc& desc,
                         GrMipmapStatus mipmapStatus,
                         std::string_view label)
        : GrSurface(gpu, desc.fSize, desc.fIsProtected, label)
        , GrTexture(gpu,
                    desc.fSize,
                    desc.fIsProtected,
                    TextureTypeFromTarget(desc.fTarget),
                    mipmapStatus,
                    label)
        , fParameters(sk_make_sp<GrGLTextureParameters>()) {
    this->init(desc);
    this->registerWithCache(budgeted);
    if (GrGLFormatIsCompressed(desc.fFormat)) {
        this->setReadOnly();
    }
}

GrGLTexture::GrGLTexture(GrGLGpu* gpu, const Desc& desc, GrMipmapStatus mipmapStatus,
                         sk_sp<GrGLTextureParameters> parameters, GrWrapCacheable cacheable,
                         GrIOType ioType, std::string_view label)
        : GrSurface(gpu, desc.fSize, desc.fIsProtected, label)
        , GrTexture(gpu,
                    desc.fSize,
                    desc.fIsProtected,
                    TextureTypeFromTarget(desc.fTarget),
                    mipmapStatus,
                    label)
        , fParameters(std::move(parameters)) {
    SkASSERT(fParameters);
    this->init(desc);
    this->registerWithCacheWrapped(cacheable);
    if (ioType == kRead_GrIOType) {
        this->setReadOnly();
    }
}

GrGLTexture::GrGLTexture(GrGLGpu* gpu,
                         const Desc& desc,
                         sk_sp<GrGLTextureParameters> parameters,
                         GrMipmapStatus mipmapStatus,
                         std::string_view label)
        : GrSurface(gpu, desc.fSize, desc.fIsProtected, label)
        , GrTexture(gpu,
                    desc.fSize,
                    desc.fIsProtected,
                    TextureTypeFromTarget(desc.fTarget),
                    mipmapStatus,
                    label) {
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

GrGLenum GrGLTexture::target() const { return target_from_texture_type(this->textureType()); }

void GrGLTexture::onRelease() {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    ATRACE_ANDROID_FRAMEWORK_ALWAYS("Texture release(%u)", this->uniqueID().asUInt());

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
    info.fTarget = target_from_texture_type(this->textureType());
    info.fID = fID;
    info.fFormat = GrGLFormatToEnum(fFormat);
    info.fProtected = skgpu::Protected(this->isProtected());

    return GrBackendTextures::MakeGL(
            this->width(), this->height(), this->mipmapped(), info, fParameters);
}

GrBackendFormat GrGLTexture::backendFormat() const {
    return GrBackendFormats::MakeGL(GrGLFormatToEnum(fFormat),
                                    target_from_texture_type(this->textureType()));
}

sk_sp<GrGLTexture> GrGLTexture::MakeWrapped(GrGLGpu* gpu,
                                            GrMipmapStatus mipmapStatus,
                                            const Desc& desc,
                                            sk_sp<GrGLTextureParameters> parameters,
                                            GrWrapCacheable cacheable,
                                            GrIOType ioType,
                                            std::string_view label) {
    return sk_sp<GrGLTexture>(new GrGLTexture(
            gpu, desc, mipmapStatus, std::move(parameters), cacheable, ioType, label));
}

bool GrGLTexture::onStealBackendTexture(GrBackendTexture* backendTexture,
                                        SkImages::BackendTextureReleaseProc* releaseProc) {
    *backendTexture = this->getBackendTexture();
    // Set the release proc to a no-op function. GL doesn't require any special cleanup.
    *releaseProc = [](GrBackendTexture){};

    // It's important that we only abandon this texture's objects, not subclass objects such as
    // those held by GrGLTextureRenderTarget. Those objects are not being stolen and need to be
    // cleaned up by us.
    this->GrGLTexture::onAbandon();
    return true;
}

void GrGLTexture::onSetLabel() {
    SkASSERT(fID);
    SkASSERT(fTextureIDOwnership == GrBackendObjectOwnership::kOwned);
    if (!this->getLabel().empty()) {
        const std::string label = "_Skia_" + this->getLabel();
        GrGLGpu* glGpu = static_cast<GrGLGpu*>(this->getGpu());
        if (glGpu->glCaps().debugSupport()) {
            GR_GL_CALL(glGpu->glInterface(), ObjectLabel(GR_GL_TEXTURE, fID, -1, label.c_str()));
        }
    }
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

    size_t size = GrSurface::ComputeSize(this->backendFormat(), this->dimensions(), 1,
                                         this->mipmapped());

    // Dump as skia/gpu_resources/resource_#/texture, to avoid conflicts in the
    // GrGLTextureRenderTarget case, where multiple things may dump to the same resource. This
    // has no downside in the normal case.
    SkString resourceName = this->getResourceName();
    resourceName.append("/texture");

    // As we are only dumping our texture memory (not any additional memory tracked by classes
    // which may inherit from us), specifically call GrGLTexture::gpuMemorySize to avoid
    // hitting an override.
    this->dumpMemoryStatisticsPriv(traceMemoryDump, resourceName, "Texture", size);

    SkString texture_id;
    texture_id.appendU32(this->textureID());
    traceMemoryDump->setMemoryBacking(resourceName.c_str(), "gl_texture", texture_id.c_str());
}
