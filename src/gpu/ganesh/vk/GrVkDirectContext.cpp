/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/ganesh/vk/GrVkDirectContext.h"

#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrTypes.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/vk/GrVkGpu.h"

#include <memory>

namespace GrDirectContexts {
sk_sp<GrDirectContext> MakeVulkan(const GrVkBackendContext& backendContext) {
    GrContextOptions defaultOptions;
    return MakeVulkan(backendContext, defaultOptions);
}

sk_sp<GrDirectContext> MakeVulkan(const GrVkBackendContext& backendContext,
                                  const GrContextOptions& options) {
    auto direct = GrDirectContextPriv::Make(GrBackendApi::kVulkan, options);

    GrDirectContextPriv::SetGpu(direct,
                                GrVkGpu::Make(backendContext, options, direct.get()));
    if (!GrDirectContextPriv::Init(direct)) {
        return nullptr;
    }

    return direct;
}
}  // namespace GrDirectContexts

#if !defined(SK_DISABLE_LEGACY_VK_GRDIRECTCONTEXT_FACTORIES)

sk_sp<GrDirectContext> GrDirectContext::MakeVulkan(const GrVkBackendContext& backendContext) {
    GrContextOptions defaultOptions;
    return MakeVulkan(backendContext, defaultOptions);
}

sk_sp<GrDirectContext> GrDirectContext::MakeVulkan(const GrVkBackendContext& backendContext,
                                                   const GrContextOptions& options) {
    sk_sp<GrDirectContext> direct(new GrDirectContext(GrBackendApi::kVulkan, options));

    direct->fGpu = GrVkGpu::Make(backendContext, options, direct.get());
    if (!direct->init()) {
        return nullptr;
    }

    return direct;
}

#endif
