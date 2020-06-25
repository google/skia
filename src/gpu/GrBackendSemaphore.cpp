/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrBackendSemaphore.h"

#ifdef SK_DIRECT3D
#include "include/gpu/d3d/GrD3DTypes.h"

GrBackendSemaphore::~GrBackendSemaphore() {
    if (fIsInitialized && GrBackendApi::kDirect3D == fBackend) {
        delete fD3DFenceInfo;
        fD3DFenceInfo = nullptr;
        fIsInitialized = false;
    }
}

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

GrBackendSemaphore::GrBackendSemaphore(const GrBackendSemaphore& that) {
    fIsInitialized = false;
    *this = that;
}

GrBackendSemaphore& GrBackendSemaphore::operator=(const GrBackendSemaphore& that) {
    SkASSERT(!fIsInitialized || fBackend == that.fBackend);
    fBackend = that.fBackend;
    switch (that.fBackend) {
#ifdef SK_GL
    case GrBackendApi::kOpenGL:
        fGLSync = that.fGLSync;
        break;
#endif
#ifdef SK_VULKAN
    case GrBackendApi::kVulkan:
        fVkSemaphore = that.fVkSemaphore;
        break;
#endif
#ifdef SK_METAL
    case GrBackendApi::kMetal:
        fMtlEvent = that.fMtlEvent;
        fMtlValue = that.fMtlValue;
        break;
#endif
    case GrBackendApi::kDirect3D:
        this->assignD3DFenceInfo(*that.fD3DFenceInfo);
        break;
    default:
        SK_ABORT("Unknown GrBackend");
    }
    fIsInitialized = true;
    return *this;
}

#endif // SK_DIRECT3D
