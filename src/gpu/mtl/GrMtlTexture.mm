/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlTexture.h"

#include "src/gpu/GrTexture.h"
#include "src/gpu/mtl/GrMtlGpu.h"
#include "src/gpu/mtl/GrMtlUtil.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

GrMtlTexture::GrMtlTexture(GrMtlGpu* gpu,
                           SkBudgeted budgeted,
                           SkISize dimensions,
                           id<MTLTexture> texture,
                           GrMipmapStatus mipmapStatus)
        : GrSurface(gpu, dimensions, GrProtected::kNo)
        , INHERITED(gpu, dimensions, GrProtected::kNo, GrTextureType::k2D, mipmapStatus)
        , fTexture(texture) {
    SkASSERT((GrMipmapStatus::kNotAllocated == mipmapStatus) == (1 == texture.mipmapLevelCount));
    if (@available(macOS 10.11, iOS 9.0, *)) {
        SkASSERT(SkToBool(texture.usage & MTLTextureUsageShaderRead));
    }
    SkASSERT(!texture.framebufferOnly);
    this->registerWithCache(budgeted);
    if (GrMtlFormatIsCompressed(texture.pixelFormat)) {
        this->setReadOnly();
    }
}

GrMtlTexture::GrMtlTexture(GrMtlGpu* gpu,
                           Wrapped,
                           SkISize dimensions,
                           id<MTLTexture> texture,
                           GrMipmapStatus mipmapStatus,
                           GrWrapCacheable cacheable,
                           GrIOType ioType)
        : GrSurface(gpu, dimensions, GrProtected::kNo)
        , INHERITED(gpu, dimensions, GrProtected::kNo, GrTextureType::k2D, mipmapStatus)
        , fTexture(texture) {
    SkASSERT((GrMipmapStatus::kNotAllocated == mipmapStatus) == (1 == texture.mipmapLevelCount));
    if (@available(macOS 10.11, iOS 9.0, *)) {
        SkASSERT(SkToBool(texture.usage & MTLTextureUsageShaderRead));
    }
    SkASSERT(!texture.framebufferOnly);
    if (ioType == kRead_GrIOType) {
        this->setReadOnly();
    }
    this->registerWithCacheWrapped(cacheable);
}

GrMtlTexture::GrMtlTexture(GrMtlGpu* gpu,
                           SkISize dimensions,
                           id<MTLTexture> texture,
                           GrMipmapStatus mipmapStatus)
        : GrSurface(gpu, dimensions, GrProtected::kNo)
        , INHERITED(gpu, dimensions, GrProtected::kNo, GrTextureType::k2D, mipmapStatus)
        , fTexture(texture) {
    SkASSERT((GrMipmapStatus::kNotAllocated == mipmapStatus) == (1 == texture.mipmapLevelCount));
    if (@available(macOS 10.11, iOS 9.0, *)) {
        SkASSERT(SkToBool(texture.usage & MTLTextureUsageShaderRead));
    }
    SkASSERT(!texture.framebufferOnly);
}

sk_sp<GrMtlTexture> GrMtlTexture::MakeNewTexture(GrMtlGpu* gpu,
                                                 SkBudgeted budgeted,
                                                 SkISize dimensions,
                                                 MTLTextureDescriptor* texDesc,
                                                 GrMipmapStatus mipmapStatus) {
    id<MTLTexture> texture = [gpu->device() newTextureWithDescriptor:texDesc];
    if (!texture) {
        return nullptr;
    }
    if (@available(macOS 10.11, iOS 9.0, *)) {
        SkASSERT(SkToBool(texture.usage & MTLTextureUsageShaderRead));
    }
    return sk_sp<GrMtlTexture>(new GrMtlTexture(gpu, budgeted, dimensions, texture, mipmapStatus));
}

sk_sp<GrMtlTexture> GrMtlTexture::MakeWrappedTexture(GrMtlGpu* gpu,
                                                     SkISize dimensions,
                                                     id<MTLTexture> texture,
                                                     GrWrapCacheable cacheable,
                                                     GrIOType ioType) {
    SkASSERT(nil != texture);
    if (@available(macOS 10.11, iOS 9.0, *)) {
        SkASSERT(SkToBool(texture.usage & MTLTextureUsageShaderRead));
    }
    GrMipmapStatus mipmapStatus = texture.mipmapLevelCount > 1 ? GrMipmapStatus::kValid
                                                               : GrMipmapStatus::kNotAllocated;
    return sk_sp<GrMtlTexture>(
            new GrMtlTexture(gpu, kWrapped, dimensions, texture, mipmapStatus, cacheable, ioType));
}

GrMtlTexture::~GrMtlTexture() {
    SkASSERT(nil == fTexture);
}

GrMtlGpu* GrMtlTexture::getMtlGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrMtlGpu*>(this->getGpu());
}

GrBackendTexture GrMtlTexture::getBackendTexture() const {
    GrMipmapped mipMapped = fTexture.mipmapLevelCount > 1 ? GrMipmapped::kYes
                                                          : GrMipmapped::kNo;
    GrMtlTextureInfo info;
    info.fTexture.reset(GrRetainPtrFromId(fTexture));
    return GrBackendTexture(this->width(), this->height(), mipMapped, info);
}

GrBackendFormat GrMtlTexture::backendFormat() const {
    return GrBackendFormat::MakeMtl(fTexture.pixelFormat);
}

