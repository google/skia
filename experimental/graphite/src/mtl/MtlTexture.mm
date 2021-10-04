/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/mtl/MtlTexture.h"

#include "experimental/graphite/include/mtl/MtlTypes.h"
#include "experimental/graphite/src/mtl/MtlGpu.h"

namespace skgpu::mtl {

Texture::Texture(SkISize dimensions,
                 const skgpu::TextureInfo& info,
                 UsageFlags supportedUsages,
                 sk_cfp<id<MTLTexture>> texture)
        : skgpu::Texture(dimensions, info, supportedUsages)
        , fTexture(std::move(texture)) {}

sk_sp<Texture> Texture::MakeSampledTexture(Gpu* gpu,
                                           SkISize dimensions,
                                           UsageFlags usage,
                                           uint32_t mipLevels,
                                           MTLPixelFormat format) {
    SkASSERT(usage == UsageFlags::kColorAttachment ||
             usage == UsageFlags::kSampledTexture ||
             usage == (UsageFlags::kColorAttachment | UsageFlags::kSampledTexture));

    int textureUsage = 0;
    int storageMode = 0;
    if (@available(macOS 10.11, iOS 9.0, *)) {
        textureUsage = MTLTextureUsageShaderRead;
        if (usage & UsageFlags::kColorAttachment) {
            textureUsage |= MTLTextureUsageRenderTarget;
        }
        storageMode = MTLStorageModePrivate;
    }
    return Texture::Make(gpu,
                         dimensions,
                         usage,
                         /*sampleCnt=*/1,
                         format,
                         mipLevels,
                         textureUsage,
                         storageMode);
}

sk_sp<Texture> Texture::MakeMSAA(Gpu* gpu,
                                 SkISize dimensions,
                                 int sampleCnt,
                                 MTLPixelFormat format) {
    int textureUsage = 0;
    int storageMode = 0;
    if (@available(macOS 10.11, iOS 9.0, *)) {
        textureUsage |= MTLTextureUsageRenderTarget;
        storageMode = MTLStorageModePrivate;
    }
    return Texture::Make(gpu,
                         dimensions,
                         UsageFlags::kColorAttachment,
                         sampleCnt,
                         format,
                         /*mipLevels=*/1,
                         textureUsage,
                         storageMode);
}

sk_sp<Texture> Texture::MakeDepthStencil(Gpu* gpu,
                                         SkISize dimensions,
                                         UsageFlags usage,
                                         int sampleCnt,
                                         MTLPixelFormat format) {
    SkASSERT(usage == UsageFlags::kStencilAttachment ||
             usage == UsageFlags::kDepthAttachment ||
             usage == (UsageFlags::kStencilAttachment | UsageFlags::kDepthAttachment));

    int textureUsage = 0;
    int storageMode = 0;
    if (@available(macOS 10.11, iOS 9.0, *)) {
        textureUsage = MTLTextureUsageRenderTarget;
        storageMode = MTLStorageModePrivate;
    }
    return Texture::Make(gpu,
                         dimensions,
                         usage,
                         sampleCnt,
                         format,
                         /*mipLevels=*/1,
                         textureUsage,
                         storageMode);
}

sk_sp<Texture> Texture::Make(Gpu* gpu,
                             SkISize dimensions,
                             UsageFlags usages,
                             int sampleCnt,
                             MTLPixelFormat format,
                             uint32_t mipLevels,
                             int mtlTextureUsage,
                             int mtlStorageMode) {
    sk_cfp<MTLTextureDescriptor*> desc([[MTLTextureDescriptor alloc] init]);
    (*desc).textureType = (sampleCnt > 1) ? MTLTextureType2DMultisample : MTLTextureType2D;
    (*desc).pixelFormat = format;
    (*desc).width = dimensions.width();
    (*desc).height = dimensions.height();
    (*desc).depth = 1;
    (*desc).mipmapLevelCount = mipLevels;
    (*desc).sampleCount = sampleCnt;
    (*desc).arrayLength = 1;
    if (@available(macOS 10.11, iOS 9.0, *)) {
        (*desc).usage = mtlTextureUsage;
        (*desc).storageMode = (MTLStorageMode)mtlStorageMode;
    }
    sk_cfp<id<MTLTexture>> texture([gpu->device() newTextureWithDescriptor:desc.get()]);
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    if (usages & UsageFlags::kStencilAttachment ||
        usages & UsageFlags::kDepthAttachment) {
        (*texture).label = @"DepthStencil";
    } else if (SkToBool(usages & UsageFlags::kColorAttachment)) {
        if (sampleCnt > 1) {
            if (SkToBool(usages & UsageFlags::kSampledTexture)) {
                (*texture).label = @"MSAA SampledTexture-ColorAttachment";
            } else {
                (*texture).label = @"MSAA ColorAttachment";
            }
        } else {
            if (SkToBool(usages & UsageFlags::kSampledTexture)) {
                (*texture).label = @"SampledTexture-ColorAttachment";
            } else {
                (*texture).label = @"ColorAttachment";
            }
        }
    } else {
        SkASSERT(usages == UsageFlags::kSampledTexture);
        (*texture).label = @"SampledTexture";
    }
#endif

    TextureInfo mtlInfo;
    mtlInfo.fSampleCount = (*desc).mipmapLevelCount;
    mtlInfo.fLevelCount = (*desc).sampleCount;
    mtlInfo.fFormat = (*desc).pixelFormat;
    if (@available(macOS 10.11, ios 9.0, *)) {
        mtlInfo.fUsage = (*desc).usage;
        mtlInfo.fStorageMode = (*desc).storageMode;
    }

    skgpu::TextureInfo textureInfo(mtlInfo);

    return sk_sp<Texture>(new Texture(dimensions, textureInfo, usages, std::move(texture)));
}

} // namespace skgpu::mtl

