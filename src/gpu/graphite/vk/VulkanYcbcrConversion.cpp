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

static constexpr int kUsesExternalFormatBits   = 1;
static constexpr int kYcbcrModelBits           = 3;
static constexpr int kYcbcrRangeBits           = 1;
static constexpr int kXChromaOffsetBits        = 1;
static constexpr int kYChromaOffsetBits        = 1;
static constexpr int kChromaFilterBits         = 1;
static constexpr int kForceExplicitReconBits   = 1;
static constexpr int kComponentBits            = 3;
static constexpr int kMatchChromaFilterBits    = 1;
static constexpr int kSupportsLinearFilterBits = 1;


static constexpr int kUsesExternalFormatShift = 0;
static constexpr int kYcbcrModelShift           = kUsesExternalFormatShift +
                                                  kUsesExternalFormatBits;
static constexpr int kYcbcrRangeShift           = kYcbcrModelShift         + kYcbcrModelBits;
static constexpr int kXChromaOffsetShift        = kYcbcrRangeShift         + kYcbcrRangeBits;
static constexpr int kYChromaOffsetShift        = kXChromaOffsetShift      + kXChromaOffsetBits;
static constexpr int kChromaFilterShift         = kYChromaOffsetShift      + kYChromaOffsetBits;
static constexpr int kForceExplicitReconShift   = kChromaFilterShift       + kChromaFilterBits;
static constexpr int kComponentRShift           = kForceExplicitReconShift +
                                                  kForceExplicitReconBits;
static constexpr int kComponentGShift           = kComponentRShift         + kComponentBits;
static constexpr int kComponentBShift           = kComponentGShift         + kComponentBits;
static constexpr int kComponentAShift           = kComponentBShift         + kComponentBits;
static constexpr int kMatchChromaFilterShift    = kComponentAShift         + kComponentBits;
static constexpr int kSupportsLinearFilterShift = kMatchChromaFilterShift  + kMatchChromaFilterBits;

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
static constexpr uint32_t kMatchChromaFilterMask =
        ((1 << kMatchChromaFilterBits) - 1) << kMatchChromaFilterShift;
static constexpr uint32_t kSupportsLinearFilterMask =
        ((1 << kSupportsLinearFilterBits) - 1) << kSupportsLinearFilterShift;

}  // anonymous namespace

