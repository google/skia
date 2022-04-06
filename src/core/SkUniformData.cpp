/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkUniformData.h"

#include <cstring>

sk_sp<SkUniformData> SkUniformData::Make(const char* data, size_t size) {
    // TODO: data should just be allocated right after UniformData in an arena
    char* newData = new char[size];
    memcpy(newData, data, size);
    return sk_sp<SkUniformData>(new SkUniformData(newData, size));
}

bool SkUniformData::operator==(const SkUniformData& other) const {
    if (this->dataSize() != other.dataSize()) {
        return false;
    }

    return !memcmp(this->data(), other.data(), this->dataSize());
}
