/*
 * Copyright 2023 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/gpu/MutableTextureState.h"

#include "include/gpu/GpuTypes.h"
#include "src/gpu/MutableTextureStatePriv.h"

namespace skgpu {

MutableTextureState::MutableTextureState():
    fBackend(BackendApi::kUnsupported),
    fIsValid(false) {}
MutableTextureState::~MutableTextureState() = default;

MutableTextureState::MutableTextureState(const MutableTextureState& that): fIsValid(false) {
    this->set(that);
}

MutableTextureState& MutableTextureState::operator=(const MutableTextureState& that) {
    if (this != &that) {
        this->set(that);
    }
    return *this;
}

void MutableTextureState::set(const MutableTextureState& that) {
    SkASSERT(!fIsValid || this->fBackend == that.fBackend);
    fIsValid = that.fIsValid;
    fBackend = that.fBackend;
    if (!fIsValid) {
        return;
    }
    fStateData.reset();
    switch (fBackend) {
        case BackendApi::kVulkan:
            that.fStateData->copyTo(fStateData);
            break;
        default:
            SK_ABORT("Unknown BackendApi");
    }
}

MutableTextureStateData::~MutableTextureStateData() = default;

}  // namespace skgpu