sk_sp<VulkanYcbcrConversion> VulkanYcbcrConversion::Make(
        const VulkanSharedContext* context, const VulkanYcbcrConversionInfo& conversionInfo) {
    if (!context->vulkanCaps().supportsYcbcrConversion()) {
        return nullptr;
    }

    VkSamplerYcbcrConversionCreateInfo ycbcrCreateInfo;
    std::optional<VkFilter> requiredSamplerFilter;
    conversionInfo.toVkSamplerYcbcrConversionCreateInfo(&ycbcrCreateInfo, &requiredSamplerFilter);

#ifdef SK_BUILD_FOR_ANDROID
    VkExternalFormatANDROID externalFormat;
    if (conversionInfo.hasExternalFormat()) {
        // Format must not be specified for external images.
        SkASSERT(conversionInfo.fFormat == VK_FORMAT_UNDEFINED);
        externalFormat.sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID;
        externalFormat.pNext = nullptr;
        externalFormat.externalFormat = conversionInfo.externalFormat();
        SkASSERT(ycbcrCreateInfo.pNext == nullptr);
        ycbcrCreateInfo.pNext = &externalFormat;
    }
#else
    // External images are supported only on Android.
    SkASSERT(!conversionInfo.hasExternalFormat());
#endif

    if (!conversionInfo.hasExternalFormat()) {
        SkASSERT(conversionInfo.format() != VK_FORMAT_UNDEFINED);
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

    static_assert(kSupportsLinearFilterShift + kSupportsLinearFilterBits <= 32);

    SkASSERT(conversionInfo.model()                       < (1u << kYcbcrModelBits          ));
    SkASSERT(conversionInfo.range()                       < (1u << kYcbcrRangeBits          ));
    SkASSERT(conversionInfo.xChromaOffset()               < (1u << kXChromaOffsetBits       ));
    SkASSERT(conversionInfo.yChromaOffset()               < (1u << kYChromaOffsetBits       ));
    SkASSERT(conversionInfo.chromaFilter()                < (1u << kChromaFilterBits        ));
    SkASSERT(conversionInfo.forceExplicitReconstruction() < (1u << kForceExplicitReconBits  ));
    SkASSERT(conversionInfo.components().r                < (1u << kComponentBits           ));
    SkASSERT(conversionInfo.components().g                < (1u << kComponentBits           ));
    SkASSERT(conversionInfo.components().b                < (1u << kComponentBits           ));
    SkASSERT(conversionInfo.components().a                < (1u << kComponentBits           ));

    const bool usesExternalFormat = conversionInfo.hasExternalFormat();
    const bool matchChroma = conversionInfo.fSamplerFilterMustMatchChromaFilter;
    const bool supportsLinear = conversionInfo.fSupportsLinearFilter;

    ImmutableSamplerInfo info;
    info.fNonFormatYcbcrConversionInfo =
        (((uint32_t)(usesExternalFormat                          ) << kUsesExternalFormatShift  ) |
         ((uint32_t)(conversionInfo.model()                      ) << kYcbcrModelShift          ) |
         ((uint32_t)(conversionInfo.range()                      ) << kYcbcrRangeShift          ) |
         ((uint32_t)(conversionInfo.xChromaOffset()              ) << kXChromaOffsetShift       ) |
         ((uint32_t)(conversionInfo.yChromaOffset()              ) << kYChromaOffsetShift       ) |
         ((uint32_t)(conversionInfo.chromaFilter()               ) << kChromaFilterShift        ) |
         ((uint32_t)(conversionInfo.forceExplicitReconstruction()) << kForceExplicitReconShift  ) |
         ((uint32_t)(conversionInfo.components().r               ) << kComponentRShift          ) |
         ((uint32_t)(conversionInfo.components().g               ) << kComponentGShift          ) |
         ((uint32_t)(conversionInfo.components().b               ) << kComponentBShift          ) |
         ((uint32_t)(conversionInfo.components().a               ) << kComponentAShift          ) |
         ((uint32_t)(matchChroma                                 ) << kMatchChromaFilterShift   ) |
         ((uint32_t)(supportsLinear                              ) << kSupportsLinearFilterShift));

    SkASSERT(info.fNonFormatYcbcrConversionInfo >> SamplerDesc::kMaxNumConversionInfoBits == 0);

    info.fFormat = usesExternalFormat ? conversionInfo.externalFormat()
                                      : static_cast<uint64_t>(conversionInfo.format());
    return info;
}

VulkanYcbcrConversionInfo VulkanYcbcrConversion::FromImmutableSamplerInfo(
        ImmutableSamplerInfo info) {
    const uint32_t nonFormatInfo = info.fNonFormatYcbcrConversionInfo;

    VkFormat format = VK_FORMAT_UNDEFINED;
    uint64_t externalFormat = 0;

    const bool usesExternalFormat =
            (nonFormatInfo >> kUsesExternalFormatShift) & kUseExternalFormatMask;
    if (usesExternalFormat) {
        externalFormat = info.fFormat;
    } else {
        format = static_cast<VkFormat>(info.fFormat);
    }

    VkSamplerYcbcrModelConversion model =
            static_cast<VkSamplerYcbcrModelConversion>(
                    (nonFormatInfo & kYcbcrModelMask) >> kYcbcrModelShift);
    VkSamplerYcbcrRange range =
            static_cast<VkSamplerYcbcrRange>(
                    (nonFormatInfo & kYcbcrRangeMask) >> kYcbcrRangeShift);
    VkComponentMapping components =
            { static_cast<VkComponentSwizzle>(
                      (nonFormatInfo & kComponentRMask) >> kComponentRShift),
              static_cast<VkComponentSwizzle>(
                      (nonFormatInfo & kComponentGMask) >> kComponentGShift),
              static_cast<VkComponentSwizzle>(
                      (nonFormatInfo & kComponentBMask) >> kComponentBShift),
              static_cast<VkComponentSwizzle>(
                      (nonFormatInfo & kComponentAMask) >> kComponentAShift) };
    VkChromaLocation xChromaOffset =
            static_cast<VkChromaLocation>(
                    (nonFormatInfo & kXChromaOffsetMask) >> kXChromaOffsetShift);
    VkChromaLocation yChromaOffset =
            static_cast<VkChromaLocation>(
                    (nonFormatInfo & kYChromaOffsetMask) >> kYChromaOffsetShift);
    VkFilter chromaFilter = static_cast<VkFilter>(
                                    (nonFormatInfo & kChromaFilterMask) >> kChromaFilterShift);
    VkBool32 forceExplicitReconstruction =
            static_cast<VkBool32>(
                    (nonFormatInfo & kForceExplicitReconMask) >> kForceExplicitReconShift);

    bool chromaMustMatch =
            static_cast<bool>((nonFormatInfo & kMatchChromaFilterMask) >> kMatchChromaFilterShift);
    bool supportsLinearFilter =
            static_cast<bool>(
                    (nonFormatInfo & kSupportsLinearFilterMask) >> kSupportsLinearFilterShift);


    VulkanYcbcrConversionInfo vkInfo(format, externalFormat, model, range, xChromaOffset,
                                     yChromaOffset, chromaFilter, forceExplicitReconstruction,
                                     components, chromaMustMatch, supportsLinearFilter);

    return vkInfo;
}

VulkanYcbcrConversion::VulkanYcbcrConversion(const VulkanSharedContext* context,
                                             VkSamplerYcbcrConversion ycbcrConversion,
                                             std::optional<VkFilter> requiredFilter)
        : Resource(context,
                   Ownership::kOwned,
                   /*gpuMemorySize=*/0,
                   /*reusableRequiresPurgeable=*/false)
        , fYcbcrConversion(ycbcrConversion)
        , fRequiredFilter(requiredFilter) {}

void VulkanYcbcrConversion::freeGpuData() {
    auto sharedContext = static_cast<const VulkanSharedContext*>(this->sharedContext());
    SkASSERT(fYcbcrConversion != VK_NULL_HANDLE);
    VULKAN_CALL(sharedContext->interface(),
                DestroySamplerYcbcrConversion(sharedContext->device(), fYcbcrConversion, nullptr));
}

} // namespace skgpu::graphite
