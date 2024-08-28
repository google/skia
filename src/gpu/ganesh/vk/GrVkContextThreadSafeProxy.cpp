/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/ganesh/vk/GrVkContextThreadSafeProxy.h"

#include "include/core/SkRefCnt.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "src/gpu/ganesh/vk/GrVkCaps.h"

class GrCaps;
struct GrContextOptions;

GrVkContextThreadSafeProxy::GrVkContextThreadSafeProxy(const GrContextOptions& opts)
        : GrContextThreadSafeProxy(GrBackendApi::kVulkan, opts) {}

bool GrVkContextThreadSafeProxy::isValidCharacterizationForVulkan(
        sk_sp<const GrCaps> caps,
        bool isTextureable,
        skgpu::Mipmapped isMipmapped,
        skgpu::Protected isProtected,
        bool vkRTSupportsInputAttachment,
        bool forVulkanSecondaryCommandBuffer) {
    if (forVulkanSecondaryCommandBuffer &&
        (isTextureable || isMipmapped == skgpu::Mipmapped::kYes || vkRTSupportsInputAttachment)) {
        return false;
    }

    const GrVkCaps* vkCaps = (const GrVkCaps*)caps.get();

    // The protection status of the characterization and the context need to match
    return isProtected == GrProtected(vkCaps->supportsProtectedContent());
}
