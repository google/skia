/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/BackendTexture.h"

#include "include/gpu/MutableTextureState.h"
#include "src/gpu/graphite/BackendTexturePriv.h"

namespace skgpu::graphite {

BackendTexture::BackendTexture() = default;

BackendTexture::~BackendTexture() = default;

BackendTexture::BackendTexture(const BackendTexture& that) {
    *this = that;
}

static inline void assert_is_supported_backend(const BackendApi& backend) {
    SkASSERT(backend == BackendApi::kDawn ||
             backend == BackendApi::kMetal ||
             backend == BackendApi::kVulkan);
}

BackendTexture& BackendTexture::operator=(const BackendTexture& that) {
    if (!that.isValid()) {
        fInfo = {};
        return *this;
    }
    // We shouldn't be mixing backends.
    SkASSERT(!this->isValid() || this->backend() == that.backend());
    // If that was valid, it should have a supported backend.
    assert_is_supported_backend(that.backend());
    fDimensions = that.fDimensions;
    fInfo = that.fInfo;

    fTextureData.reset();
    that.fTextureData->copyTo(fTextureData);
    return *this;
}

bool BackendTexture::operator==(const BackendTexture& that) const {
    if (!this->isValid() || !that.isValid()) {
        return false;
    }

    if (fDimensions != that.fDimensions || fInfo != that.fInfo) {
        return false;
    }
    assert_is_supported_backend(this->backend());
    return fTextureData->equal(that.fTextureData.get());
}

BackendTextureData::~BackendTextureData(){};

} // namespace skgpu::graphite

