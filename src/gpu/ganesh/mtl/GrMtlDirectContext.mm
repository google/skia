/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/gpu/ganesh/mtl/GrMtlDirectContext.h"

#include "include/gpu/GrDirectContext.h"
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

#if !defined(SK_DISABLE_LEGACY_METAL_GRDIRECTCONTEXT_FACTORIES)
/*************************************************************************************************/
sk_sp<GrDirectContext> GrDirectContext::MakeMetal(const GrMtlBackendContext& backendContext) {
    return GrDirectContexts::MakeMetal(backendContext);
}

sk_sp<GrDirectContext> GrDirectContext::MakeMetal(const GrMtlBackendContext& backendContext,
                                                  const GrContextOptions& options) {
    return GrDirectContexts::MakeMetal(backendContext, options);
}

sk_sp<GrDirectContext> GrDirectContext::MakeMetal(void* device, void* queue) {
    GrContextOptions defaultOptions;
    return MakeMetal(device, queue, defaultOptions);
}

sk_sp<GrDirectContext> GrDirectContext::MakeMetal(void* device,
                                                  void* queue,
                                                  const GrContextOptions& options) {
    GrMtlBackendContext backendContext = {};
    backendContext.fDevice.reset(device);
    backendContext.fQueue.reset(queue);

    return GrDirectContexts::MakeMetal(backendContext, options);
}
#endif
