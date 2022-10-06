/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnCaps.h"

#include "include/gpu/graphite/TextureInfo.h"

namespace skgpu::graphite {

DawnCaps::DawnCaps() : Caps() {}

DawnCaps::~DawnCaps() = default;

TextureInfo DawnCaps::getDefaultSampledTextureInfo(SkColorType,
                                                   uint32_t levelCount,
                                                   Protected,
                                                   Renderable) const {
    return {};
}

TextureInfo DawnCaps::getDefaultMSAATextureInfo(const TextureInfo& singleSampledInfo,
                                                Discardable discardable) const {
    return {};
}

TextureInfo DawnCaps::getDefaultDepthStencilTextureInfo(SkEnumBitMask<DepthStencilFlags>,
                                                        uint32_t sampleCount,
                                                        Protected) const {
    return {};
}

} // namespace skgpu::graphite

