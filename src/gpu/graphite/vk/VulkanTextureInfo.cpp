/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkStream.h"
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
        if (fYcbcrConversionInfo.fFormat == VK_FORMAT_UNDEFINED) {
            return TextureFormat::kExternal;
        } else {
            return VkFormatToTextureFormat(fYcbcrConversionInfo.fFormat);
        }
    } else {
        return VkFormatToTextureFormat(fFormat);
    }
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

bool VulkanTextureInfo::serialize(SkWStream* stream) const {
    SkASSERT(SkTFitsIn<uint64_t>(fFlags));
    SkASSERT(SkTFitsIn<uint64_t>(fFormat));
    SkASSERT(SkTFitsIn<uint64_t>(fImageUsageFlags));
    SkASSERT(SkTFitsIn<uint8_t>(fImageTiling));
    SkASSERT(SkTFitsIn<uint8_t>(fSharingMode));
    SkASSERT(SkTFitsIn<uint32_t>(fAspectMask));

    if (!stream->write64(static_cast<uint64_t>(fFlags)))           { return false; }
    if (!stream->write64(static_cast<uint64_t>(fFormat)))          { return false; }
    if (!stream->write64(static_cast<uint64_t>(fImageUsageFlags))) { return false; }
    if (!stream->write8(static_cast<uint8_t>(fImageTiling)))       { return false; }
    if (!stream->write8(static_cast<uint8_t>(fSharingMode)))       { return false; }
    if (!stream->write32(static_cast<uint32_t>(fAspectMask)))      { return false; }

    return SerializeVkYCbCrInfo(stream, fYcbcrConversionInfo);
}

// TODO(robertphillips): add validity checks to deserialized values
bool VulkanTextureInfo::deserialize(SkStream* stream) {
    uint64_t tmp64;

    if (!stream->readU64(&tmp64)) { return false; }
    fFlags = static_cast<VkImageCreateFlags>(tmp64);

    if (!stream->readU64(&tmp64)) { return false; }
    fFormat = static_cast<VkFormat>(tmp64);

    if (!stream->readU64(&tmp64)) { return false; }
    fImageUsageFlags = static_cast<VkImageUsageFlags>(tmp64);

    uint32_t tmp32;
    uint8_t tmp8;

    if (!stream->readU8(&tmp8)) { return false; }
    fImageTiling = static_cast<VkImageTiling>(tmp8);

    if (!stream->readU8(&tmp8)) { return false; }
    fSharingMode = static_cast<VkSharingMode>(tmp8);

    if (!stream->readU32(&tmp32)) { return false; }
    fAspectMask = static_cast<VkImageAspectFlags>(tmp32);

    return DeserializeVkYCbCrInfo(stream, &fYcbcrConversionInfo);
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
