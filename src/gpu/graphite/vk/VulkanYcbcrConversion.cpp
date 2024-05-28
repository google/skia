/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanYcbcrConversion.h"

#include "include/gpu/graphite/vk/VulkanGraphiteTypes.h"
#include "src/gpu/graphite/vk/VulkanCaps.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"

namespace skgpu::graphite {

sk_sp<VulkanYcbcrConversion> VulkanYcbcrConversion::Make(
        const VulkanSharedContext* context, const VulkanYcbcrConversionInfo& conversionInfo) {
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
    VULKAN_CALL_RESULT(context,
                       result,
                       CreateSamplerYcbcrConversion(
                               context->device(), &ycbcrCreateInfo, nullptr, &conversion));
    if (result != VK_SUCCESS) {
        return nullptr;
    }
    return sk_sp<VulkanYcbcrConversion>(new VulkanYcbcrConversion(context, conversion));
}

sk_sp<VulkanYcbcrConversion> VulkanYcbcrConversion::Make(const VulkanSharedContext* context,
                                                         uint32_t nonFormatInfo,
                                                         uint64_t format) {
    VkSamplerYcbcrConversionCreateInfo ycbcrCreateInfo;

    bool useExternalFormat =  static_cast<bool>(
            (nonFormatInfo & ycbcrPackaging::kUseExternalFormatMask) >>
                    ycbcrPackaging::kUsesExternalFormatShift);

    ycbcrCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO;
    ycbcrCreateInfo.pNext = nullptr;
    ycbcrCreateInfo.format = useExternalFormat ? VK_FORMAT_UNDEFINED
                                               : static_cast<VkFormat>(format);

    ycbcrCreateInfo.ycbcrModel = static_cast<VkSamplerYcbcrModelConversion>(
                    (nonFormatInfo & ycbcrPackaging::kYcbcrModelMask) >>
                            ycbcrPackaging::kYcbcrModelShift);
    ycbcrCreateInfo.ycbcrRange = static_cast<VkSamplerYcbcrRange>(
                    (nonFormatInfo & ycbcrPackaging::kYcbcrRangeMask) >>
                            ycbcrPackaging::kYcbcrRangeShift);
    ycbcrCreateInfo.components = {
                static_cast<VkComponentSwizzle>(
                        (nonFormatInfo & ycbcrPackaging::kComponentRMask) >>
                                ycbcrPackaging::kComponentRShift),
                static_cast<VkComponentSwizzle>(
                        (nonFormatInfo & ycbcrPackaging::kComponentGMask) >>
                                ycbcrPackaging::kComponentGShift),
                static_cast<VkComponentSwizzle>(
                        (nonFormatInfo & ycbcrPackaging::kComponentBMask) >>
                                ycbcrPackaging::kComponentBShift),
                static_cast<VkComponentSwizzle>(
                        (nonFormatInfo & ycbcrPackaging::kComponentAMask) >>
                                ycbcrPackaging::kComponentAShift)};
    ycbcrCreateInfo.xChromaOffset = static_cast<VkChromaLocation>(
                    (nonFormatInfo & ycbcrPackaging::kXChromaOffsetMask) >>
                            ycbcrPackaging::kXChromaOffsetShift);
    ycbcrCreateInfo.yChromaOffset = static_cast<VkChromaLocation>(
                    (nonFormatInfo & ycbcrPackaging::kYChromaOffsetMask) >>
                            ycbcrPackaging::kYChromaOffsetShift);
    ycbcrCreateInfo.chromaFilter = static_cast<VkFilter>(
                    (nonFormatInfo & ycbcrPackaging::kChromaFilterMask) >>
                            ycbcrPackaging::kChromaFilterShift);
    ycbcrCreateInfo.forceExplicitReconstruction = static_cast<VkBool32>(
                    (nonFormatInfo & ycbcrPackaging::kForceExplicitReconMask) >>
                            ycbcrPackaging::kForceExplicitReconShift);

#ifdef SK_BUILD_FOR_ANDROID
    VkExternalFormatANDROID externalFormat;
    if (useExternalFormat) {
        externalFormat.sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID;
        externalFormat.pNext = nullptr;
        externalFormat.externalFormat = format;
        SkASSERT(ycbcrCreateInfo.pNext == nullptr);
        ycbcrCreateInfo.pNext = &externalFormat;
    }
#endif

    VkSamplerYcbcrConversion conversion;
    VkResult result;
    VULKAN_CALL_RESULT(context,
                       result,
                       CreateSamplerYcbcrConversion(
                               context->device(), &ycbcrCreateInfo, nullptr, &conversion));
    if (result != VK_SUCCESS) {
        return nullptr;
    }
    return sk_sp<VulkanYcbcrConversion>(new VulkanYcbcrConversion(context, conversion));
}

namespace {
// Define this anonymous helper to get a static resource type for YCbCr conversions regardless of
// which method is used to create it (from VulkanYcbcrConversionInfo or from SamplerDesc)
ResourceType conversion_rsrc_type() {
    static const ResourceType conversionType = GraphiteResourceKey::GenerateResourceType();
    return conversionType;
}
}
GraphiteResourceKey VulkanYcbcrConversion::MakeYcbcrConversionKey(
        const VulkanSharedContext* context, const VulkanYcbcrConversionInfo& info) {
    bool useExternalFormat = info.fFormat == VK_FORMAT_UNDEFINED;
    GraphiteResourceKey key;
    GraphiteResourceKey::Builder builder(&key,
                                         conversion_rsrc_type(),
                                         ycbcrPackaging::numInt32sNeeded(info),
                                         Shareable::kYes);
    int i = 0;
    builder[i++] = ycbcrPackaging::nonFormatInfoAsUInt32(info);
    if (useExternalFormat) {
        builder[i++] = (uint32_t)info.fExternalFormat;
        builder[i++] = (uint32_t)(info.fExternalFormat >> 32);
    } else {
        builder[i++] = (uint32_t)info.fFormat;
    }
    SkASSERT(i == ycbcrPackaging::numInt32sNeeded(info));

    builder.finish();
    return key;
}

GraphiteResourceKey VulkanYcbcrConversion::GetKeyFromSamplerDesc(const SamplerDesc& samplerDesc) {
    GraphiteResourceKey key;

    uint32_t nonFormatYcbcrInfo =
            (uint32_t)(samplerDesc.desc() >> SamplerDesc::kImmutableSamplerInfoShift);
    SkASSERT(nonFormatYcbcrInfo != 0);

    bool usesExternalFormat =  static_cast<bool>(
                ((nonFormatYcbcrInfo & ycbcrPackaging::kUseExternalFormatMask) >>
                        ycbcrPackaging::kUsesExternalFormatShift));
    int num32DataCnt =
            usesExternalFormat ? ycbcrPackaging::kInt32sNeededExternalFormat
                               : ycbcrPackaging::kInt32sNeededKnownFormat;
    GraphiteResourceKey::Builder builder(&key, conversion_rsrc_type(), num32DataCnt,
                                         Shareable::kYes);

    int i = 0;
    builder[i++] = nonFormatYcbcrInfo;
    if (usesExternalFormat) {
        builder[i++] = samplerDesc.externalFormatMSBs();
    }
    builder[i++] = samplerDesc.format();
    SkASSERT(i == num32DataCnt);

    builder.finish();
    return key;
}

VulkanYcbcrConversion::VulkanYcbcrConversion(const VulkanSharedContext* context,
                                             VkSamplerYcbcrConversion ycbcrConversion)
        : Resource(context,
                   Ownership::kOwned,
                   skgpu::Budgeted::kYes, // Shareable, so must be budgeted
                   /*gpuMemorySize=*/0)
        , fYcbcrConversion (ycbcrConversion) {}

void VulkanYcbcrConversion::freeGpuData() {
    auto sharedContext = static_cast<const VulkanSharedContext*>(this->sharedContext());
    SkASSERT(fYcbcrConversion != VK_NULL_HANDLE);
    VULKAN_CALL(sharedContext->interface(),
                DestroySamplerYcbcrConversion(sharedContext->device(), fYcbcrConversion, nullptr));
}

} // namespace skgpu::graphite

