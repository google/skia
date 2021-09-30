/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_MtlTexture_DEFINED
#define skgpu_MtlTexture_DEFINED

#include "experimental/graphite/src/Texture.h"

namespace skgpu::mtl {

class Texture : public skgpu::Texture {
public:
    static sk_sp<Texture> Make(SkISize dimensions);

    ~Texture() override {}

private:
    Texture(SkISize dimensions, const skgpu::TextureInfo& info)
            : skgpu::Texture(dimensions, info) {}
};

} // namepsace skgpu::mtl

#endif // skgpu_MtlTexture_DEFINED
