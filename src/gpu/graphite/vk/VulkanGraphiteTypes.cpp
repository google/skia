/*
 * Copyright 2022 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStream.h"
#include "src/gpu/graphite/vk/VulkanGraphiteTypesPriv.h"
#include "src/gpu/vk/VulkanUtilsPriv.h"

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


bool VulkanTextureSpec::serialize(SkWStream* stream) const {
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
bool VulkanTextureSpec::Deserialize(SkStream* stream, VulkanTextureSpec* out) {
    uint64_t tmp64;

    if (!stream->readU64(&tmp64)) { return false; }
    out->fFlags = static_cast<VkImageCreateFlags>(tmp64);

    if (!stream->readU64(&tmp64)) { return false; }
    out->fFormat = static_cast<VkFormat>(tmp64);

    if (!stream->readU64(&tmp64)) { return false; }
    out->fImageUsageFlags = static_cast<VkImageUsageFlags>(tmp64);

    uint32_t tmp32;
    uint8_t tmp8;

    if (!stream->readU8(&tmp8)) { return false; }
    out->fImageTiling = static_cast<VkImageTiling>(tmp8);

    if (!stream->readU8(&tmp8)) { return false; }
    out->fSharingMode = static_cast<VkSharingMode>(tmp8);

    if (!stream->readU32(&tmp32)) { return false; }
    out->fAspectMask = static_cast<VkImageAspectFlags>(tmp32);

    return DeserializeVkYCbCrInfo(stream, &out->fYcbcrConversionInfo);
}

} // namespace skgpu::graphite
