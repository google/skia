/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_MtlTexture_DEFINED
#define skgpu_MtlTexture_DEFINED

#include "experimental/graphite/src/Texture.h"

#import <Metal/Metal.h>

namespace skgpu::mtl {

class Gpu;

class Texture : public skgpu::Texture {
public:
    static sk_sp<Texture> MakeSampledTexture(Gpu* gpu,
                                             SkISize dimensions,
                                             UsageFlags usage,
                                             uint32_t mipLevels,
                                             MTLPixelFormat format);

    static sk_sp<Texture> MakeMSAA(Gpu* gpu,
                                   SkISize dimensions,
                                   int sampleCnt,
                                   MTLPixelFormat format);

    static sk_sp<Texture> MakeDepthStencil(Gpu* gpu,
                                           SkISize dimensions,
                                           UsageFlags usage, // Must only be depth and/or stencil
                                           int sampleCnt,
                                           MTLPixelFormat format);

    ~Texture() override {}

    id<MTLTexture> mtlTexture() const { return fTexture.get(); }

private:
    static sk_sp<Texture> Make(Gpu* gpu,
                               SkISize dimensions,
                               UsageFlags usages,
                               int sampleCnt,
                               MTLPixelFormat format,
                               uint32_t mipLevels,
                               int mtlTextureUsage,
                               int mtlStorageMode);

    Texture(SkISize dimensions,
            const skgpu::TextureInfo& info,
            UsageFlags supportedUsages,
            sk_cfp<id<MTLTexture>> texture);

    sk_cfp<id<MTLTexture>> fTexture;
};

} // namepsace skgpu::mtl

#endif // skgpu_MtlTexture_DEFINED
