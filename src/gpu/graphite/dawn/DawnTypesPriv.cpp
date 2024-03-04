/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/gpu/graphite/DawnTypesPriv.h"

#include "include/core/SkTypes.h"

namespace skgpu::graphite {

DawnTextureInfo::DawnTextureInfo(const wgpu::Texture& texture) {
    SkASSERT(texture);

    fSampleCount = texture.GetSampleCount();
    fMipmapped  = texture.GetMipLevelCount() > 1 ? Mipmapped::kYes : Mipmapped::kNo;

    fFormat = texture.GetFormat();
    fViewFormat = fFormat;
    fUsage = texture.GetUsage();
    fAspect = wgpu::TextureAspect::All;
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
