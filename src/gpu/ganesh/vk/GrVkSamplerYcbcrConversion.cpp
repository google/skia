/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/vk/GrVkSamplerYcbcrConversion.h"

#include "include/gpu/vk/VulkanTypes.h"
#include "src/gpu/ganesh/vk/GrVkGpu.h"
#include "src/gpu/vk/VulkanUtilsPriv.h"

GrVkSamplerYcbcrConversion* GrVkSamplerYcbcrConversion::Create(
        GrVkGpu* gpu, const GrVkYcbcrConversionInfo& info) {
    if (!gpu->vkCaps().supportsYcbcrConversion()) {
        return nullptr;
    }

    VkSamplerYcbcrConversionCreateInfo ycbcrCreateInfo;
    skgpu::SetupSamplerYcbcrConversionInfo(&ycbcrCreateInfo, info);

#ifdef SK_BUILD_FOR_ANDROID
    VkExternalFormatANDROID externalFormat;
    if (info.fExternalFormat) {
        // Format must not be specified for external images.
        SkASSERT(info.fFormat == VK_FORMAT_UNDEFINED);
        externalFormat.sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID;
        externalFormat.pNext = nullptr;
        externalFormat.externalFormat = info.fExternalFormat;
        SkASSERT(ycbcrCreateInfo.pNext == nullptr);
        ycbcrCreateInfo.pNext = &externalFormat;
    }
#else
    // External images are supported only on Android.
    SkASSERT(!info.fExternalFormat);
#endif

    if (!info.fExternalFormat) {
        SkASSERT(info.fFormat != VK_FORMAT_UNDEFINED);
    }

    VkSamplerYcbcrConversion conversion;
    VkResult result;
    GR_VK_CALL_RESULT(gpu, result, CreateSamplerYcbcrConversion(gpu->device(), &ycbcrCreateInfo,
                                                                nullptr, &conversion));
    if (result != VK_SUCCESS) {
        return nullptr;
    }

    return new GrVkSamplerYcbcrConversion(gpu, conversion, GenerateKey(info));
}

void GrVkSamplerYcbcrConversion::freeGPUData() const {
    SkASSERT(fYcbcrConversion);
    GR_VK_CALL(fGpu->vkInterface(), DestroySamplerYcbcrConversion(fGpu->device(),
                                                                  fYcbcrConversion, nullptr));
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
    static_assert(kReconShift <= 7);

    uint8_t ycbcrKey = static_cast<uint8_t>(ycbcrInfo.fYcbcrModel);
    ycbcrKey |= (static_cast<uint8_t>(ycbcrInfo.fYcbcrRange) << kRangeShift);
    ycbcrKey |= (static_cast<uint8_t>(ycbcrInfo.fXChromaOffset) << kXChromaOffsetShift);
    ycbcrKey |= (static_cast<uint8_t>(ycbcrInfo.fYChromaOffset) << kYChromaOffsetShift);
    ycbcrKey |= (static_cast<uint8_t>(ycbcrInfo.fChromaFilter) << kChromaFilterShift);
    ycbcrKey |= (static_cast<uint8_t>(ycbcrInfo.fForceExplicitReconstruction) << kReconShift);

    return Key{ycbcrInfo.fFormat, ycbcrInfo.fExternalFormat, ycbcrKey};
}
