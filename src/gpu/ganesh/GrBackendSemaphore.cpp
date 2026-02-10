/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/ganesh/GrBackendSemaphore.h"

#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "src/gpu/ganesh/GrBackendSemaphorePriv.h"

GrBackendSemaphore::GrBackendSemaphore() = default;

GrBackendSemaphoreData::~GrBackendSemaphoreData() = default;

GrBackendSemaphore::~GrBackendSemaphore() = default;

GrBackendSemaphore::GrBackendSemaphore(const GrBackendSemaphore& that) { *this = that; }

GrBackendSemaphore& GrBackendSemaphore::operator=(const GrBackendSemaphore& that) {
    SkASSERT(fBackend == GrBackendApi::kUnsupported || fBackend == that.fBackend);
    fBackend = that.fBackend;
    fSemaphoreData.reset();
    switch (that.fBackend) {
        case GrBackendApi::kOpenGL:
            SK_ABORT("Unsupported");
            break;
        case GrBackendApi::kVulkan:
        case GrBackendApi::kMetal:
        case GrBackendApi::kDirect3D:
            that.fSemaphoreData->copyTo(fSemaphoreData);
            break;
        default:
            SK_ABORT("Unknown GrBackend");
    }
    return *this;
}

