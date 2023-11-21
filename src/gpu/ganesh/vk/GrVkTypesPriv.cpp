/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/vk/GrVkTypesPriv.h"

#include "include/gpu/MutableTextureState.h"
#include "include/gpu/vk/VulkanMutableTextureState.h"
#include "src/gpu/ganesh/vk/GrVkImageLayout.h"

GrVkImageInfo GrVkImageInfoWithMutableState(const GrVkImageInfo& info,
                                            const skgpu::MutableTextureState* mutableState) {
    SkASSERT(mutableState);
    GrVkImageInfo newInfo = info;
    newInfo.fImageLayout = skgpu::MutableTextureStates::GetVkImageLayout(mutableState);
    newInfo.fCurrentQueueFamily = skgpu::MutableTextureStates::GetVkQueueFamilyIndex(mutableState);
    return newInfo;
}

GrVkSurfaceInfo GrVkImageSpecToSurfaceInfo(const GrVkImageSpec& vkSpec,
                                           uint32_t sampleCount,
                                           uint32_t levelCount,
                                           skgpu::Protected isProtected) {
    GrVkSurfaceInfo info;
    // Shared info
    info.fSampleCount = sampleCount;
    info.fLevelCount = levelCount;
    info.fProtected = isProtected;

    // Vulkan info
    info.fImageTiling = vkSpec.fImageTiling;
    info.fFormat = vkSpec.fFormat;
    info.fImageUsageFlags = vkSpec.fImageUsageFlags;
    info.fYcbcrConversionInfo = vkSpec.fYcbcrConversionInfo;
    info.fSharingMode = vkSpec.fSharingMode;

    return info;
}
