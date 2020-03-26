
/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DTypes_DEFINED
#define GrD3DTypes_DEFINED

#include <dxgiformat.h>

#include <functional>
#include "include/gpu/GrTypes.h"

struct ID3D12Resource;
typedef int GrD3DResourceStateEnum;

struct GrD3DTextureResourceInfo {
    ID3D12Resource*          fResource;
    GrD3DResourceStateEnum   fResourceState;
    DXGI_FORMAT              fFormat;
    uint32_t                 fLevelCount;
    GrProtected              fProtected;

    GrD3DTextureResourceInfo()
            : fResource(nullptr)
            , fResourceState(0) // D3D_RESOURCE_STATES_COMMON
            , fFormat(DXGI_FORMAT_UNKNOWN)
            , fLevelCount(0)
            , fProtected(GrProtected::kNo) {}

    GrD3DTextureResourceInfo(ID3D12Resource* resource,
                             GrD3DResourceStateEnum resourceState,
                             DXGI_FORMAT format,
                             uint32_t levelCount,
                             GrProtected isProtected = GrProtected::kNo)
            : fResource(resource)
            , fResourceState(resourceState)
            , fFormat(format)
            , fLevelCount(levelCount)
            , fProtected(isProtected) {}

    GrD3DTextureResourceInfo(const GrD3DTextureResourceInfo& info,
                             GrD3DResourceStateEnum resourceState)
            : fResource(info.fResource)
            , fResourceState(resourceState)
            , fFormat(info.fFormat)
            , fLevelCount(info.fLevelCount)
            , fProtected(info.fProtected) {}

    // This gives a way for a client to update the resource state of the texture if they change it
    // while we're still holding onto the wrapped texture. They will first need to get a handle
    // to our internal GrD3DTextureResourceInfo by calling getTextureHandle on a GrD3DTexture.
    void updateResourceState(GrD3DResourceStateEnum resourceState) {
        fResourceState = resourceState;
    }
#if GR_TEST_UTILS
    bool operator==(const GrD3DTextureResourceInfo& that) const {
        return fResource == that.fResource && fResourceState == that.fResourceState &&
               fFormat == that.fFormat && fLevelCount == that.fLevelCount &&
               fProtected == that.fProtected;
    }
#endif
};

#endif
