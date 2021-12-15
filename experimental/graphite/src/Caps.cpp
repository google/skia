/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/Caps.h"

#include "experimental/graphite/include/TextureInfo.h"
#include "src/sksl/SkSLUtil.h"

namespace skgpu {

Caps::Caps() {}
Caps::~Caps() {}

bool Caps::isTexturable(const TextureInfo& info) const {
    if (info.numSamples() > 1) {
        return false;
    }
    return this->onIsTexturable(info);
}

bool Caps::areColorTypeAndTextureInfoCompatible(SkColorType type, const TextureInfo& info) const {
    if (type == kUnknown_SkColorType) {
        return false;
    }

    return this->onAreColorTypeAndTextureInfoCompatible(type, info);
}

} // namespace skgpu
