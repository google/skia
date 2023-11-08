/*
 * Copyright 2022 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/gpu/graphite/VulkanGraphiteTypesPriv.h"

namespace skgpu::graphite {

VulkanTextureInfo VulkanTextureSpecToTextureInfo(const VulkanTextureSpec& vkSpec,
                                                 uint32_t sampleCount,
                                                 Mipmapped mipmapped) {
    return VulkanTextureInfo(sampleCount,
                             mipmapped,
                             vkSpec.fFlags,
                             vkSpec.fFormat,
                             vkSpec.fImageTiling,
                             vkSpec.fImageUsageFlags,
                             vkSpec.fSharingMode,
                             vkSpec.fAspectMask,
                             vkSpec.fYcbcrConversionInfo);
}

} // namespace skgpu::graphite

