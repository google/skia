/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/d3d/GrD3DTypes.h"
#include "include/private/GrD3DTypesPriv.h"

#include "src/gpu/d3d/GrD3DResourceState.h"

GrD3DBackendSurfaceInfo::GrD3DBackendSurfaceInfo(const GrD3DTextureResourceInfo& info,
                                                 GrD3DResourceState* state)
    : fTextureResourceInfo(new GrD3DTextureResourceInfo(info))
    , fResourceState(state) {}

void GrD3DBackendSurfaceInfo::cleanup() {
    SkSafeUnref(fResourceState);
    fResourceState = nullptr;
};

void GrD3DBackendSurfaceInfo::assign(const GrD3DBackendSurfaceInfo& that, bool isThisValid) {
    fTextureResourceInfo.reset(new GrD3DTextureResourceInfo(*that.fTextureResourceInfo));
    GrD3DResourceState* oldLayout = fResourceState;
    fResourceState = SkSafeRef(that.fResourceState);
    if (isThisValid) {
        SkSafeUnref(oldLayout);
    }
}

void GrD3DBackendSurfaceInfo::setResourceState(GrD3DResourceStateEnum resourceState) {
    SkASSERT(fResourceState);
    fResourceState->setResourceState(static_cast<D3D12_RESOURCE_STATES>(resourceState));
}

sk_sp<GrD3DResourceState> GrD3DBackendSurfaceInfo::getGrD3DResourceState() const {
    SkASSERT(fResourceState);
    return sk_ref_sp(fResourceState);
}

GrD3DTextureResourceInfo GrD3DBackendSurfaceInfo::snapTextureResourceInfo() const {
    return GrD3DTextureResourceInfo(*fTextureResourceInfo, fResourceState->getResourceState());
}

bool GrD3DBackendSurfaceInfo::isProtected() const {
    return fTextureResourceInfo->fProtected == GrProtected::kYes;
}

#if GR_TEST_UTILS
bool GrD3DBackendSurfaceInfo::operator==(const GrD3DBackendSurfaceInfo& that) const {
    GrD3DTextureResourceInfo cpyInfoThis = *fTextureResourceInfo;
    GrD3DTextureResourceInfo cpyInfoThat = *that.fTextureResourceInfo;
    // We don't care about the fResourceState here since we require they use the same
    // GrD3DResourceState.
    cpyInfoThis.fResourceState = D3D12_RESOURCE_STATE_COMMON;
    cpyInfoThat.fResourceState = D3D12_RESOURCE_STATE_COMMON;
    return cpyInfoThis == cpyInfoThat && fResourceState == that.fResourceState;
}
#endif
