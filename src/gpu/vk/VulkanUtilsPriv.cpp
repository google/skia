/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/vk/VulkanUtilsPriv.h"

namespace skgpu {
/**
 * Returns a populated VkSamplerYcbcrConversionCreateInfo object based on VulkanYcbcrConversionInfo
*/
void SetupSamplerYcbcrConversionInfo(VkSamplerYcbcrConversionCreateInfo* outInfo,
                                     const VulkanYcbcrConversionInfo& conversionInfo) {
#ifdef SK_DEBUG
    const VkFormatFeatureFlags& featureFlags = conversionInfo.fFormatFeatures;
    if (conversionInfo.fXChromaOffset == VK_CHROMA_LOCATION_MIDPOINT ||
        conversionInfo.fYChromaOffset == VK_CHROMA_LOCATION_MIDPOINT) {
        SkASSERT(featureFlags & VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT);
    }
    if (conversionInfo.fXChromaOffset == VK_CHROMA_LOCATION_COSITED_EVEN ||
        conversionInfo.fYChromaOffset == VK_CHROMA_LOCATION_COSITED_EVEN) {
        SkASSERT(featureFlags & VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT);
    }
    if (conversionInfo.fChromaFilter == VK_FILTER_LINEAR) {
        SkASSERT(featureFlags & VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT);
    }
    if (conversionInfo.fForceExplicitReconstruction) {
        SkASSERT(featureFlags &
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT);
    }
#endif

    outInfo->sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO;
    outInfo->pNext = nullptr;
    outInfo->format = conversionInfo.fFormat;
    outInfo->ycbcrModel = conversionInfo.fYcbcrModel;
    outInfo->ycbcrRange = conversionInfo.fYcbcrRange;

    // Components is ignored for external format conversions. For all other formats identity swizzle
    // is used. It can be added to VulkanYcbcrConversionInfo if necessary.
    outInfo->components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                           VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
    outInfo->xChromaOffset = conversionInfo.fXChromaOffset;
    outInfo->yChromaOffset = conversionInfo.fYChromaOffset;
    outInfo->chromaFilter = conversionInfo.fChromaFilter;
    outInfo->forceExplicitReconstruction = conversionInfo.fForceExplicitReconstruction;
}

} // namespace skgpu
