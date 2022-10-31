/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanCaps.h"

#include "include/gpu/graphite/TextureInfo.h"
#include "include/gpu/vk/VulkanExtensions.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtils.h"

namespace skgpu::graphite {

namespace { // anonymous namespace
    bool is_renderable(const VkFormatFeatureFlags& flags) {
        return SkToBool(VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT & flags);
    }

    bool is_texturable(const VkFormatFeatureFlags& flags) {
        return SkToBool(VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT & flags) &&
               SkToBool(VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT & flags);
    }
} // end of anonymous namespace

VulkanCaps::VulkanCaps(const skgpu::VulkanInterface* vkInterface,
                       VkPhysicalDevice physDev,
                       uint32_t physicalDeviceVersion,
                       const skgpu::VulkanExtensions* extensions) :
    Caps() {
    VkPhysicalDeviceProperties physDevProperties;
    VULKAN_CALL(vkInterface, GetPhysicalDeviceProperties(physDev, &physDevProperties));
    this->initFormatTable(vkInterface, physDev, physDevProperties);
}

VulkanCaps::~VulkanCaps() {}

TextureInfo VulkanCaps::getDefaultSampledTextureInfo(SkColorType,
                                                     Mipmapped mipmapped,
                                                     Protected,
                                                     Renderable) const {
    return {};
}

TextureInfo VulkanCaps::getDefaultMSAATextureInfo(const TextureInfo& singleSampledInfo,
                                                  Discardable discardable) const {
    return {};
}

TextureInfo VulkanCaps::getDefaultDepthStencilTextureInfo(SkEnumBitMask<DepthStencilFlags>,
                                                          uint32_t sampleCount,
                                                          Protected) const {
    return {};
}

// These are all the valid VkFormats that we support in Skia. They are roughly ordered from most
// frequently used to least to improve look up times in arrays.
static constexpr VkFormat kVkFormats[] = {
    VK_FORMAT_R8G8B8A8_UNORM,
    VK_FORMAT_R8_UNORM,
    VK_FORMAT_B8G8R8A8_UNORM,
    VK_FORMAT_R5G6B5_UNORM_PACK16,
    VK_FORMAT_R16G16B16A16_SFLOAT,
    VK_FORMAT_R16_SFLOAT,
    VK_FORMAT_R8G8B8_UNORM,
    VK_FORMAT_R8G8_UNORM,
    VK_FORMAT_A2B10G10R10_UNORM_PACK32,
    VK_FORMAT_A2R10G10B10_UNORM_PACK32,
    VK_FORMAT_B4G4R4A4_UNORM_PACK16,
    VK_FORMAT_R4G4B4A4_UNORM_PACK16,
    VK_FORMAT_R8G8B8A8_SRGB,
    VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK,
    VK_FORMAT_BC1_RGB_UNORM_BLOCK,
    VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
    VK_FORMAT_R16_UNORM,
    VK_FORMAT_R16G16_UNORM,
    VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM,
    VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
    VK_FORMAT_R16G16B16A16_UNORM,
    VK_FORMAT_R16G16_SFLOAT,
};

void VulkanCaps::FormatInfo::initSampleCounts(const skgpu::VulkanInterface* interface,
                                              VkPhysicalDevice physDev,
                                              const VkPhysicalDeviceProperties& physProps,
                                              VkFormat format) {
    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                              VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                              VK_IMAGE_USAGE_SAMPLED_BIT |
                              VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkImageFormatProperties properties;
    VULKAN_CALL(interface, GetPhysicalDeviceImageFormatProperties(physDev,
                                                                 format,
                                                                 VK_IMAGE_TYPE_2D,
                                                                 VK_IMAGE_TILING_OPTIMAL,
                                                                 usage,
                                                                 0,  // createFlags
                                                                 &properties));
    VkSampleCountFlags flags = properties.sampleCounts;
    if (flags & VK_SAMPLE_COUNT_1_BIT) {
        fColorSampleCounts.push_back(1);
    }
    if (kImagination_VkVendor == physProps.vendorID) {
        // MSAA does not work on imagination
        return;
    }
    if (kIntel_VkVendor == physProps.vendorID) {
        // MSAA doesn't work well on Intel GPUs chromium:527565, chromium:983926
        return;
    }
    if (flags & VK_SAMPLE_COUNT_2_BIT) {
        fColorSampleCounts.push_back(2);
    }
    if (flags & VK_SAMPLE_COUNT_4_BIT) {
        fColorSampleCounts.push_back(4);
    }
    if (flags & VK_SAMPLE_COUNT_8_BIT) {
        fColorSampleCounts.push_back(8);
    }
    if (flags & VK_SAMPLE_COUNT_16_BIT) {
        fColorSampleCounts.push_back(16);
    }
    // Standard sample locations are not defined for more than 16 samples, and we don't need more
    // than 16. Omit 32 and 64.
}

void VulkanCaps::FormatInfo::init(const skgpu::VulkanInterface* interface,
                                  VkPhysicalDevice physDev,
                                  const VkPhysicalDeviceProperties& properties,
                                  VkFormat format) {
    memset(&fFormatProperties, 0, sizeof(VkFormatProperties));
    VULKAN_CALL(interface, GetPhysicalDeviceFormatProperties(physDev, format, &fFormatProperties));

    if (is_renderable(fFormatProperties.optimalTilingFeatures)) {
        this->initSampleCounts(interface, physDev, properties, format);
    }
}

void VulkanCaps::initFormatTable(const skgpu::VulkanInterface* interface,
                                 VkPhysicalDevice physDev,
                                 const VkPhysicalDeviceProperties& properties) {
    static_assert(std::size(kVkFormats) == VulkanCaps::kNumVkFormats,
                  "Size of VkFormats array must match static value in header");

    std::fill_n(fColorTypeToFormatTable, kSkColorTypeCnt, VK_FORMAT_UNDEFINED);

    // Go through all the formats and init their support surface and data ColorTypes.
    // Format: VK_FORMAT_R8G8B8A8_UNORM
    {
        constexpr VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        if (is_texturable(info.fFormatProperties.optimalTilingFeatures)) {
            info.fColorTypeInfoCount = 2;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R8G8B8A8_UNORM, Surface: kRGBA_8888
            {
                constexpr SkColorType ct = SkColorType::kRGBA_8888_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
            // Format: VK_FORMAT_R8G8B8A8_UNORM, Surface: kRGB_888x
            {
                constexpr SkColorType ct = SkColorType::kRGB_888x_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag;
                ctInfo.fReadSwizzle = skgpu::Swizzle::RGB1();
            }
        }
    }

    // Format: VK_FORMAT_R8_UNORM
    {
        constexpr VkFormat format = VK_FORMAT_R8_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        if (is_texturable(info.fFormatProperties.optimalTilingFeatures)) {
            info.fColorTypeInfoCount = 3;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R8_UNORM, Surface: kR_8
            {
                constexpr SkColorType ct = SkColorType::kR8_unorm_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
            // Format: VK_FORMAT_R8_UNORM, Surface: kAlpha_8
            {
                constexpr SkColorType ct = SkColorType::kAlpha_8_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
                ctInfo.fReadSwizzle = skgpu::Swizzle("000r");
                ctInfo.fWriteSwizzle = skgpu::Swizzle("a000");
            }
            // Format: VK_FORMAT_R8_UNORM, Surface: kGray_8
            {
                constexpr SkColorType ct = SkColorType::kGray_8_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag;
                ctInfo.fReadSwizzle = skgpu::Swizzle("rrr1");
            }
        }
    }

    // Format: VK_FORMAT_B8G8R8A8_UNORM
    {
        constexpr VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        if (is_texturable(info.fFormatProperties.optimalTilingFeatures)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_B8G8R8A8_UNORM, Surface: kBGRA_8888
            {
                constexpr SkColorType ct = SkColorType::kBGRA_8888_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_R5G6B5_UNORM_PACK16
    {
        constexpr VkFormat format = VK_FORMAT_R5G6B5_UNORM_PACK16;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        if (is_texturable(info.fFormatProperties.optimalTilingFeatures)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R5G6B5_UNORM_PACK16, Surface: kRGB_565_SkColorType
            {
                constexpr SkColorType ct = SkColorType::kRGB_565_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_R16G16B16A16_SFLOAT
    {
        constexpr VkFormat format = VK_FORMAT_R16G16B16A16_SFLOAT;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        if (is_texturable(info.fFormatProperties.optimalTilingFeatures)) {
            info.fColorTypeInfoCount = 2;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R16G16B16A16_SFLOAT, Surface: SkColorType::kRGBA_F16_SkColorType
            {
                constexpr SkColorType ct = SkColorType::kRGBA_F16_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_R16_SFLOAT
    {
        constexpr VkFormat format = VK_FORMAT_R16_SFLOAT;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        if (is_texturable(info.fFormatProperties.optimalTilingFeatures)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R16_SFLOAT, Surface: kAlpha_F16
            {
                constexpr SkColorType ct = SkColorType::kA16_float_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
                ctInfo.fReadSwizzle = skgpu::Swizzle("000r");
                ctInfo.fWriteSwizzle = skgpu::Swizzle("a000");
            }
        }
    }
    // Format: VK_FORMAT_R8G8B8_UNORM
    {
        constexpr VkFormat format = VK_FORMAT_R8G8B8_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        if (is_texturable(info.fFormatProperties.optimalTilingFeatures)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R8G8B8_UNORM, Surface: kRGB_888x
            {
                constexpr SkColorType ct = SkColorType::kRGB_888x_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                // The Vulkan format is 3 bpp so we must convert to/from that when transferring.
                ctInfo.fTransferColorType = SkColorType::kRGB_888x_SkColorType;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_R8G8_UNORM
    {
        constexpr VkFormat format = VK_FORMAT_R8G8_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        if (is_texturable(info.fFormatProperties.optimalTilingFeatures)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R8G8_UNORM, Surface: kR8G8_unorm
            {
                constexpr SkColorType ct = SkColorType::kR8G8_unorm_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_A2B10G10R10_UNORM_PACK32
    {
        constexpr VkFormat format = VK_FORMAT_A2B10G10R10_UNORM_PACK32;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        if (is_texturable(info.fFormatProperties.optimalTilingFeatures)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_A2B10G10R10_UNORM_PACK32, Surface: kRGBA_1010102
            {
                constexpr SkColorType ct = SkColorType::kRGBA_1010102_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_A2R10G10B10_UNORM_PACK32
    {
        constexpr VkFormat format = VK_FORMAT_A2R10G10B10_UNORM_PACK32;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        if (is_texturable(info.fFormatProperties.optimalTilingFeatures)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_A2R10G10B10_UNORM_PACK32, Surface: kBGRA_1010102
            {
                constexpr SkColorType ct = SkColorType::kBGRA_1010102_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_B4G4R4A4_UNORM_PACK16
    {
        constexpr VkFormat format = VK_FORMAT_B4G4R4A4_UNORM_PACK16;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        if (is_texturable(info.fFormatProperties.optimalTilingFeatures)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_B4G4R4A4_UNORM_PACK16, Surface: kARGB_4444_SkColorType
            {
                constexpr SkColorType ct = SkColorType::kARGB_4444_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
                ctInfo.fReadSwizzle = skgpu::Swizzle::BGRA();
                ctInfo.fWriteSwizzle = skgpu::Swizzle::BGRA();
            }
        }
    }

    // Format: VK_FORMAT_R4G4B4A4_UNORM_PACK16
    {
        constexpr VkFormat format = VK_FORMAT_R4G4B4A4_UNORM_PACK16;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        if (is_texturable(info.fFormatProperties.optimalTilingFeatures)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R4G4B4A4_UNORM_PACK16, Surface: kARGB_4444_SkColorType
            {
                constexpr SkColorType ct = SkColorType::kARGB_4444_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_R8G8B8A8_SRGB
    {
        constexpr VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        if (is_texturable(info.fFormatProperties.optimalTilingFeatures)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R8G8B8A8_SRGB, Surface: kRGBA_8888_SRGB
            {
                constexpr SkColorType ct = SkColorType::kSRGBA_8888_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_R16_UNORM
    {
        constexpr VkFormat format = VK_FORMAT_R16_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        if (is_texturable(info.fFormatProperties.optimalTilingFeatures)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R16_UNORM, Surface: kAlpha_16
            {
                constexpr SkColorType ct = SkColorType::kA16_unorm_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
                ctInfo.fReadSwizzle = skgpu::Swizzle("000r");
                ctInfo.fWriteSwizzle = skgpu::Swizzle("a000");
            }
        }
    }
    // Format: VK_FORMAT_R16G16_UNORM
    {
        constexpr VkFormat format = VK_FORMAT_R16G16_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        if (is_texturable(info.fFormatProperties.optimalTilingFeatures)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R16G16_UNORM, Surface: kRG_1616
            {
                constexpr SkColorType ct = SkColorType::kR16G16_unorm_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_R16G16B16A16_UNORM
    {
        constexpr VkFormat format = VK_FORMAT_R16G16B16A16_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        if (is_texturable(info.fFormatProperties.optimalTilingFeatures)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R16G16B16A16_UNORM, Surface: kRGBA_16161616
            {
                constexpr SkColorType ct = SkColorType::kRGBA_F16_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_R16G16_SFLOAT
    {
        constexpr VkFormat format = VK_FORMAT_R16G16_SFLOAT;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        if (is_texturable(info.fFormatProperties.optimalTilingFeatures)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R16G16_SFLOAT, Surface: kRG_F16
            {
                constexpr SkColorType ct = SkColorType::kR16G16_float_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM
    {
        constexpr VkFormat format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
        auto& info = this->getFormatInfo(format);
        if (fSupportsYcbcrConversion) {
            info.init(interface, physDev, properties, format);
            SkDEBUGCODE(info.fIsWrappedOnly = true;)
        }
        if (is_texturable(info.fFormatProperties.optimalTilingFeatures)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM, Surface: kRGB_888x
            {
                constexpr SkColorType ct = SkColorType::kRGB_888x_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag;
            }
        }
    }
    // Format: VK_FORMAT_G8_B8R8_2PLANE_420_UNORM
    {
        constexpr VkFormat format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
        auto& info = this->getFormatInfo(format);
        if (fSupportsYcbcrConversion) {
            info.init(interface, physDev, properties, format);
            SkDEBUGCODE(info.fIsWrappedOnly = true;)
        }
        if (is_texturable(info.fFormatProperties.optimalTilingFeatures)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, Surface: kRGB_888x
            {
                constexpr SkColorType ct = SkColorType::kRGB_888x_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag;
            }
        }
    }
    // Format: VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK
    {
        constexpr VkFormat format = VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        // Setting this to texel block size
        // No supported SkColorType.
    }

    // Format: VK_FORMAT_BC1_RGB_UNORM_BLOCK
    {
        constexpr VkFormat format = VK_FORMAT_BC1_RGB_UNORM_BLOCK;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        // Setting this to texel block size
        // No supported SkColorType.
    }

    // Format: VK_FORMAT_BC1_RGBA_UNORM_BLOCK
    {
        constexpr VkFormat format = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        // Setting this to texel block size
        // No supported SkColorType.
    }

    ////////////////////////////////////////////////////////////////////////////
    // Map SkColorType (used for creating Surfaces) to VkFormats. The order in which the formats are
    // passed into the setColorType function indicates the priority in selecting which format we use
    // for a given SkColorType.

    typedef SkColorType ct;

    this->setColorType(ct::kAlpha_8_SkColorType,            { VK_FORMAT_R8_UNORM });
    this->setColorType(ct::kRGB_565_SkColorType,            { VK_FORMAT_R5G6B5_UNORM_PACK16 });
    this->setColorType(ct::kARGB_4444_SkColorType,          { VK_FORMAT_R4G4B4A4_UNORM_PACK16,
                                                              VK_FORMAT_B4G4R4A4_UNORM_PACK16 });
    this->setColorType(ct::kRGBA_8888_SkColorType,          { VK_FORMAT_R8G8B8A8_UNORM });
    this->setColorType(ct::kSRGBA_8888_SkColorType,         { VK_FORMAT_R8G8B8A8_SRGB });
    this->setColorType(ct::kRGB_888x_SkColorType,           { VK_FORMAT_R8G8B8_UNORM,
                                                              VK_FORMAT_R8G8B8A8_UNORM });
    this->setColorType(ct::kR8G8_unorm_SkColorType,         { VK_FORMAT_R8G8_UNORM });
    this->setColorType(ct::kBGRA_8888_SkColorType,          { VK_FORMAT_B8G8R8A8_UNORM });
    this->setColorType(ct::kRGBA_1010102_SkColorType,       { VK_FORMAT_A2B10G10R10_UNORM_PACK32 });
    this->setColorType(ct::kBGRA_1010102_SkColorType,       { VK_FORMAT_A2R10G10B10_UNORM_PACK32 });
    this->setColorType(ct::kGray_8_SkColorType,             { VK_FORMAT_R8_UNORM });
    this->setColorType(ct::kA16_float_SkColorType,          { VK_FORMAT_R16_SFLOAT });
    this->setColorType(ct::kRGBA_F16_SkColorType,           { VK_FORMAT_R16G16B16A16_SFLOAT });
    this->setColorType(ct::kA16_unorm_SkColorType,          { VK_FORMAT_R16_UNORM });
    this->setColorType(ct::kR16G16_unorm_SkColorType,       { VK_FORMAT_R16G16_UNORM });
    this->setColorType(ct::kR16G16B16A16_unorm_SkColorType, { VK_FORMAT_R16G16B16A16_UNORM });
    this->setColorType(ct::kR16G16_float_SkColorType,       { VK_FORMAT_R16G16_SFLOAT });
}

void VulkanCaps::setColorType(SkColorType colorType, std::initializer_list<VkFormat> formats) {
#ifdef SK_DEBUG
    // If any format's FormatInfo claims support for the passed-in colorType, ensure that format is
    // included in the provided formats list.
    for (size_t i = 0; i < kNumVkFormats; ++i) {
        const auto& formatInfo = fFormatTable[i];
        for (int j = 0; j < formatInfo.fColorTypeInfoCount; ++j) {
            const auto& ctInfo = formatInfo.fColorTypeInfos[j];
            if (ctInfo.fColorType == colorType && !formatInfo.fIsWrappedOnly) {
                bool found = false;
                for (auto it = formats.begin(); it != formats.end(); ++it) {
                    if (kVkFormats[i] == *it) {
                        found = true;
                    }
                }
                SkASSERT(found);
            }
        }
    }
#endif
    int idx = static_cast<int>(colorType);
    for (auto it = formats.begin(); it != formats.end(); ++it) {
        const auto& info = this->getFormatInfo(*it);
        for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
            if (info.fColorTypeInfos[i].fColorType == colorType) {
                fColorTypeToFormatTable[idx] = *it;
                return;
            }
        }
    }
}

const VulkanCaps::FormatInfo& VulkanCaps::getFormatInfo(VkFormat format) const {
    VulkanCaps* nonConstThis = const_cast<VulkanCaps*>(this);
    return nonConstThis->getFormatInfo(format);
}

VulkanCaps::FormatInfo& VulkanCaps::getFormatInfo(VkFormat format) {
    static_assert(std::size(kVkFormats) == VulkanCaps::kNumVkFormats,
                  "Size of VkFormats array must match static value in header");
    for (size_t i = 0; i < std::size(kVkFormats); ++i) {
        if (kVkFormats[i] == format) {
            return fFormatTable[i];
        }
    }
    static FormatInfo kInvalidFormat;
    return kInvalidFormat;
}

} // namespace skgpu::graphite

