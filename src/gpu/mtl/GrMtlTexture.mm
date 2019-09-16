/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlTexture.h"

#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/mtl/GrMtlGpu.h"
#include "src/gpu/mtl/GrMtlUtil.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

GrMtlTexture::GrMtlTexture(GrMtlGpu* gpu,
                           SkBudgeted budgeted,
                           const GrSurfaceDesc& desc,
                           id<MTLTexture> texture,
                           GrMipMapsStatus mipMapsStatus)
        : GrSurface(gpu, {desc.fWidth, desc.fHeight}, desc.fConfig, GrProtected::kNo)
        , INHERITED(gpu, {desc.fWidth, desc.fHeight}, desc.fConfig, GrProtected::kNo,
                    GrTextureType::k2D, mipMapsStatus)
        , fTexture(texture) {
    SkASSERT((GrMipMapsStatus::kNotAllocated == mipMapsStatus) == (1 == texture.mipmapLevelCount));
    this->registerWithCache(budgeted);
}

GrMtlTexture::GrMtlTexture(GrMtlGpu* gpu,
                           Wrapped,
                           const GrSurfaceDesc& desc,
                           id<MTLTexture> texture,
                           GrMipMapsStatus mipMapsStatus,
                           GrWrapCacheable cacheable,
                           GrIOType ioType)
        : GrSurface(gpu, {desc.fWidth, desc.fHeight}, desc.fConfig, GrProtected::kNo)
        , INHERITED(gpu, {desc.fWidth, desc.fHeight}, desc.fConfig, GrProtected::kNo,
                    GrTextureType::k2D, mipMapsStatus)
        , fTexture(texture) {
    SkASSERT((GrMipMapsStatus::kNotAllocated == mipMapsStatus) == (1 == texture.mipmapLevelCount));
    if (ioType == kRead_GrIOType) {
        this->setReadOnly();
    }
    this->registerWithCacheWrapped(cacheable);
}

GrMtlTexture::GrMtlTexture(GrMtlGpu* gpu,
                           const GrSurfaceDesc& desc,
                           id<MTLTexture> texture,
                           GrMipMapsStatus mipMapsStatus)
        : GrSurface(gpu, {desc.fWidth, desc.fHeight}, desc.fConfig, GrProtected::kNo)
        , INHERITED(gpu, {desc.fWidth, desc.fHeight}, desc.fConfig, GrProtected::kNo,
                    GrTextureType::k2D, mipMapsStatus)
        , fTexture(texture) {
    SkASSERT((GrMipMapsStatus::kNotAllocated == mipMapsStatus) == (1 == texture.mipmapLevelCount));
}

sk_sp<GrMtlTexture> GrMtlTexture::MakeNewTexture(GrMtlGpu* gpu, SkBudgeted budgeted,
                                                   const GrSurfaceDesc& desc,
                                                   MTLTextureDescriptor* texDesc,
                                                   GrMipMapsStatus mipMapsStatus) {
    id<MTLTexture> texture = [gpu->device() newTextureWithDescriptor:texDesc];
    if (!texture) {
        return nullptr;
    }
    SkASSERT(MTLTextureUsageShaderRead & texture.usage);
    return sk_sp<GrMtlTexture>(new GrMtlTexture(gpu, budgeted, desc, texture, mipMapsStatus));
}

sk_sp<GrMtlTexture> GrMtlTexture::MakeWrappedTexture(GrMtlGpu* gpu,
                                                     const GrSurfaceDesc& desc,
                                                     id<MTLTexture> texture,
                                                     GrWrapCacheable cacheable,
                                                     GrIOType ioType) {
    SkASSERT(nil != texture);
    SkASSERT(MTLTextureUsageShaderRead & texture.usage);
    GrMipMapsStatus mipMapsStatus = texture.mipmapLevelCount > 1 ? GrMipMapsStatus::kValid
                                                                 : GrMipMapsStatus::kNotAllocated;
    return sk_sp<GrMtlTexture>(new GrMtlTexture(gpu, kWrapped, desc, texture, mipMapsStatus,
                                                cacheable, ioType));
}

GrMtlTexture::~GrMtlTexture() {
    SkASSERT(nil == fTexture);
}

GrMtlGpu* GrMtlTexture::getMtlGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrMtlGpu*>(this->getGpu());
}

GrBackendTexture GrMtlTexture::getBackendTexture() const {
    GrMipMapped mipMapped = fTexture.mipmapLevelCount > 1 ? GrMipMapped::kYes
                                                          : GrMipMapped::kNo;
    GrMtlTextureInfo info;
    info.fTexture.reset(GrRetainPtrFromId(fTexture));
    return GrBackendTexture(this->width(), this->height(), mipMapped, info);
}

GrBackendFormat GrMtlTexture::backendFormat() const {
    return GrBackendFormat::MakeMtl(fTexture.pixelFormat);
}

