/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Texture.h"

namespace skgpu::graphite {

Texture::Texture(const Gpu* gpu, SkISize dimensions, const TextureInfo& info, Ownership ownership)
        : Resource(gpu, ownership)
        , fDimensions(dimensions)
        , fInfo(info) {}

Texture::~Texture() {}

} // namespace skgpu::graphite
