/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrSurfaceInfo.h"

#ifdef SK_DIRECT3D
#include "include/gpu/d3d/GrD3DTypes.h"
#endif

#ifdef SK_DIRECT3D
GrSurfaceInfo::GrSurfaceInfo(const GrD3DSurfaceInfo& d3dInfo)
        : fBackend(GrBackendApi::kDirect3D)
        , fValid(true)
        , fSampleCount(d3dInfo.fSampleCount)
        , fLevelCount(d3dInfo.fLevelCount)
        , fProtected(d3dInfo.fProtected)
        , fD3DSpec(d3dInfo) {}
#endif

GrSurfaceInfo::~GrSurfaceInfo() {
#ifdef SK_DIRECT3D
    if (this->isValid() && fBackend == GrBackendApi::kDirect3D) {
        fD3DSpec.cleanup();
    }
#endif
}

#ifdef SK_DIRECT3D
bool GrSurfaceInfo::getD3DSurfaceInfo(GrD3DSurfaceInfo* outSpec) const {
    if (!this->isValid() || fBackend != GrBackendApi::kDirect3D) {
        return false;
    }
    *outSpec = fD3DSpec.getSurfaceInfo(fSampleCount, fLevelCount, fProtected);
    return true;
}
#endif
