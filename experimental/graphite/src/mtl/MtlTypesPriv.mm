/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/include/private/MtlTypesPriv.h"

namespace skgpu::mtl {

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

    return info;
}

}  // namespace skgpu::mtl
