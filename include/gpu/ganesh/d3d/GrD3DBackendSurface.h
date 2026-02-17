/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrD3DBackendSurface_DEFINED
#define GrD3DBackendSurface_DEFINED

#include "include/private/base/SkAPI.h"
#include "include/private/gpu/ganesh/GrD3DTypesMinimal.h"

#include <string_view>

class GrBackendFormat;
class GrBackendTexture;
class GrBackendRenderTarget;

namespace GrBackendFormats {

SK_API GrBackendFormat MakeD3D(DXGI_FORMAT);
SK_API DXGI_FORMAT AsDxgiFormat(const GrBackendFormat&);

}  // namespace GrBackendFormats

namespace GrBackendTextures {

SK_API GrBackendTexture MakeD3D(int width,
                                int height,
                                const GrD3DTextureResourceInfo& d3dInfo,
                                std::string_view label = {});

// Return a snapshot of the GrD3DTextureResourceInfo struct.
// This snapshot will set the fResourceState to the current resource state.
SK_API GrD3DTextureResourceInfo GetD3DTextureResourceInfo(const GrBackendTexture&);

// Anytime the client changes the D3D12_RESOURCE_STATES of the D3D12_RESOURCE captured by this
// GrBackendTexture, they must call this function to notify Skia of the changed layout.
SK_API void SetD3DResourceState(GrBackendTexture*, GrD3DResourceStateEnum);

}  // namespace GrBackendTextures

namespace GrBackendRenderTargets {

SK_API GrBackendRenderTarget MakeD3D(int width,
                                     int height,
                                     const GrD3DTextureResourceInfo& d3dInfo);

// Return a snapshot of the GrD3DTextureResourceInfo struct.
// This snapshot will set the fResourceState to the current resource state.
SK_API GrD3DTextureResourceInfo GetD3DTextureResourceInfo(const GrBackendRenderTarget&);

// Anytime the client changes the D3D12_RESOURCE_STATES of the D3D12_RESOURCE captured by this
// GrBackendRenderTarget, they must call this function to notify Skia of the changed layout.
SK_API void SetD3DResourceState(GrBackendRenderTarget*, GrD3DResourceStateEnum);

}  // namespace GrBackendRenderTargets

#endif
