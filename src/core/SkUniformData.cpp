/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkUniformData.h"

#include <cstring>

sk_sp<SkUniformData> SkUniformData::Make(SkSpan<const SkUniform> uniforms, size_t dataSize) {
    // TODO: the offsets and data should just be allocated right after UniformData in an arena
    uint32_t* offsets = new uint32_t[uniforms.size()];
    char* data = new char[dataSize];

    return sk_sp<SkUniformData>(new SkUniformData(uniforms, offsets, data, dataSize));
}

bool SkUniformData::operator==(const SkUniformData& other) const {
    if (this->uniforms().size() != other.uniforms().size() ||
        this->dataSize() != other.dataSize()) {
        return false;
    }

    return !memcmp(this->uniforms().data(), other.uniforms().data(),
                   this->uniforms().size_bytes()) &&
           !memcmp(this->data(), other.data(), this->dataSize()) &&
           !memcmp(this->offsets(), other.offsets(), this->count()*sizeof(uint32_t));
}
