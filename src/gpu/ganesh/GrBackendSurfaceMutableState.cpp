/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrBackendSurfaceMutableState.h"

#include <new>

GrBackendSurfaceMutableState::GrBackendSurfaceMutableState(const GrBackendSurfaceMutableState& that)
        : fBackend(that.fBackend), fIsValid(that.fIsValid) {
    if (!fIsValid) {
        return;
    }
    switch (fBackend) {
        case GrBackend::kVulkan:
#ifdef SK_VULKAN
            SkASSERT(that.fBackend == GrBackend::kVulkan);
            fVkState = that.fVkState;
#endif
            break;
        default:
            (void)that;
            SkUNREACHABLE;
    }
}

GrBackendSurfaceMutableState& GrBackendSurfaceMutableState::operator=(
        const GrBackendSurfaceMutableState& that) {
    if (this != &that) {
        this->~GrBackendSurfaceMutableState();
        new (this) GrBackendSurfaceMutableState(that);
    }
    return *this;
}

