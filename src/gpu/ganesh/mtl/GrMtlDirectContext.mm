/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/gpu/ganesh/mtl/GrMtlDirectContext.h"

#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/mtl/GrMtlBackendContext.h"
#include "src/gpu/ganesh/GrContextThreadSafeProxyPriv.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/mtl/GrMtlTrampoline.h"

namespace GrDirectContexts {
sk_sp<GrDirectContext> MakeMetal(const GrMtlBackendContext& backendContext) {
    GrContextOptions defaultOptions;
    return MakeMetal(backendContext, defaultOptions);
}

sk_sp<GrDirectContext> MakeMetal(const GrMtlBackendContext& backendContext,
                                 const GrContextOptions& options) {
    sk_sp<GrDirectContext> direct = GrDirectContextPriv::Make(
            GrBackendApi::kMetal,
            options,
            GrContextThreadSafeProxyPriv::Make(GrBackendApi::kMetal, options));

    GrDirectContextPriv::SetGpu(direct,
                                GrMtlTrampoline::MakeGpu(backendContext, options, direct.get()));
    if (!GrDirectContextPriv::Init(direct)) {
        return nullptr;
    }

    return direct;
}
}  // namespace GrDirectContexts
