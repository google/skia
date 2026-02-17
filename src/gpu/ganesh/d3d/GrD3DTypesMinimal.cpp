/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/gpu/ganesh/GrD3DTypesMinimal.h"

#include "include/gpu/ganesh/d3d/GrD3DTypes.h"
#include "src/gpu/ganesh/d3d/GrD3DResourceState.h"
#include "src/gpu/ganesh/d3d/GrD3DTypesPriv.h"

GrD3DBackendSurfaceInfo::GrD3DBackendSurfaceInfo(const GrD3DTextureResourceInfo& info,
                                                 sk_sp<GrD3DResourceState> state)
        : fTextureResourceInfo(new GrD3DTextureResourceInfo(info))
        , fResourceState(std::move(state)) {}

GrD3DBackendSurfaceInfo::~GrD3DBackendSurfaceInfo() = default;

GrD3DBackendSurfaceInfo::GrD3DBackendSurfaceInfo(const GrD3DBackendSurfaceInfo& that)
        : fTextureResourceInfo(new GrD3DTextureResourceInfo(*that.fTextureResourceInfo))
        , fResourceState(that.fResourceState) {}

GrD3DBackendSurfaceInfo& GrD3DBackendSurfaceInfo::operator=(const GrD3DBackendSurfaceInfo& that) {
    if (this != &that) {
        fTextureResourceInfo.reset(new GrD3DTextureResourceInfo(*that.fTextureResourceInfo));
        fResourceState = that.fResourceState;
    }
    return *this;
}

GrD3DBackendSurfaceInfo::GrD3DBackendSurfaceInfo(GrD3DBackendSurfaceInfo&&) = default;
GrD3DBackendSurfaceInfo& GrD3DBackendSurfaceInfo::operator=(GrD3DBackendSurfaceInfo&&) = default;

void GrD3DBackendSurfaceInfo::setResourceState(GrD3DResourceStateEnum resourceState) {
    SkASSERT(fResourceState);
    fResourceState->setResourceState(static_cast<D3D12_RESOURCE_STATES>(resourceState));
}

sk_sp<GrD3DResourceState> GrD3DBackendSurfaceInfo::getResourceState() const {
    SkASSERT(fResourceState);
    return fResourceState;
}

GrD3DTextureResourceInfo GrD3DBackendSurfaceInfo::snapTextureResourceInfo() const {
    return GrD3DTextureResourceInfo(
            *fTextureResourceInfo,
            static_cast<D3D12_RESOURCE_STATES>(fResourceState->getResourceState()));
}

bool GrD3DBackendSurfaceInfo::isProtected() const {
    SkASSERT(fTextureResourceInfo);
    return fTextureResourceInfo->fProtected == GrProtected::kYes;
}

#if defined(GPU_TEST_UTILS)
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
