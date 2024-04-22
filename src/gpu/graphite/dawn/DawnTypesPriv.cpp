/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/gpu/graphite/DawnTypesPriv.h"

namespace skgpu::graphite {

SkString DawnTextureSpec::toString() const {
    return SkStringPrintf("format=0x%08X,viewFormat=0x%08X,usage=0x%08X,aspect=0x%08X,slice=%u",
                          static_cast<unsigned int>(fFormat),
                          static_cast<unsigned int>(fViewFormat),
                          static_cast<unsigned int>(fUsage),
                          static_cast<unsigned int>(fAspect),
                          fSlice);
}

DawnTextureInfo DawnTextureInfoFromWGPUTexture(WGPUTexture texture) {
    SkASSERT(texture);
    return DawnTextureInfo(
            wgpuTextureGetSampleCount(texture),
            wgpuTextureGetMipLevelCount(texture) > 1 ? Mipmapped::kYes : Mipmapped::kNo,
            /*format=*/static_cast<wgpu::TextureFormat>(wgpuTextureGetFormat(texture)),
            /*viewFormat=*/static_cast<wgpu::TextureFormat>(wgpuTextureGetFormat(texture)),
            static_cast<wgpu::TextureUsage>(wgpuTextureGetUsage(texture)),
            wgpu::TextureAspect::All, /*slice=*/0);
}

DawnTextureInfo DawnTextureSpecToTextureInfo(const DawnTextureSpec& dawnSpec,
                                             uint32_t sampleCount,
                                             Mipmapped mipmapped) {
    DawnTextureInfo info;
    // Shared info
    info.fSampleCount = sampleCount;
    info.fMipmapped = mipmapped;

    // Dawn info
    info.fFormat = dawnSpec.fFormat;
    info.fViewFormat = dawnSpec.fViewFormat;
    info.fUsage = dawnSpec.fUsage;
    info.fAspect = dawnSpec.fAspect;

    return info;
}

}  // namespace skgpu::graphite
