/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkString.h"
#include "include/gpu/graphite/vk/VulkanGraphiteTypes.h"
#include "include/gpu/vk/VulkanMutableTextureState.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtils.h"
#include "src/gpu/vk/VulkanUtilsPriv.h"

#include <cstdint>

namespace skgpu::graphite {

SkString VulkanTextureInfo::toBackendString() const {
    return SkStringPrintf(
            "flags=0x%08X,imageTiling=%d,imageUsageFlags=0x%08X,sharingMode=%d,"
            "aspectMask=%u",
            fFlags,
            fImageTiling,
            fImageUsageFlags,
            fSharingMode,
            fAspectMask);
}

TextureFormat VulkanTextureInfo::viewFormat() const {
    if (fYcbcrConversionInfo.isValid()) {
        return fYcbcrConversionInfo.format() == VK_FORMAT_UNDEFINED
                ? TextureFormat::kExternal
                : VkFormatToTextureFormat(fYcbcrConversionInfo.format());
    }

    return VkFormatToTextureFormat(fFormat);
}

bool VulkanTextureInfo::isCompatible(const TextureInfo& that, bool requireExact) const {
    const auto& vt = TextureInfoPriv::Get<VulkanTextureInfo>(that);

    // The usages may match or the usage passed in may be a superset of the usage stored within.
    const auto usageMask = requireExact ? vt.fImageUsageFlags : fImageUsageFlags;
    return fFlags == vt.fFlags &&
           fFormat == vt.fFormat &&
           fImageTiling == vt.fImageTiling &&
           fSharingMode == vt.fSharingMode &&
           fAspectMask == vt.fAspectMask &&
           (usageMask & vt.fImageUsageFlags) == fImageUsageFlags &&
           fYcbcrConversionInfo == vt.fYcbcrConversionInfo;
}

namespace TextureInfos {

TextureInfo MakeVulkan(const VulkanTextureInfo& vkInfo) {
    return TextureInfoPriv::Make(vkInfo);
}

bool GetVulkanTextureInfo(const TextureInfo& info, VulkanTextureInfo* out) {
    return TextureInfoPriv::Copy(info, out);
}

}  // namespace TextureInfos

}  // namespace skgpu::graphite
