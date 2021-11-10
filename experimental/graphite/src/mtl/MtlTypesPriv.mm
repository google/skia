/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/include/private/MtlTypesPriv.h"

#import <Metal/Metal.h>

namespace skgpu::mtl {

TextureInfo::TextureInfo(Handle texture) {
    SkASSERT(texture);
    id<MTLTexture> mtlTex = (id<MTLTexture>)texture;

    fSampleCount = mtlTex.sampleCount;
    fLevelCount = mtlTex.mipmapLevelCount;

    fFormat = mtlTex.pixelFormat;
    fUsage = mtlTex.usage;
    fStorageMode = mtlTex.storageMode;
    fFramebufferOnly = mtlTex.framebufferOnly;
}

TextureInfo TextureSpecToTextureInfo(const TextureSpec& mtlSpec,
                                     uint32_t sampleCount,
                                     uint32_t levelCount) {
    TextureInfo info;
    // Shared info
    info.fSampleCount = sampleCount;
    info.fLevelCount = levelCount;

    // Mtl info
    info.fFormat = mtlSpec.fFormat;
    info.fUsage = mtlSpec.fUsage;
    info.fStorageMode = mtlSpec.fStorageMode;
    info.fFramebufferOnly = mtlSpec.fFramebufferOnly;

    return info;
}

}  // namespace skgpu::mtl
