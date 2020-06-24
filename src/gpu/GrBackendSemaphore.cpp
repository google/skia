/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrBackendSemaphore.h"

#include "include/gpu/d3d/GrD3DTypes.h"

void GrBackendSemaphore::initDirect3D(const GrD3DFenceInfo& info) {
    fBackend = GrBackendApi::kDirect3D;
    fD3DFenceInfo.assign(info, fIsInitialized);
    fIsInitialized = true;
}

bool GrBackendSemaphore::getD3DFenceInfo(GrD3DFenceInfo* outInfo) const {
    if (fIsInitialized && GrBackendApi::kDirect3D == fBackend) {
        *outInfo = fD3DFenceInfo.snapFenceInfo();
        return true;
    }
    return false;
}

GrBackendSemaphore& GrBackendSemaphore::operator=(const GrBackendSemaphore& that) {
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
#ifdef SK_DIRECT3D
    case GrBackendApi::kDirect3D:
        fD3DFenceInfo.assign(that.fD3DFenceInfo, fIsInitialized);
        break;
#endif
    default:
        SK_ABORT("Unknown GrBackend");
    }
    fIsInitialized = true;
    return *this;
}
