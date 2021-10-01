/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/mtl/MtlTexture.h"

#include "experimental/graphite/include/mtl/MtlTypes.h"

#import <Metal/Metal.h>

namespace skgpu::mtl {

sk_sp<Texture> Texture::Make(SkISize dimensions) {
    TextureInfo mtlInfo;
    mtlInfo.fSampleCount = 1;
    mtlInfo.fLevelCount = 1;

    mtlInfo.fFormat = MTLPixelFormatRGBA8Unorm;
    if (@available(macOS 10.11, ios 9.0, *)) {
        mtlInfo.fUsage = MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget;
        mtlInfo.fStorageMode = MTLStorageModePrivate;
    }

    skgpu::TextureInfo textureInfo(mtlInfo);

    UsageFlags usage = UsageFlags::kTexture | UsageFlags::kColorAttachment;

    return sk_sp<Texture>(new Texture(dimensions, textureInfo, usage));
}

} // namespace skgpu::mtl

