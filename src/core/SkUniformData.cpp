/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkUniformData.h"

sk_sp<SkUniformData> SkUniformData::Make(int count,
                                         const SkUniform* uniforms,
                                         size_t dataSize) {
    // TODO: the offsets and data should just be allocated right after UniformData in an arena
    uint32_t* offsets = new uint32_t[count];
    char* data = new char[dataSize];

    return sk_sp<SkUniformData>(new SkUniformData(count, uniforms, offsets, data, dataSize));
}
