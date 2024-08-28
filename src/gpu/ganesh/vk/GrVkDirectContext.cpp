/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/ganesh/vk/GrVkDirectContext.h"

#include "include/gpu/ganesh/GrContextOptions.h"
#include "include/gpu/ganesh/GrContextThreadSafeProxy.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/vk/GrVkContextThreadSafeProxy.h"
#include "src/gpu/ganesh/vk/GrVkGpu.h"

namespace GrDirectContexts {
sk_sp<GrDirectContext> MakeVulkan(const skgpu::VulkanBackendContext& backendContext) {
    GrContextOptions defaultOptions;
    return MakeVulkan(backendContext, defaultOptions);
}

sk_sp<GrDirectContext> MakeVulkan(const skgpu::VulkanBackendContext& backendContext,
                                  const GrContextOptions& options) {
    auto direct = GrDirectContextPriv::Make(
            GrBackendApi::kVulkan, options, sk_make_sp<GrVkContextThreadSafeProxy>(options));

    GrDirectContextPriv::SetGpu(direct,
                                GrVkGpu::Make(backendContext, options, direct.get()));
    if (!GrDirectContextPriv::Init(direct)) {
        return nullptr;
    }

    return direct;
}
}  // namespace GrDirectContexts
