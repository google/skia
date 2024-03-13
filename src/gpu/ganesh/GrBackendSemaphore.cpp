/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrBackendSemaphore.h"

#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "src/gpu/ganesh/GrBackendSemaphorePriv.h"

#ifdef SK_DIRECT3D
#include "include/gpu/d3d/GrD3DTypes.h"
#endif

GrBackendSemaphore::GrBackendSemaphore()
        : fBackend(GrBackendApi::kUnsupported), fIsInitialized(false) {}

GrBackendSemaphoreData::~GrBackendSemaphoreData() = default;

GrBackendSemaphore::~GrBackendSemaphore() {
#ifdef SK_DIRECT3D
    if (fIsInitialized && GrBackendApi::kDirect3D == fBackend) {
        delete fD3DFenceInfo;
        fD3DFenceInfo = nullptr;
        fIsInitialized = false;
    }
#endif
}

GrBackendSemaphore::GrBackendSemaphore(const GrBackendSemaphore& that) {
    fIsInitialized = false;
    *this = that;
}

GrBackendSemaphore& GrBackendSemaphore::operator=(const GrBackendSemaphore& that) {
    SkASSERT(!fIsInitialized || fBackend == that.fBackend);
    fBackend = that.fBackend;
    fSemaphoreData.reset();
    switch (that.fBackend) {
        case GrBackendApi::kOpenGL:
            SK_ABORT("Unsupported");
            break;
        case GrBackendApi::kVulkan:
        case GrBackendApi::kMetal:
            that.fSemaphoreData->copyTo(fSemaphoreData);
            break;
#ifdef SK_DIRECT3D
        case GrBackendApi::kDirect3D:
            this->assignD3DFenceInfo(*that.fD3DFenceInfo);
            break;
#endif
        default:
            SK_ABORT("Unknown GrBackend");
    }
    fIsInitialized = true;
    return *this;
}

#ifdef SK_DIRECT3D
void GrBackendSemaphore::assignD3DFenceInfo(const GrD3DFenceInfo& info) {
    SkASSERT(GrBackendApi::kDirect3D == fBackend);
    if (fIsInitialized) {
        *fD3DFenceInfo = info;
    } else {
        fD3DFenceInfo = new GrD3DFenceInfo(info);
    }
}

bool GrBackendSemaphore::getD3DFenceInfo(GrD3DFenceInfo* outInfo) const {
    if (fIsInitialized && GrBackendApi::kDirect3D == fBackend) {
        *outInfo = *fD3DFenceInfo;
        return true;
    }
    return false;
}
#endif // SK_DIRECT3D
