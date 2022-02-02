/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkUniformData.h"

#include "src/core/SkOpts.h"

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

////////////////////////////////////////////////////////////////////////////////////////////////////
void SkUniformBlock::add(sk_sp<SkUniformData> uniforms) {
    fUniformData.push_back(std::move(uniforms));
}

size_t SkUniformBlock::totalSize() const {
    size_t total = 0;

    // TODO: It seems like we need to worry about alignment between the separate sets of uniforms
    for (auto& u : fUniformData) {
        total += u->dataSize();
    }

    return total;
}

int SkUniformBlock::count() const {
    int total = 0;

    for (auto& u : fUniformData) {
        total += u->count();
    }

    return total;
}

bool SkUniformBlock::operator==(const SkUniformBlock& other) const {
    if (fUniformData.size() != other.fUniformData.size()) {
        return false;
    }

    for (size_t i = 0; i < fUniformData.size(); ++i) {
        if (*fUniformData[i] != *other.fUniformData[i]) {
            return false;
        }
    }

    return true;
}

size_t SkUniformBlock::hash() const {
    int32_t hash = 0;

    for (auto& u : fUniformData) {
        hash = SkOpts::hash_fn(u->data(), u->dataSize(), hash);
    }

    return hash;
}
