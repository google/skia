/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/vk/GrVkSamplerYcbcrConversion.h"

#include "include/gpu/vk/VulkanTypes.h"
#include "include/private/base/SkAssert.h"
#include "src/gpu/ganesh/vk/GrVkCaps.h"
#include "src/gpu/ganesh/vk/GrVkGpu.h"
#include "src/gpu/ganesh/vk/GrVkUtil.h"
#include "src/gpu/vk/VulkanUtilsPriv.h"

GrVkSamplerYcbcrConversion* GrVkSamplerYcbcrConversion::Create(
        GrVkGpu* gpu, const skgpu::VulkanYcbcrConversionInfo& info) {
    if (!gpu->vkCaps().supportsYcbcrConversion()) {
        return nullptr;
    }

    VkSamplerYcbcrConversionCreateInfo ycbcrCreateInfo;
    std::optional<VkFilter> requiredSamplerFilter;
    info.toVkSamplerYcbcrConversionCreateInfo(&ycbcrCreateInfo, &requiredSamplerFilter);

#ifdef SK_BUILD_FOR_ANDROID
    VkExternalFormatANDROID externalFormat;
    if (info.hasExternalFormat()) {
        // Format must not be specified for external images.
        SkASSERT(info.format() == VK_FORMAT_UNDEFINED);
        externalFormat.sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID;
        externalFormat.pNext = nullptr;
        externalFormat.externalFormat = info.externalFormat();
        SkASSERT(ycbcrCreateInfo.pNext == nullptr);
        ycbcrCreateInfo.pNext = &externalFormat;
    }
#else
    // External images are supported only on Android.
    SkASSERT(!info.hasExternalFormat());
#endif

    if (!info.hasExternalFormat()) {
        SkASSERT(info.format() != VK_FORMAT_UNDEFINED);
    }

    VkSamplerYcbcrConversion conversion;
    VkResult result;
    GR_VK_CALL_RESULT(gpu, result, CreateSamplerYcbcrConversion(gpu->device(), &ycbcrCreateInfo,
                                                                nullptr, &conversion));
    if (result != VK_SUCCESS) {
        return nullptr;
    }

    return new GrVkSamplerYcbcrConversion(
            gpu, conversion, requiredSamplerFilter, GenerateKey(info));
}

void GrVkSamplerYcbcrConversion::freeGPUData() const {
    SkASSERT(fYcbcrConversion);
    GR_VK_CALL(fGpu->vkInterface(), DestroySamplerYcbcrConversion(fGpu->device(),
                                                                  fYcbcrConversion, nullptr));
}

GrVkSamplerYcbcrConversion::Key GrVkSamplerYcbcrConversion::GenerateKey(
        const skgpu::VulkanYcbcrConversionInfo& ycbcrInfo) {
    SkASSERT(static_cast<int>(ycbcrInfo.model() <= 7));
    static const int kRangeShift = 3;
    SkASSERT(static_cast<int>(ycbcrInfo.range()) <= 1);
    static const int kXChromaOffsetShift = kRangeShift + 1;
    SkASSERT(static_cast<int>(ycbcrInfo.xChromaOffset()) <= 1);
    static const int kYChromaOffsetShift = kXChromaOffsetShift + 1;
    SkASSERT(static_cast<int>(ycbcrInfo.yChromaOffset()) <= 1);
    static const int kChromaFilterShift = kYChromaOffsetShift + 1;
    SkASSERT(static_cast<int>(ycbcrInfo.chromaFilter()) <= 1);
    static const int kReconShift = kChromaFilterShift + 1;
    SkASSERT(static_cast<int>(ycbcrInfo.forceExplicitReconstruction()) <= 1);
    static const int kCompRShift = kReconShift + 1;
    static const int kCompGShift = kCompRShift + 3;
    static const int kCompBShift = kCompGShift + 3;
    static const int kCompAShift = kCompBShift + 3;
    SkASSERT(static_cast<int>(ycbcrInfo.components().r <= 6) &&
             static_cast<int>(ycbcrInfo.components().g <= 6) &&
             static_cast<int>(ycbcrInfo.components().b <= 6) &&
             static_cast<int>(ycbcrInfo.components().a <= 6));
    static_assert(kCompAShift <= 17);

    uint32_t ycbcrKey = static_cast<uint32_t>(ycbcrInfo.model());
    ycbcrKey |= (static_cast<uint32_t>(ycbcrInfo.range()) << kRangeShift);
    ycbcrKey |= (static_cast<uint32_t>(ycbcrInfo.xChromaOffset()) << kXChromaOffsetShift);
    ycbcrKey |= (static_cast<uint32_t>(ycbcrInfo.yChromaOffset()) << kYChromaOffsetShift);
    ycbcrKey |= (static_cast<uint32_t>(ycbcrInfo.chromaFilter()) << kChromaFilterShift);
    ycbcrKey |= (static_cast<uint32_t>(ycbcrInfo.forceExplicitReconstruction()) << kReconShift);
    ycbcrKey |= (static_cast<uint32_t>(ycbcrInfo.components().r) << kCompRShift);
    ycbcrKey |= (static_cast<uint32_t>(ycbcrInfo.components().g) << kCompGShift);
    ycbcrKey |= (static_cast<uint32_t>(ycbcrInfo.components().b) << kCompBShift);
    ycbcrKey |= (static_cast<uint32_t>(ycbcrInfo.components().a) << kCompAShift);

    return Key{ycbcrInfo.format(), ycbcrInfo.externalFormat(), ycbcrKey};
}
