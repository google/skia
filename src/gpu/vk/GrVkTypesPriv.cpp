/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/GrVkTypesPriv.h"

#include "src/gpu/GrBackendSurfaceMutableStateImpl.h"
#include "src/gpu/vk/GrVkImageLayout.h"

void GrVkBackendSurfaceInfo::cleanup() {};

void GrVkBackendSurfaceInfo::assign(const GrVkBackendSurfaceInfo& that, bool isThisValid) {
    fImageInfo = that.fImageInfo;
}

GrVkImageInfo GrVkBackendSurfaceInfo::snapImageInfo(
        const GrBackendSurfaceMutableStateImpl* mutableState) const {
    SkASSERT(mutableState);
    GrVkImageInfo newInfo = fImageInfo;
    newInfo.fImageLayout = mutableState->getImageLayout();
    newInfo.fCurrentQueueFamily = mutableState->getQueueFamilyIndex();
    return newInfo;
}

GrVkSurfaceInfo GrVkImageSpecToSurfaceInfo(const GrVkImageSpec& vkSpec,
                                           uint32_t sampleCount,
                                           uint32_t levelCount,
                                           GrProtected isProtected) {
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

#if GR_TEST_UTILS
bool GrVkBackendSurfaceInfo::operator==(const GrVkBackendSurfaceInfo& that) const {
    GrVkImageInfo cpyInfoThis = fImageInfo;
    GrVkImageInfo cpyInfoThat = that.fImageInfo;
    // We don't care about the fImageLayout or fCurrentQueueFamily here since we require they use
    // the same mutableState.
    cpyInfoThis.fImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    cpyInfoThat.fImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    cpyInfoThis.fCurrentQueueFamily = VK_QUEUE_FAMILY_IGNORED;
    cpyInfoThat.fCurrentQueueFamily = VK_QUEUE_FAMILY_IGNORED;
    return cpyInfoThis == cpyInfoThat;
}
#endif
