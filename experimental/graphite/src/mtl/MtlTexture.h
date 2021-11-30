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
    static sk_cfp<id<MTLTexture>> MakeMtlTexture(const Gpu*,
                                                 SkISize dimensions,
                                                 const skgpu::TextureInfo&);

    static sk_sp<Texture> Make(const Gpu* gpu,
                               SkISize dimensions,
                               const skgpu::TextureInfo&);

    static sk_sp<Texture> MakeWrapped(SkISize dimensions,
                                      const skgpu::TextureInfo&,
                                      sk_cfp<id<MTLTexture>>);

    ~Texture() override {}

    id<MTLTexture> mtlTexture() const { return fTexture.get(); }

private:
    Texture(SkISize dimensions,
            const skgpu::TextureInfo& info,
            sk_cfp<id<MTLTexture>>,
            Ownership);

    sk_cfp<id<MTLTexture>> fTexture;
};

} // namepsace skgpu::mtl

#endif // skgpu_MtlTexture_DEFINED
