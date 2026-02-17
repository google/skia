/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DTypesMinimal_DEFINED
#define GrD3DTypesMinimal_DEFINED

// Minimal definitions of Direct3D types, without including d3d12.h

#include "include/core/SkRefCnt.h"

#include <dxgiformat.h>
#include <memory>

#include "include/gpu/ganesh/GrTypes.h"

struct ID3D12Resource;
class GrD3DResourceState;
typedef int GrD3DResourceStateEnum;
struct GrD3DSurfaceInfo;
struct GrD3DTextureResourceInfo;
struct GrD3DTextureResourceSpec;
struct GrD3DFenceInfo;

// This struct is to used to store the the actual information about the Direct3D backend image on
// GrBackendTexture and GrBackendRenderTarget. When a client calls getD3DTextureInfo on a
// GrBackendTexture/RenderTarget, we use the GrD3DBackendSurfaceInfo to create a snapshot
// GrD3DTextureResourceInfo object. Internally, this uses a ref count GrD3DResourceState object to
// track the current D3D12_RESOURCE_STATES which can be shared with an internal GrD3DTextureResource
// so that state updates can be seen by all users of the texture.
struct GrD3DBackendSurfaceInfo {
    GrD3DBackendSurfaceInfo(const GrD3DTextureResourceInfo& info, sk_sp<GrD3DResourceState> state);
    ~GrD3DBackendSurfaceInfo();

    GrD3DBackendSurfaceInfo(const GrD3DBackendSurfaceInfo&);
    GrD3DBackendSurfaceInfo& operator=(const GrD3DBackendSurfaceInfo&);

    GrD3DBackendSurfaceInfo(GrD3DBackendSurfaceInfo&&);
    GrD3DBackendSurfaceInfo& operator=(GrD3DBackendSurfaceInfo&&);

    void setResourceState(GrD3DResourceStateEnum state);

    sk_sp<GrD3DResourceState> getResourceState() const;

    GrD3DTextureResourceInfo snapTextureResourceInfo() const;

    bool isProtected() const;
#if defined(GPU_TEST_UTILS)
    bool operator==(const GrD3DBackendSurfaceInfo& that) const;
#endif

private:
    std::unique_ptr<GrD3DTextureResourceInfo> fTextureResourceInfo;
    sk_sp<GrD3DResourceState> fResourceState;
};

#endif
