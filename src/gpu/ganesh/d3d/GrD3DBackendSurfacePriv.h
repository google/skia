/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DBackendSurfacePriv_DEFINED
#define GrD3DBackendSurfacePriv_DEFINED

#include "include/core/SkRefCnt.h"

class GrBackendTexture;
class GrBackendRenderTarget;
class GrD3DResourceState;

namespace GrBackendTextures {

GrBackendTexture MakeD3D(int width,
                         int height,
                         const GrD3DTextureResourceInfo& d3dInfo,
                         sk_sp<GrD3DResourceState> state,
                         std::string_view label = {});

sk_sp<GrD3DResourceState> GetD3DResourceState(const GrBackendTexture&);

}  // namespace GrBackendTextures

namespace GrBackendRenderTargets {

GrBackendRenderTarget MakeD3D(int width,
                              int height,
                              const GrD3DTextureResourceInfo& d3dInfo,
                              sk_sp<GrD3DResourceState> state);

sk_sp<GrD3DResourceState> GetD3DResourceState(const GrBackendRenderTarget&);

}  // namespace GrBackendRenderTargets

#endif
