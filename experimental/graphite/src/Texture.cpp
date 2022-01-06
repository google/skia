/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/Texture.h"

namespace skgpu {

Texture::Texture(const Gpu* gpu, SkISize dimensions, const TextureInfo& info, Ownership ownership)
        : Resource(gpu), fDimensions(dimensions), fInfo(info), fOwnership(ownership) {}

Texture::~Texture() {}

} // namespace skgpu
