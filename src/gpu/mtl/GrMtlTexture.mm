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

sk_sp<GrMtlTexture> GrMtlTexture::CreateNewTexture(GrMtlGpu* gpu, SkBudgeted budgeted,
                                                   const GrSurfaceDesc& desc, int mipLevels) {
    MTLPixelFormat format;
    if (!GrPixelConfigToMTLFormat(desc.fConfig, &format)) {
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
    // Shared is not available on MacOS. Is there a reason to want managed to allow mapping?
    descriptor.storageMode = MTLStorageModePrivate;

    MTLTextureUsage texUsage = MTLTextureUsageShaderRead;
    if (GrMTLFormatIsSRGB(format, nullptr)) {
        texUsage |= MTLTextureUsagePixelFormatView;
    }
    descriptor.usage = texUsage;

    id<MTLTexture> texture = [gpu->device() newTextureWithDescriptor:descriptor];

    GrMipMapsStatus mipMapsStatus = mipLevels > 1 ? GrMipMapsStatus::kValid
                                                  : GrMipMapsStatus::kNotAllocated;

    return sk_sp<GrMtlTexture>(new GrMtlTexture(gpu, budgeted, desc, texture, mipMapsStatus));
}

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
}

GrMtlTexture::~GrMtlTexture() {
    SkASSERT(nil == fTexture);
}

GrMtlGpu* GrMtlTexture::getMtlGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrMtlGpu*>(this->getGpu());
}

GrBackendObject GrMtlTexture::getTextureHandle() const {
    void* voidTex = (__bridge_retained void*)fTexture;
    return (GrBackendObject)voidTex;
}

GrBackendTexture GrMtlTexture::getBackendTexture() const {
    return GrBackendTexture(); // invalid
}
