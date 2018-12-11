/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrVkSamplerYcbcrConversion.h"

#include "GrVkGpu.h"

GrVkSamplerYcbcrConversion* GrVkSamplerYcbcrConversion::Create(
        const GrVkGpu* gpu, const GrVkYcbcrConversionInfo& info) {
    if (!gpu->vkCaps().supportsYcbcrConversion()) {
        return nullptr;
    }
    // We only support creating ycbcr conversion for external formats;
    SkASSERT(info.fExternalFormat);

#ifdef SK_DEBUG
    const VkFormatFeatureFlags& featureFlags = info.fExternalFormatFeatures;
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

#ifdef SK_BUILD_FOR_ANDROID
    VkExternalFormatANDROID externalFormat;
    externalFormat.sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID;
    externalFormat.pNext = nullptr;
    externalFormat.externalFormat = info.fExternalFormat;

    VkSamplerYcbcrConversionCreateInfo ycbcrCreateInfo;
    memset(&ycbcrCreateInfo, 0, sizeof(VkSamplerYcbcrConversionCreateInfo));
    ycbcrCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO;
    ycbcrCreateInfo.pNext = &externalFormat;
    ycbcrCreateInfo.format = VK_FORMAT_UNDEFINED;
    ycbcrCreateInfo.ycbcrModel = info.fYcbcrModel;
    ycbcrCreateInfo.ycbcrRange = info.fYcbcrRange;
    // Componets is ignored for external format conversions;
    // ycbcrCreateInfo.components = {0, 0, 0, 0};
    ycbcrCreateInfo.xChromaOffset = info.fXChromaOffset;
    ycbcrCreateInfo.yChromaOffset = info.fYChromaOffset;
    ycbcrCreateInfo.chromaFilter = info.fChromaFilter;
    ycbcrCreateInfo.forceExplicitReconstruction = info.fForceExplicitReconstruction;

    VkSamplerYcbcrConversion conversion;
    GR_VK_CALL(gpu->vkInterface(), CreateSamplerYcbcrConversion(gpu->device(), &ycbcrCreateInfo,
                                                                nullptr, &conversion));
    if (conversion == VK_NULL_HANDLE) {
        return nullptr;
    }
    return new GrVkSamplerYcbcrConversion(conversion, GenerateKey(info));
#else
    return nullptr;
#endif
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

    return {ycbcrInfo.fExternalFormat, ycbcrKey};
}

