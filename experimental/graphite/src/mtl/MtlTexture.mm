/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/mtl/MtlTexture.h"

#include "experimental/graphite/include/mtl/MtlTypes.h"
#include "experimental/graphite/include/private/MtlTypesPriv.h"
#include "experimental/graphite/src/mtl/MtlGpu.h"
#include "experimental/graphite/src/mtl/MtlUtils.h"

namespace skgpu::mtl {

Texture::Texture(SkISize dimensions,
                 const skgpu::TextureInfo& info,
                 sk_cfp<id<MTLTexture>> texture)
        : skgpu::Texture(dimensions, info)
        , fTexture(std::move(texture)) {}

sk_sp<Texture> Texture::Make(const Gpu* gpu,
                             SkISize dimensions,
                             const skgpu::TextureInfo& info) {
    const TextureSpec& mtlSpec = info.mtlTextureSpec();

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
        if (FormatIsDepthOrStencil((MTLPixelFormat)mtlSpec.fFormat)) {
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

    return sk_sp<Texture>(new Texture(dimensions, info, std::move(texture)));
}

} // namespace skgpu::mtl

