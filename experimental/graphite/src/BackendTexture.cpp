/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/include/BackendTexture.h"

namespace skgpu {

BackendTexture::~BackendTexture() {}

BackendTexture::BackendTexture(const BackendTexture& that) {
    *this = that;
}

BackendTexture& BackendTexture::operator=(const BackendTexture& that) {
    bool valid = this->isValid();
    if (!that.isValid()) {
        fInfo = {};
        return *this;
    } else if (valid && this->backend() != that.backend()) {
        valid = false;
    }
    fDimensions = that.fDimensions;
    fInfo = that.fInfo;

    switch (that.backend()) {
#ifdef SK_METAL
        case BackendApi::kMetal:
            fMtlTexture = that.fMtlTexture;
            break;
#endif
        default:
            SK_ABORT("Unsupport Backend");
    }
    return *this;
}

bool BackendTexture::operator==(const BackendTexture& that) const {
    if (!this->isValid() || !that.isValid()) {
        return false;
    }

    if (fDimensions != that.fDimensions || fInfo != that.fInfo) {
        return false;
    }

    switch (that.backend()) {
#ifdef SK_METAL
        case BackendApi::kMetal:
            if (fMtlTexture != that.fMtlTexture) {
                return false;
            }
            break;
#endif
        default:
            SK_ABORT("Unsupport Backend");
    }
    return true;
}

#ifdef SK_METAL
BackendTexture::BackendTexture(SkISize dimensions, mtl::Handle mtlTexture)
        : fDimensions(dimensions)
        , fInfo(mtl::TextureInfo(mtlTexture))
        , fMtlTexture(mtlTexture) {}

mtl::Handle BackendTexture::getMtlTexture() const {
    if (this->isValid() && this->backend() == BackendApi::kMetal) {
        return fMtlTexture;
    }
    return nullptr;
}

#endif

} // namespace skgpu

