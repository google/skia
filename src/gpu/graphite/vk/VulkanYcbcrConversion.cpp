/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanYcbcrConversion.h"

#include "include/gpu/graphite/vk/VulkanGraphiteTypes.h"
#include "src/gpu/graphite/vk/VulkanCaps.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtils.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"

namespace skgpu::graphite {

namespace {

static constexpr int kUsesExternalFormatBits  = 1;
static constexpr int kYcbcrModelBits          = 3;
static constexpr int kYcbcrRangeBits          = 1;
static constexpr int kXChromaOffsetBits       = 1;
static constexpr int kYChromaOffsetBits       = 1;
static constexpr int kChromaFilterBits        = 1;
static constexpr int kForceExplicitReconBits  = 1;
static constexpr int kComponentBits           = 3;

static constexpr int kUsesExternalFormatShift = 0;
static constexpr int kYcbcrModelShift         = kUsesExternalFormatShift +
                                                kUsesExternalFormatBits;
static constexpr int kYcbcrRangeShift         = kYcbcrModelShift         + kYcbcrModelBits;
static constexpr int kXChromaOffsetShift      = kYcbcrRangeShift         + kYcbcrRangeBits;
static constexpr int kYChromaOffsetShift      = kXChromaOffsetShift      + kXChromaOffsetBits;
static constexpr int kChromaFilterShift       = kYChromaOffsetShift      + kYChromaOffsetBits;
static constexpr int kForceExplicitReconShift = kChromaFilterShift       + kChromaFilterBits;
static constexpr int kComponentRShift         = kForceExplicitReconShift + kComponentBits;
static constexpr int kComponentGShift         = kComponentRShift         + kComponentBits;
static constexpr int kComponentBShift         = kComponentGShift         + kComponentBits;
static constexpr int kComponentAShift         = kComponentBShift         + kComponentBits;

static constexpr uint32_t kUseExternalFormatMask =
        ((1 << kUsesExternalFormatBits) - 1) << kUsesExternalFormatShift;
static constexpr uint32_t kYcbcrModelMask =
        ((1 << kYcbcrModelBits) - 1) << kYcbcrModelShift;
static constexpr uint32_t kYcbcrRangeMask =
        ((1 << kYcbcrRangeBits) - 1) << kYcbcrRangeShift;
static constexpr uint32_t kXChromaOffsetMask =
        ((1 << kXChromaOffsetBits) - 1) << kXChromaOffsetShift;
static constexpr uint32_t kYChromaOffsetMask =
        ((1 << kYChromaOffsetBits) - 1) << kYChromaOffsetShift;
static constexpr uint32_t kChromaFilterMask =
        ((1 << kChromaFilterBits) - 1) << kChromaFilterShift;
static constexpr uint32_t kForceExplicitReconMask =
        ((1 << kForceExplicitReconBits) - 1) << kForceExplicitReconShift;
static constexpr uint32_t kComponentRMask = ((1 << kComponentBits) - 1) << kComponentRShift;
static constexpr uint32_t kComponentBMask = ((1 << kComponentBits) - 1) << kComponentGShift;
static constexpr uint32_t kComponentGMask = ((1 << kComponentBits) - 1) << kComponentBShift;
static constexpr uint32_t kComponentAMask = ((1 << kComponentBits) - 1) << kComponentAShift;

}  // anonymous namespace

sk_sp<VulkanYcbcrConversion> VulkanYcbcrConversion::Make(
        const VulkanSharedContext* context, const VulkanYcbcrConversionInfo& conversionInfo) {
    if (!context->vulkanCaps().supportsYcbcrConversion()) {
        return nullptr;
    }

    VkSamplerYcbcrConversionCreateInfo ycbcrCreateInfo;
    std::optional<VkFilter> requiredSamplerFilter;
    skgpu::SetupSamplerYcbcrConversionInfo(
            &ycbcrCreateInfo, &requiredSamplerFilter, conversionInfo);

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
    return sk_sp<VulkanYcbcrConversion>(
            new VulkanYcbcrConversion(context, conversion, requiredSamplerFilter));
}

ImmutableSamplerInfo VulkanYcbcrConversion::ToImmutableSamplerInfo(
        const VulkanYcbcrConversionInfo& conversionInfo) {
    SkASSERT(conversionInfo.isValid());

    static_assert(kComponentAShift + kComponentBits <= 32);

    SkASSERT(conversionInfo.fYcbcrModel                  < (1u << kYcbcrModelBits        ));
    SkASSERT(conversionInfo.fYcbcrRange                  < (1u << kYcbcrRangeBits        ));
    SkASSERT(conversionInfo.fXChromaOffset               < (1u << kXChromaOffsetBits     ));
    SkASSERT(conversionInfo.fYChromaOffset               < (1u << kYChromaOffsetBits     ));
    SkASSERT(conversionInfo.fChromaFilter                < (1u << kChromaFilterBits      ));
    SkASSERT(conversionInfo.fForceExplicitReconstruction < (1u << kForceExplicitReconBits));
    SkASSERT(conversionInfo.fComponents.r                < (1u << kComponentBits         ));
    SkASSERT(conversionInfo.fComponents.g                < (1u << kComponentBits         ));
    SkASSERT(conversionInfo.fComponents.b                < (1u << kComponentBits         ));
    SkASSERT(conversionInfo.fComponents.a                < (1u << kComponentBits         ));

    const bool usesExternalFormat = conversionInfo.fFormat == VK_FORMAT_UNDEFINED;

    ImmutableSamplerInfo info;
    info.fNonFormatYcbcrConversionInfo =
            (((uint32_t)(usesExternalFormat                         ) << kUsesExternalFormatShift) |
             ((uint32_t)(conversionInfo.fYcbcrModel                 ) << kYcbcrModelShift        ) |
             ((uint32_t)(conversionInfo.fYcbcrRange                 ) << kYcbcrRangeShift        ) |
             ((uint32_t)(conversionInfo.fXChromaOffset              ) << kXChromaOffsetShift     ) |
             ((uint32_t)(conversionInfo.fYChromaOffset              ) << kYChromaOffsetShift     ) |
             ((uint32_t)(conversionInfo.fChromaFilter               ) << kChromaFilterShift      ) |
             ((uint32_t)(conversionInfo.fForceExplicitReconstruction) << kForceExplicitReconShift) |
             ((uint32_t)(conversionInfo.fComponents.r               ) << kComponentRShift        ) |
             ((uint32_t)(conversionInfo.fComponents.g               ) << kComponentGShift        ) |
             ((uint32_t)(conversionInfo.fComponents.b               ) << kComponentBShift        ) |
             ((uint32_t)(conversionInfo.fComponents.a               ) << kComponentAShift        ));
    info.fFormat = usesExternalFormat ? conversionInfo.fExternalFormat
                                      : static_cast<uint64_t>(conversionInfo.fFormat);
    return info;
}

VulkanYcbcrConversionInfo VulkanYcbcrConversion::FromImmutableSamplerInfo(
        ImmutableSamplerInfo info) {
    const uint32_t nonFormatInfo = info.fNonFormatYcbcrConversionInfo;

    VulkanYcbcrConversionInfo vkInfo;
    const bool usesExternalFormat =
            (nonFormatInfo >> kUsesExternalFormatShift) & kUseExternalFormatMask;
    if (usesExternalFormat) {
        vkInfo.fFormat = VK_FORMAT_UNDEFINED;
        vkInfo.fExternalFormat = info.fFormat;
    } else {
        vkInfo.fFormat = static_cast<VkFormat>(info.fFormat);
        vkInfo.fExternalFormat = 0;
    }

    vkInfo.fYcbcrModel    =   static_cast<VkSamplerYcbcrModelConversion>(
                                      (nonFormatInfo & kYcbcrModelMask) >> kYcbcrModelShift);
    vkInfo.fYcbcrRange    =   static_cast<VkSamplerYcbcrRange>(
                                      (nonFormatInfo & kYcbcrRangeMask) >> kYcbcrRangeShift);
    vkInfo.fComponents    = { static_cast<VkComponentSwizzle>(
                                      (nonFormatInfo & kComponentRMask) >> kComponentRShift),
                              static_cast<VkComponentSwizzle>(
                                      (nonFormatInfo & kComponentGMask) >> kComponentGShift),
                              static_cast<VkComponentSwizzle>(
                                      (nonFormatInfo & kComponentBMask) >> kComponentBShift),
                              static_cast<VkComponentSwizzle>(
                                      (nonFormatInfo & kComponentAMask) >> kComponentAShift) };
    vkInfo.fXChromaOffset =   static_cast<VkChromaLocation>(
                                      (nonFormatInfo & kXChromaOffsetMask) >> kXChromaOffsetShift);
    vkInfo.fYChromaOffset =   static_cast<VkChromaLocation>(
                                      (nonFormatInfo & kYChromaOffsetMask) >> kYChromaOffsetShift);
    vkInfo.fChromaFilter  =   static_cast<VkFilter>(
                                      (nonFormatInfo & kChromaFilterMask) >> kChromaFilterShift);

    vkInfo.fForceExplicitReconstruction = static_cast<VkBool32>(
                    (nonFormatInfo & kForceExplicitReconMask) >> kForceExplicitReconShift);

    return vkInfo;
}

VulkanYcbcrConversion::VulkanYcbcrConversion(const VulkanSharedContext* context,
                                             VkSamplerYcbcrConversion ycbcrConversion,
                                             std::optional<VkFilter> requiredFilter)
        : Resource(context,
                   Ownership::kOwned,
                   /*gpuMemorySize=*/0)
        , fYcbcrConversion(ycbcrConversion)
        , fRequiredFilter(requiredFilter) {}

void VulkanYcbcrConversion::freeGpuData() {
    auto sharedContext = static_cast<const VulkanSharedContext*>(this->sharedContext());
    SkASSERT(fYcbcrConversion != VK_NULL_HANDLE);
    VULKAN_CALL(sharedContext->interface(),
                DestroySamplerYcbcrConversion(sharedContext->device(), fYcbcrConversion, nullptr));
}

} // namespace skgpu::graphite
