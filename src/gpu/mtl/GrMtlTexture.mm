/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrMtlTexture.h"

#include "GrMtlGpu.h"
#include "GrMtlUtil.h"
#include "GrTexturePriv.h"

// This method parallels GrTextureProxy::highestFilterMode
static inline GrSamplerState::Filter highest_filter_mode(GrPixelConfig config) {
    return GrSamplerState::Filter::kMipMap;
}

GrMtlTexture::GrMtlTexture(GrMtlGpu* gpu,
                           SkBudgeted budgeted,
                           const GrSurfaceDesc& desc,
                           id<MTLTexture> texture,
                           GrMipMapsStatus mipMapsStatus)
        : GrSurface(gpu, desc)
        , INHERITED(gpu, desc, kTexture2DSampler_GrSLType, highest_filter_mode(desc.fConfig),
                    mipMapsStatus)
        , fTexture(texture) {
    SkASSERT((GrMipMapsStatus::kNotAllocated == mipMapsStatus) == (1 == texture.mipmapLevelCount));
    this->registerWithCache(budgeted);
}

GrMtlTexture::GrMtlTexture(GrMtlGpu* gpu,
                           Wrapped,
                           const GrSurfaceDesc& desc,
                           id<MTLTexture> texture,
                           GrMipMapsStatus mipMapsStatus)
        : GrSurface(gpu, desc)
        , INHERITED(gpu, desc, kTexture2DSampler_GrSLType, highest_filter_mode(desc.fConfig),
                    mipMapsStatus)
        , fTexture(texture) {
    SkASSERT((GrMipMapsStatus::kNotAllocated == mipMapsStatus) == (1 == texture.mipmapLevelCount));
    this->registerWithCacheWrapped();
}

GrMtlTexture::GrMtlTexture(GrMtlGpu* gpu,
                           const GrSurfaceDesc& desc,
                           id<MTLTexture> texture,
                           GrMipMapsStatus mipMapsStatus)
        : GrSurface(gpu, desc)
        , INHERITED(gpu, desc, kTexture2DSampler_GrSLType, highest_filter_mode(desc.fConfig),
                    mipMapsStatus)
        , fTexture(texture) {
    SkASSERT((GrMipMapsStatus::kNotAllocated == mipMapsStatus) == (1 == texture.mipmapLevelCount));
}

sk_sp<GrMtlTexture> GrMtlTexture::CreateNewTexture(GrMtlGpu* gpu, SkBudgeted budgeted,
                                                   const GrSurfaceDesc& desc, int mipLevels) {
    MTLPixelFormat format;
    if (!GrPixelConfigToMTLFormat(desc.fConfig, &format)) {
        return nullptr;
    }

    if (desc.fSampleCnt > 1) {
        SkASSERT(false); // Currently we don't support msaa
        return nullptr;
    }

    MTLTextureDescriptor* descriptor = [[MTLTextureDescriptor alloc] init];
    descriptor.textureType = MTLTextureType2D;
    descriptor.pixelFormat = format;
    descriptor.width = desc.fWidth;
    descriptor.height = desc.fHeight;
    descriptor.depth = 1;
    descriptor.mipmapLevelCount = mipLevels;
    descriptor.sampleCount = 1;
    descriptor.arrayLength = 1;
    // descriptor.resourceOptions This looks to be set by setting cpuCacheMode and storageModes
    descriptor.cpuCacheMode = MTLCPUCacheModeWriteCombined;
    // Make all textures have private gpu only access. We can use transfer buffers to copy to them.
    descriptor.storageMode = MTLStorageModePrivate;
    descriptor.usage = MTLTextureUsageShaderRead;

    id<MTLTexture> texture = [gpu->device() newTextureWithDescriptor:descriptor];

    GrMipMapsStatus mipMapsStatus = mipLevels > 1 ? GrMipMapsStatus::kValid
                                                  : GrMipMapsStatus::kNotAllocated;

    return sk_sp<GrMtlTexture>(new GrMtlTexture(gpu, budgeted, desc, texture, mipMapsStatus));
}

sk_sp<GrMtlTexture> GrMtlTexture::MakeWrappedTexture(GrMtlGpu* gpu,
                                                     const GrSurfaceDesc& desc,
                                                     GrWrapOwnership wrapOwnership,
                                                     const GrMtlTextureInfo& textureInfo) {
    SkASSERT(nullptr != textureInfo.fTexture);

    GrMipMapsStatus mipMapsStatus = textureInfo.fLevelCount > 1 ? GrMipMapsStatus::kValid
                                                                : GrMipMapsStatus::kNotAllocated;

    id<MTLTexture> texture = TransferTexture(textureInfo.fTexture, wrapOwnership);
    SkASSERT(MTLTextureUsageShaderRead & texture.usage);
    return sk_sp<GrMtlTexture>(new GrMtlTexture(gpu, kWrapped, desc, texture, mipMapsStatus));
}

GrMtlTexture::~GrMtlTexture() {
    SkASSERT(nil == fTexture);
}

GrMtlGpu* GrMtlTexture::getMtlGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrMtlGpu*>(this->getGpu());
}

GrBackendTexture GrMtlTexture::getBackendTexture() const {
    GrMtlTextureInfo textureInfo;
    ExtractMTLTextureInfo(fTexture, &textureInfo);
    return GrBackendTexture(textureInfo);
}
