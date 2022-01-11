/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/include/TextureInfo.h"

namespace skgpu {

TextureInfo& TextureInfo::operator=(const TextureInfo& that) {
    if (!that.isValid()) {
        fValid = false;
        return *this;
    }
    fBackend = that.fBackend;
    fSampleCount = that.fSampleCount;
    fLevelCount = that.fLevelCount;
    fProtected = that.fProtected;

    switch (that.backend()) {
#ifdef SK_METAL
        case BackendApi::kMetal:
            fMtlSpec = that.fMtlSpec;
            break;
#endif
        default:
            SK_ABORT("Unsupport Backend");
    }

    fValid = true;
    return *this;
}

bool TextureInfo::operator==(const TextureInfo& that) const {
    if (!this->isValid() && !that.isValid()) {
        return true;
    }
    if (!this->isValid() || !that.isValid()) {
        return false;
    }

    if (fBackend != that.fBackend) {
        return false;
    }

    if (fSampleCount != that.fSampleCount ||
        fLevelCount != that.fLevelCount ||
        fProtected != that.fProtected) {
        return false;
    }

    switch (fBackend) {
#ifdef SK_METAL
        case BackendApi::kMetal:
            return fMtlSpec == that.fMtlSpec;
#endif
        default:
            return false;
    }
}

} // namespace skgpu

