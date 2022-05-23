/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/mtl/MtlTexture.h"

#include "include/gpu/graphite/mtl/MtlTypes.h"
#include "include/private/gpu/graphite/MtlTypesPriv.h"
#include "src/gpu/graphite/mtl/MtlCaps.h"
#include "src/gpu/graphite/mtl/MtlGpu.h"
#include "src/gpu/graphite/mtl/MtlUtils.h"

namespace skgpu::graphite {

sk_cfp<id<MTLTexture>> MtlTexture::MakeMtlTexture(const MtlGpu* gpu,
                                                  SkISize dimensions,
                                                  const TextureInfo& info) {
    const skgpu::graphite::Caps* caps = gpu->caps();
    if (dimensions.width() > caps->maxTextureSize() ||
        dimensions.height() > caps->maxTextureSize()) {
        return nullptr;
    }

    const MtlTextureSpec& mtlSpec = info.mtlTextureSpec();
    SkASSERT(!mtlSpec.fFramebufferOnly);

    if (mtlSpec.fUsage & MTLTextureUsageShaderRead && !caps->isTexturable(info)) {
        return nullptr;
    }

    if (mtlSpec.fUsage & MTLTextureUsageRenderTarget &&
        !(caps->isRenderable(info) || MtlFormatIsDepthOrStencil((MTLPixelFormat)mtlSpec.fFormat))) {
        return nullptr;
    }

    sk_cfp<MTLTextureDescriptor*> desc([[MTLTextureDescriptor alloc] init]);
    (*desc).textureType = (info.numSamples() > 1) ? MTLTextureType2DMultisample : MTLTextureType2D;
    (*desc).pixelFormat = (MTLPixelFormat)mtlSpec.fFormat;
    (*desc).width = dimensions.width();
    (*desc).height = dimensions.height();
    (*desc).depth = 1;
    (*desc).mipmapLevelCount = info.numMipLevels();
    (*desc).sampleCount = info.numSamples();
    (*desc).arrayLength = 1;
    (*desc).usage = mtlSpec.fUsage;
    (*desc).storageMode = (MTLStorageMode)mtlSpec.fStorageMode;

    sk_cfp<id<MTLTexture>> texture([gpu->device() newTextureWithDescriptor:desc.get()]);
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    if (mtlSpec.fUsage & MTLTextureUsageRenderTarget) {
        if (MtlFormatIsDepthOrStencil((MTLPixelFormat)mtlSpec.fFormat)) {
            (*texture).label = @"DepthStencil";
        } else {
            if (info.numSamples() > 1) {
                if (mtlSpec.fUsage & MTLTextureUsageShaderRead) {
                    (*texture).label = @"MSAA SampledTexture-ColorAttachment";
                } else {
                    (*texture).label = @"MSAA ColorAttachment";
                }
            } else {
                if (mtlSpec.fUsage & MTLTextureUsageShaderRead) {
                    (*texture).label = @"SampledTexture-ColorAttachment";
                } else {
                    (*texture).label = @"ColorAttachment";
                }
            }
        }
    } else {
        SkASSERT(mtlSpec.fUsage & MTLTextureUsageShaderRead);
        (*texture).label = @"SampledTexture";
    }
#endif

    return texture;
}

MtlTexture::MtlTexture(const MtlGpu* gpu,
                       SkISize dimensions,
                       const TextureInfo& info,
                       sk_cfp<id<MTLTexture>> texture,
                       Ownership ownership,
                       SkBudgeted budgeted)
        : Texture(gpu, dimensions, info, ownership, budgeted)
        , fTexture(std::move(texture)) {}

sk_sp<Texture> MtlTexture::Make(const MtlGpu* gpu,
                                SkISize dimensions,
                                const TextureInfo& info,
                                SkBudgeted budgeted) {
    sk_cfp<id<MTLTexture>> texture = MakeMtlTexture(gpu, dimensions, info);
    if (!texture) {
        return nullptr;
    }
    return sk_sp<Texture>(new MtlTexture(gpu,
                                         dimensions,
                                         info,
                                         std::move(texture),
                                         Ownership::kOwned,
                                         budgeted));
}

sk_sp<Texture> MtlTexture::MakeWrapped(const MtlGpu* gpu,
                                       SkISize dimensions,
                                       const TextureInfo& info,
                                       sk_cfp<id<MTLTexture>> texture) {
    return sk_sp<Texture>(new MtlTexture(gpu,
                                         dimensions,
                                         info,
                                         std::move(texture),
                                         Ownership::kWrapped,
                                         SkBudgeted::kNo));
}

void MtlTexture::freeGpuData() {
    fTexture.reset();
}

} // namespace skgpu::graphite

