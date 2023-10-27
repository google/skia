/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/gpu/graphite/MtlGraphiteTypesPriv.h"

#import <Metal/Metal.h>

namespace skgpu::graphite {

MtlTextureInfo::MtlTextureInfo(CFTypeRef texture) {
    SkASSERT(texture);
    id<MTLTexture> mtlTex = (id<MTLTexture>)texture;

    fSampleCount = mtlTex.sampleCount;
    fMipmapped = mtlTex.mipmapLevelCount > 1 ? Mipmapped::kYes : Mipmapped::kNo;

    fFormat = mtlTex.pixelFormat;
    fUsage = mtlTex.usage;
    fStorageMode = mtlTex.storageMode;
    fFramebufferOnly = mtlTex.framebufferOnly;
}

MtlTextureInfo MtlTextureSpecToTextureInfo(const MtlTextureSpec& mtlSpec,
                                           uint32_t sampleCount,
                                           Mipmapped mipmapped) {
    MtlTextureInfo info;
    // Shared info
    info.fSampleCount = sampleCount;
    info.fMipmapped = mipmapped;

    // Mtl info
    info.fFormat = mtlSpec.fFormat;
    info.fUsage = mtlSpec.fUsage;
    info.fStorageMode = mtlSpec.fStorageMode;
    info.fFramebufferOnly = mtlSpec.fFramebufferOnly;

    return info;
}

}  // namespace skgpu::graphite
