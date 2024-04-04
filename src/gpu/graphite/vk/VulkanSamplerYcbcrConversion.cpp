/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanSamplerYcbcrConversion.h"

#include "include/gpu/graphite/vk/VulkanGraphiteTypes.h"
#include "src/gpu/graphite/vk/VulkanCaps.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"

namespace skgpu::graphite {

sk_sp<VulkanSamplerYcbcrConversion> VulkanSamplerYcbcrConversion::Make(
        const VulkanSharedContext* context,
        const VulkanYcbcrConversionInfo& conversionInfo) {
    if (!context->vulkanCaps().supportsYcbcrConversion()) {
        return nullptr;
    }

    VkSamplerYcbcrConversionCreateInfo ycbcrCreateInfo;
    skgpu::SetupSamplerYcbcrConversionInfo(&ycbcrCreateInfo, conversionInfo);

#ifdef SK_BUILD_FOR_ANDROID
    VkExternalFormatANDROID externalFormat;
    if (conversionInfo.fExternalFormat) {
        // Format must not be specified for external images.
        SkASSERT(conversionInfo.fFormat == VK_FORMAT_UNDEFINED);
        externalFormat.sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID;
        externalFormat.pNext = nullptr;
        externalFormat.externalFormat = conversionInfo.fExternalFormat;
        SkASSERT(ycbcrCreateInfo.pNext == nullptr);
        ycbcrCreateInfo.pNext = &externalFormat;
    }
#else
    // External images are supported only on Android.
    SkASSERT(!conversionInfo.fExternalFormat);
#endif

    if (!conversionInfo.fExternalFormat) {
        SkASSERT(conversionInfo.fFormat != VK_FORMAT_UNDEFINED);
    }

    VkSamplerYcbcrConversion conversion;
    VkResult result;
    VULKAN_CALL_RESULT(context->interface(), result,
                       CreateSamplerYcbcrConversion(context->device(),
                                                    &ycbcrCreateInfo,
                                                    nullptr,
                                                    &conversion));
    if (result != VK_SUCCESS) {
        return nullptr;
    }
    return sk_sp<VulkanSamplerYcbcrConversion>(
            new VulkanSamplerYcbcrConversion(context, conversion));
}

GraphiteResourceKey VulkanSamplerYcbcrConversion::MakeYcbcrConversionKey(
        const VulkanSharedContext* context, const VulkanYcbcrConversionInfo& info) {
    static const ResourceType kType = GraphiteResourceKey::GenerateResourceType();

    // One uint32 for Vkformat, two for external format, and one to house all ycbcr information
    static const int num32DataCnt = 4;
    GraphiteResourceKey key;
    GraphiteResourceKey::Builder builder(&key, kType, num32DataCnt, Shareable::kYes);

    SkASSERT(info.fYcbcrModel                  < (1u << 7));
    SkASSERT(info.fYcbcrRange                  < (1u << 1));
    SkASSERT(info.fXChromaOffset               < (1u << 1));
    SkASSERT(info.fYChromaOffset               < (1u << 1));
    SkASSERT(info.fChromaFilter                < (1u << 1));
    SkASSERT(info.fForceExplicitReconstruction < (1u << 7));
    SkASSERT(info.fComponents.r                < (1u << 3));
    SkASSERT(info.fComponents.g                < (1u << 3));
    SkASSERT(info.fComponents.b                < (1u << 3));
    SkASSERT(info.fComponents.a                < (1u << 3));

    builder[0] = info.fFormat;
    builder[1] = (uint32_t)(info.fExternalFormat << 32);
    builder[2] = (uint32_t)info.fExternalFormat;
    builder[3] = (static_cast<uint32_t>(info.fYcbcrModel                 ) <<  0) |
                 (static_cast<uint32_t>(info.fYcbcrRange                 ) <<  8) |
                 (static_cast<uint32_t>(info.fXChromaOffset              ) <<  9) |
                 (static_cast<uint32_t>(info.fYChromaOffset              ) << 10) |
                 (static_cast<uint32_t>(info.fChromaFilter               ) << 11) |
                 (static_cast<uint32_t>(info.fForceExplicitReconstruction) << 12) |
                 (static_cast<uint32_t>(info.fComponents.r               ) << 20) |
                 (static_cast<uint32_t>(info.fComponents.g               ) << 23) |
                 (static_cast<uint32_t>(info.fComponents.b               ) << 26) |
                 (static_cast<uint32_t>(info.fComponents.a               ) << 29) ;

    builder.finish();
    return key;
}

VulkanSamplerYcbcrConversion::VulkanSamplerYcbcrConversion(
        const VulkanSharedContext* context, VkSamplerYcbcrConversion ycbcrConversion)
        : Resource(context,
                   Ownership::kOwned,
                   skgpu::Budgeted::kNo,
                   /*gpuMemorySize=*/0,
                   /*label=*/"VulkanSamplerYcbcrConversion")
        , fYcbcrConversion (ycbcrConversion) {}

void VulkanSamplerYcbcrConversion::freeGpuData() {
    auto sharedContext = static_cast<const VulkanSharedContext*>(this->sharedContext());
    SkASSERT(fYcbcrConversion != VK_NULL_HANDLE);
    VULKAN_CALL(sharedContext->interface(),
                DestroySamplerYcbcrConversion(sharedContext->device(), fYcbcrConversion, nullptr));
}

} // namespace skgpu::graphite

