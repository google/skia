/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/vk/GrVkSamplerYcbcrConversion.h"

#include "src/gpu/vk/GrVkGpu.h"

GrVkSamplerYcbcrConversion* GrVkSamplerYcbcrConversion::Create(
        const GrVkGpu* gpu, const GrVkYcbcrConversionInfo& info) {
    if (!gpu->vkCaps().supportsYcbcrConversion()) {
        return nullptr;
    }

#ifdef SK_DEBUG
    const VkFormatFeatureFlags& featureFlags = info.fFormatFeatures;
    if (info.fXChromaOffset == VK_CHROMA_LOCATION_MIDPOINT ||
        info.fYChromaOffset == VK_CHROMA_LOCATION_MIDPOINT) {
        SkASSERT(featureFlags & VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT);
    }
    if (info.fXChromaOffset == VK_CHROMA_LOCATION_COSITED_EVEN ||
        info.fYChromaOffset == VK_CHROMA_LOCATION_COSITED_EVEN) {
        SkASSERT(featureFlags & VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT);
    }
    if (info.fChromaFilter == VK_FILTER_LINEAR) {
        SkASSERT(featureFlags & VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT);
    }
    if (info.fForceExplicitReconstruction) {
        SkASSERT(featureFlags &
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT);
    }
#endif


    VkSamplerYcbcrConversionCreateInfo ycbcrCreateInfo;
    ycbcrCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO;
    ycbcrCreateInfo.pNext = nullptr;
    ycbcrCreateInfo.format = info.fFormat;
    ycbcrCreateInfo.ycbcrModel = info.fYcbcrModel;
    ycbcrCreateInfo.ycbcrRange = info.fYcbcrRange;

    // Components is ignored for external format conversions. For all other formats identity swizzle
    // is used. It can be added to GrVkYcbcrConversionInfo if necessary.
    ycbcrCreateInfo.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                  VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
    ycbcrCreateInfo.xChromaOffset = info.fXChromaOffset;
    ycbcrCreateInfo.yChromaOffset = info.fYChromaOffset;
    ycbcrCreateInfo.chromaFilter = info.fChromaFilter;
    ycbcrCreateInfo.forceExplicitReconstruction = info.fForceExplicitReconstruction;

#ifdef SK_BUILD_FOR_ANDROID
    VkExternalFormatANDROID externalFormat;
    if (info.fExternalFormat) {
        // Format must not be specified for external images.
        SkASSERT(info.fFormat == VK_FORMAT_UNDEFINED);
        externalFormat.sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID;
        externalFormat.pNext = nullptr;
        externalFormat.externalFormat = info.fExternalFormat;
        ycbcrCreateInfo.pNext = &externalFormat;
    }
#else
    // External images are supported only on Android;
    SkASSERT(!info.fExternalFormat);
#endif

    if (!info.fExternalFormat) {
        SkASSERT(info.fFormat != VK_FORMAT_UNDEFINED);
    }

    VkSamplerYcbcrConversion conversion;
    GR_VK_CALL(gpu->vkInterface(), CreateSamplerYcbcrConversion(gpu->device(), &ycbcrCreateInfo,
                                                                nullptr, &conversion));
    if (conversion == VK_NULL_HANDLE) {
        return nullptr;
    }

    return new GrVkSamplerYcbcrConversion(conversion, GenerateKey(info));
}

void GrVkSamplerYcbcrConversion::freeGPUData(GrVkGpu* gpu) const {
    SkASSERT(fYcbcrConversion);
    GR_VK_CALL(gpu->vkInterface(), DestroySamplerYcbcrConversion(gpu->device(), fYcbcrConversion,
                                                                 nullptr));
}

GrVkSamplerYcbcrConversion::Key GrVkSamplerYcbcrConversion::GenerateKey(
        const GrVkYcbcrConversionInfo& ycbcrInfo) {
    SkASSERT(static_cast<int>(ycbcrInfo.fYcbcrModel <= 7));
    static const int kRangeShift = 3;
    SkASSERT(static_cast<int>(ycbcrInfo.fYcbcrRange) <= 1);
    static const int kXChromaOffsetShift = kRangeShift + 1;
    SkASSERT(static_cast<int>(ycbcrInfo.fXChromaOffset) <= 1);
    static const int kYChromaOffsetShift = kXChromaOffsetShift + 1;
    SkASSERT(static_cast<int>(ycbcrInfo.fXChromaOffset) <= 1);
    static const int kChromaFilterShift = kYChromaOffsetShift + 1;
    SkASSERT(static_cast<int>(ycbcrInfo.fChromaFilter) <= 1);
    static const int kReconShift = kChromaFilterShift + 1;
    SkASSERT(static_cast<int>(ycbcrInfo.fForceExplicitReconstruction) <= 1);
    GR_STATIC_ASSERT(kReconShift <= 7);

    uint8_t ycbcrKey = static_cast<uint8_t>(ycbcrInfo.fYcbcrModel);
    ycbcrKey |= (static_cast<uint8_t>(ycbcrInfo.fYcbcrRange) << kRangeShift);
    ycbcrKey |= (static_cast<uint8_t>(ycbcrInfo.fXChromaOffset) << kXChromaOffsetShift);
    ycbcrKey |= (static_cast<uint8_t>(ycbcrInfo.fYChromaOffset) << kYChromaOffsetShift);
    ycbcrKey |= (static_cast<uint8_t>(ycbcrInfo.fChromaFilter) << kChromaFilterShift);
    ycbcrKey |= (static_cast<uint8_t>(ycbcrInfo.fForceExplicitReconstruction) << kReconShift);

    return Key{ycbcrInfo.fFormat, ycbcrInfo.fExternalFormat, ycbcrKey};
}
