/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/mtl/GrMtlTexture.h"

#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/mtl/GrMtlGpu.h"
#include "src/gpu/mtl/MtlUtilsPriv.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

GR_NORETAIN_BEGIN

GrMtlTexture::GrMtlTexture(GrMtlGpu* gpu,
                           skgpu::Budgeted budgeted,
                           SkISize dimensions,
                           sk_sp<GrMtlAttachment> texture,
                           GrMipmapStatus mipmapStatus,
                           std::string_view label)
        : GrSurface(gpu, dimensions, GrProtected::kNo, label)
        , INHERITED(gpu, dimensions, GrProtected::kNo, GrTextureType::k2D, mipmapStatus, label)
        , fTexture(std::move(texture)) {
    SkDEBUGCODE(id<MTLTexture> mtlTexture = fTexture->mtlTexture();)
    SkASSERT((GrMipmapStatus::kNotAllocated == mipmapStatus) == (1 == mtlTexture.mipmapLevelCount));
    if (@available(macOS 10.11, iOS 9.0, tvOS 9.0, *)) {
        SkASSERT(SkToBool(mtlTexture.usage & MTLTextureUsageShaderRead));
    }
    SkASSERT(!mtlTexture.framebufferOnly);
    this->registerWithCache(budgeted);
    if (skgpu::MtlFormatIsCompressed(fTexture->mtlFormat())) {
        this->setReadOnly();
    }
}

GrMtlTexture::GrMtlTexture(GrMtlGpu* gpu,
                           Wrapped,
                           SkISize dimensions,
                           sk_sp<GrMtlAttachment> texture,
                           GrMipmapStatus mipmapStatus,
                           GrWrapCacheable cacheable,
                           GrIOType ioType,
                           std::string_view label)
        : GrSurface(gpu, dimensions, GrProtected::kNo, label)
        , INHERITED(gpu, dimensions, GrProtected::kNo, GrTextureType::k2D, mipmapStatus, label)
        , fTexture(std::move(texture)) {
    SkDEBUGCODE(id<MTLTexture> mtlTexture = fTexture->mtlTexture();)
    SkASSERT((GrMipmapStatus::kNotAllocated == mipmapStatus) == (1 == mtlTexture.mipmapLevelCount));
    if (@available(macOS 10.11, iOS 9.0, tvOS 9.0, *)) {
        SkASSERT(SkToBool(mtlTexture.usage & MTLTextureUsageShaderRead));
    }
    SkASSERT(!mtlTexture.framebufferOnly);
    if (ioType == kRead_GrIOType) {
        this->setReadOnly();
    }
    this->registerWithCacheWrapped(cacheable);
}

GrMtlTexture::GrMtlTexture(GrMtlGpu* gpu,
                           SkISize dimensions,
                           sk_sp<GrMtlAttachment> texture,
                           GrMipmapStatus mipmapStatus,
                           std::string_view label)
        : GrSurface(gpu, dimensions, GrProtected::kNo, label)
        , INHERITED(gpu, dimensions, GrProtected::kNo, GrTextureType::k2D, mipmapStatus, label)
        , fTexture(std::move(texture)) {
    SkDEBUGCODE(id<MTLTexture> mtlTexture = fTexture->mtlTexture();)
    SkASSERT((GrMipmapStatus::kNotAllocated == mipmapStatus) == (1 == mtlTexture.mipmapLevelCount));
    if (@available(macOS 10.11, iOS 9.0, tvOS 9.0, *)) {
        SkASSERT(SkToBool(mtlTexture.usage & MTLTextureUsageShaderRead));
    }
    SkASSERT(!mtlTexture.framebufferOnly);
}

sk_sp<GrMtlTexture> GrMtlTexture::MakeNewTexture(GrMtlGpu* gpu,
                                                 skgpu::Budgeted budgeted,
                                                 SkISize dimensions,
                                                 MTLPixelFormat format,
                                                 uint32_t mipLevels,
                                                 GrMipmapStatus mipmapStatus,
                                                 std::string_view label) {
    sk_sp<GrMtlAttachment> texture = GrMtlAttachment::MakeTexture(
            gpu, dimensions, format, mipLevels, GrRenderable::kNo, /*numSamples=*/1, budgeted);

    if (!texture) {
        return nullptr;
    }
    return sk_sp<GrMtlTexture>(new GrMtlTexture(gpu, budgeted, dimensions, std::move(texture),
                                                mipmapStatus, label));
}

sk_sp<GrMtlTexture> GrMtlTexture::MakeWrappedTexture(GrMtlGpu* gpu,
                                                     SkISize dimensions,
                                                     id<MTLTexture> texture,
                                                     GrWrapCacheable cacheable,
                                                     GrIOType ioType) {
    SkASSERT(nil != texture);
    if (@available(macOS 10.11, iOS 9.0, tvOS 9.0, *)) {
        SkASSERT(SkToBool(texture.usage & MTLTextureUsageShaderRead));
    }
    sk_sp<GrMtlAttachment> attachment =
            GrMtlAttachment::MakeWrapped(gpu, dimensions, texture,
                                         GrAttachment::UsageFlags::kTexture, cacheable,
                                         /*label=*/"MtlAttachment_MakeWrapped");
    if (!attachment) {
        return nullptr;
    }

    GrMipmapStatus mipmapStatus = texture.mipmapLevelCount > 1 ? GrMipmapStatus::kValid
                                                               : GrMipmapStatus::kNotAllocated;
    return sk_sp<GrMtlTexture>(
            new GrMtlTexture(gpu, kWrapped, dimensions, std::move(attachment), mipmapStatus,
                             cacheable, ioType, /*label=*/"MtlTextureWrappedTexture"));
}

GrMtlTexture::~GrMtlTexture() {
    SkASSERT(nil == fTexture);
}

GrMtlGpu* GrMtlTexture::getMtlGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrMtlGpu*>(this->getGpu());
}

GrBackendTexture GrMtlTexture::getBackendTexture() const {
    skgpu::Mipmapped mipmapped = fTexture->mtlTexture().mipmapLevelCount > 1
                                         ? skgpu::Mipmapped::kYes
                                         : skgpu::Mipmapped::kNo;
    GrMtlTextureInfo info;
    info.fTexture.reset(GrRetainPtrFromId(fTexture->mtlTexture()));
    return GrBackendTexture(this->width(), this->height(), mipmapped, info);
}

GrBackendFormat GrMtlTexture::backendFormat() const {
    return GrBackendFormat::MakeMtl(fTexture->mtlFormat());
}

void GrMtlTexture::onSetLabel() {
    SkASSERT(fTexture);
    if (!this->getLabel().empty()) {
        NSString* labelStr = @(this->getLabel().c_str());
        fTexture->mtlTexture().label = [@"_Skia_" stringByAppendingString:labelStr];
    }
}

GR_NORETAIN_END
