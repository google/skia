/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/gpu/ganesh/GrVkTypesPriv.h"

#include "src/gpu/MutableTextureStateRef.h"
#include "src/gpu/ganesh/vk/GrVkImageLayout.h"

GrVkImageInfo GrVkImageInfoWithMutableState(const GrVkImageInfo& info,
                                            const skgpu::MutableTextureStateRef* mutableState) {
    SkASSERT(mutableState);
    GrVkImageInfo newInfo = info;
    newInfo.fImageLayout = mutableState->getImageLayout();
    newInfo.fCurrentQueueFamily = mutableState->getQueueFamilyIndex();
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
