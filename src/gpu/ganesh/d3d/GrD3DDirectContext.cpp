/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/ganesh/d3d/GrD3DDirectContext.h"

#include "include/gpu/ganesh/GrDirectContext.h"
#include "src/gpu/ganesh/GrContextThreadSafeProxyPriv.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/d3d/GrD3DGpu.h"

namespace GrDirectContexts {

sk_sp<GrDirectContext> MakeD3D(const GrD3DBackendContext& backendContext) {
    GrContextOptions defaultOptions;
    return GrDirectContexts::MakeD3D(backendContext, defaultOptions);
}

sk_sp<GrDirectContext> MakeD3D(const GrD3DBackendContext& backendContext,
                               const GrContextOptions& options) {
    auto direct = GrDirectContextPriv::Make(
            GrBackendApi::kDirect3D,
            options,
            GrContextThreadSafeProxyPriv::Make(GrBackendApi::kDirect3D, options));

    GrDirectContextPriv::SetGpu(direct, GrD3DGpu::Make(backendContext, options, direct.get()));
    if (!GrDirectContextPriv::Init(direct)) {
        return nullptr;
    }

    return direct;
}

}  // namespace GrDirectContexts
