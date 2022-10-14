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
    fLevelCount  = texture.GetMipLevelCount();

    fFormat = texture.GetFormat();
    fUsage =  texture.GetUsage();
}

DawnTextureInfo DawnTextureSpecToTextureInfo(const DawnTextureSpec& dawnSpec,
                                             uint32_t sampleCount,
                                             uint32_t levelCount) {
    DawnTextureInfo info;
    // Shared info
    info.fSampleCount = sampleCount;
    info.fLevelCount = levelCount;

    // Dawn info
    info.fFormat = dawnSpec.fFormat;
    info.fUsage = dawnSpec.fUsage;

    return info;
}

}  // namespace skgpu::graphite
