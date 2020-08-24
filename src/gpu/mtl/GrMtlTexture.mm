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

GrMtlTexture::GrMtlTexture(GrMtlGpu* gpu,
                           SkBudgeted budgeted,
                           SkISize dimensions,
                           sk_cf_obj<id<MTLTexture>> texture,
                           GrMipmapStatus mipmapStatus)
        : GrSurface(gpu, dimensions, GrProtected::kNo)
        , INHERITED(gpu, dimensions, GrProtected::kNo, GrTextureType::k2D, mipmapStatus)
        , fTexture(std::move(texture)) {
    SkASSERT((GrMipmapStatus::kNotAllocated == mipmapStatus) ==
                     (1 == (*fTexture).mipmapLevelCount));
    if (@available(macOS 10.11, iOS 9.0, *)) {
        SkASSERT(SkToBool((*fTexture).usage & MTLTextureUsageShaderRead));
    }
    SkASSERT(!(*fTexture).framebufferOnly);
    this->registerWithCache(budgeted);
    if (GrMtlFormatIsCompressed((*fTexture).pixelFormat)) {
        this->setReadOnly();
    }
}

GrMtlTexture::GrMtlTexture(GrMtlGpu* gpu,
                           Wrapped,
                           SkISize dimensions,
                           sk_cf_obj<id<MTLTexture>> texture,
                           GrMipmapStatus mipmapStatus,
                           GrWrapCacheable cacheable,
                           GrIOType ioType)
        : GrSurface(gpu, dimensions, GrProtected::kNo)
        , INHERITED(gpu, dimensions, GrProtected::kNo, GrTextureType::k2D, mipmapStatus)
        , fTexture(std::move(texture)) {
    SkASSERT((GrMipmapStatus::kNotAllocated == mipmapStatus) ==
                     (1 == (*fTexture).mipmapLevelCount));
    if (@available(macOS 10.11, iOS 9.0, *)) {
        SkASSERT(SkToBool((*fTexture).usage & MTLTextureUsageShaderRead));
    }
    SkASSERT(!(*fTexture).framebufferOnly);
    if (ioType == kRead_GrIOType) {
        this->setReadOnly();
    }
    this->registerWithCacheWrapped(cacheable);
}

GrMtlTexture::GrMtlTexture(GrMtlGpu* gpu,
                           SkISize dimensions,
                           sk_cf_obj<id<MTLTexture>> texture,
                           GrMipmapStatus mipmapStatus)
        : GrSurface(gpu, dimensions, GrProtected::kNo)
        , INHERITED(gpu, dimensions, GrProtected::kNo, GrTextureType::k2D, mipmapStatus)
        , fTexture(std::move(texture)) {
    SkASSERT((GrMipmapStatus::kNotAllocated == mipmapStatus) ==
                     (1 == (*fTexture).mipmapLevelCount));
    if (@available(macOS 10.11, iOS 9.0, *)) {
        SkASSERT(SkToBool((*fTexture).usage & MTLTextureUsageShaderRead));
    }
    SkASSERT(!(*fTexture).framebufferOnly);
}

sk_sp<GrMtlTexture> GrMtlTexture::MakeNewTexture(GrMtlGpu* gpu,
                                                 SkBudgeted budgeted,
                                                 SkISize dimensions,
                                                 MTLTextureDescriptor* texDesc,
                                                 GrMipmapStatus mipmapStatus) {
    sk_cf_obj<id<MTLTexture>> texture([gpu->device() newTextureWithDescriptor:texDesc]);
    if (!texture) {
        return nullptr;
    }
    if (@available(macOS 10.11, iOS 9.0, *)) {
        SkASSERT(SkToBool((*texture).usage & MTLTextureUsageShaderRead));
    }
    return sk_sp<GrMtlTexture>(new GrMtlTexture(gpu, budgeted, dimensions, std::move(texture),
                                                mipmapStatus));
}

sk_sp<GrMtlTexture> GrMtlTexture::MakeWrappedTexture(GrMtlGpu* gpu,
                                                     SkISize dimensions,
                                                     sk_cf_obj<id<MTLTexture>> texture,
                                                     GrWrapCacheable cacheable,
                                                     GrIOType ioType) {
    SkASSERT(texture);
    if (@available(macOS 10.11, iOS 9.0, *)) {
        SkASSERT(SkToBool((*texture).usage & MTLTextureUsageShaderRead));
    }
    GrMipmapStatus mipmapStatus = (*texture).mipmapLevelCount > 1 ? GrMipmapStatus::kValid
                                                                  : GrMipmapStatus::kNotAllocated;
    return sk_sp<GrMtlTexture>(
            new GrMtlTexture(gpu, kWrapped, dimensions, std::move(texture), mipmapStatus, cacheable,
                             ioType));
}

GrMtlTexture::~GrMtlTexture() {
    SkASSERT(nil == fTexture);
}

GrMtlGpu* GrMtlTexture::getMtlGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrMtlGpu*>(this->getGpu());
}

GrBackendTexture GrMtlTexture::getBackendTexture() const {
    GrMipmapped mipMapped = (*fTexture).mipmapLevelCount > 1 ? GrMipmapped::kYes
                                                             : GrMipmapped::kNo;
    GrMtlTextureInfo info;
    info.fTexture.retain(fTexture.get());
    return GrBackendTexture(this->width(), this->height(), mipMapped, info);
}

GrBackendFormat GrMtlTexture::backendFormat() const {
    return GrBackendFormat::MakeMtl((*fTexture).pixelFormat);
}
