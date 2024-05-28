/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanCaps.h"

#include "include/core/SkTextureCompressionType.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/gpu/graphite/vk/VulkanGraphiteTypes.h"
#include "include/gpu/vk/VulkanExtensions.h"
#include "include/gpu/vk/VulkanTypes.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/GraphiteResourceKey.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/RuntimeEffectDictionary.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"
#include "src/gpu/graphite/vk/VulkanRenderPass.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"
#include "src/gpu/graphite/vk/VulkanYcbcrConversion.h"
#include "src/gpu/vk/VulkanUtilsPriv.h"

#ifdef SK_BUILD_FOR_ANDROID
#include <sys/system_properties.h>
#endif

namespace skgpu::graphite {

VulkanCaps::VulkanCaps(const ContextOptions& contextOptions,
                       const skgpu::VulkanInterface* vkInterface,
                       VkPhysicalDevice physDev,
                       uint32_t physicalDeviceVersion,
                       const VkPhysicalDeviceFeatures2* features,
                       const skgpu::VulkanExtensions* extensions,
                       Protected isProtected)
        : Caps() {
    this->init(contextOptions, vkInterface, physDev, physicalDeviceVersion, features, extensions,
               isProtected);
}

VulkanCaps::~VulkanCaps() {}

void VulkanCaps::init(const ContextOptions& contextOptions,
                      const skgpu::VulkanInterface* vkInterface,
                      VkPhysicalDevice physDev,
                      uint32_t physicalDeviceVersion,
                      const VkPhysicalDeviceFeatures2* features,
                      const skgpu::VulkanExtensions* extensions,
                      Protected isProtected) {
    VkPhysicalDeviceProperties physDevProperties;
    VULKAN_CALL(vkInterface, GetPhysicalDeviceProperties(physDev, &physDevProperties));

#if defined(GRAPHITE_TEST_UTILS)
    this->setDeviceName(physDevProperties.deviceName);
#endif

    // Graphite requires Vulkan version 1.1 or later, which always has protected support.
    if (isProtected == Protected::kYes) {
        fProtectedSupport = true;
        fShouldAlwaysUseDedicatedImageMemory = true;
    }

    fPhysicalDeviceMemoryProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
    fPhysicalDeviceMemoryProperties2.pNext = nullptr;
    VULKAN_CALL(vkInterface,
                GetPhysicalDeviceMemoryProperties2(physDev, &fPhysicalDeviceMemoryProperties2));

    // We could actually query and get a max size for each config, however maxImageDimension2D will
    // give the minimum max size across all configs. So for simplicity we will use that for now.
    fMaxTextureSize = std::min(physDevProperties.limits.maxImageDimension2D, (uint32_t)INT_MAX);

    fRequiredUniformBufferAlignment = physDevProperties.limits.minUniformBufferOffsetAlignment;
    fRequiredStorageBufferAlignment =  physDevProperties.limits.minStorageBufferOffsetAlignment;
    fRequiredTransferBufferAlignment = 4;

    fResourceBindingReqs.fUniformBufferLayout = Layout::kStd140;
    // TODO(skia:14639): We cannot use std430 layout for SSBOs until SkSL gracefully handles
    // implicit array stride.
    fResourceBindingReqs.fStorageBufferLayout = Layout::kStd140;
    fResourceBindingReqs.fSeparateTextureAndSamplerBinding = false;
    fResourceBindingReqs.fDistinctIndexRanges = false;

    VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
    VULKAN_CALL(vkInterface, GetPhysicalDeviceMemoryProperties(physDev, &deviceMemoryProperties));
    fSupportsMemorylessAttachments = false;
    VkMemoryPropertyFlags requiredLazyFlags = VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
    if (fProtectedSupport) {
        // If we have a protected context we can only use memoryless images if they also support
        // being protected. With current devices we don't actually expect this combination to be
        // supported, but this at least covers us for future devices that may allow it.
        requiredLazyFlags |= VK_MEMORY_PROPERTY_PROTECTED_BIT;
    }
    for (uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; ++i) {
        if (deviceMemoryProperties.memoryTypes[i].propertyFlags & requiredLazyFlags) {
            fSupportsMemorylessAttachments = true;
        }
    }

#ifdef SK_BUILD_FOR_UNIX
    if (kNvidia_VkVendor == physDevProperties.vendorID) {
        // On NVIDIA linux we see a big perf regression when not using dedicated image allocations.
        fShouldAlwaysUseDedicatedImageMemory = true;
    }
#endif

    if (physDevProperties.vendorID == kNvidia_VkVendor ||
        physDevProperties.vendorID == kAMD_VkVendor) {
        // On discrete GPUs, it can be faster to read gpu-only memory compared to memory that is
        // also mappable on the host.
        fGpuOnlyBuffersMorePerformant = true;

        // On discrete GPUs we try to use special DEVICE_LOCAL and HOST_VISIBLE memory for our
        // cpu write, gpu read buffers. This memory is not ideal to be kept persistently mapped.
        // Some discrete GPUs do not expose this special memory, however we still disable
        // persistently mapped buffers for all of them since most GPUs with updated drivers do
        // expose it. If this becomes an issue we can try to be more fine grained.
        fShouldPersistentlyMapCpuToGpuBuffers = false;
    }

    if (!contextOptions.fDisableDriverCorrectnessWorkarounds) {
        this->applyDriverCorrectnessWorkarounds(physDevProperties);
    }

    if (physDevProperties.vendorID == kAMD_VkVendor) {
        // AMD advertises support for MAX_UINT vertex attributes but in reality only supports 32.
        fMaxVertexAttributes = 32;
    } else {
        fMaxVertexAttributes = physDevProperties.limits.maxVertexInputAttributes;
    }
    fMaxUniformBufferRange = physDevProperties.limits.maxUniformBufferRange;

#ifdef SK_BUILD_FOR_ANDROID
    if (extensions->hasExtension(
            VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME, 2)) {
        fSupportsAHardwareBufferImages = true;
    }
#endif

    // Determine whether the client enabled certain physical device features.
    if (features) {
        auto ycbcrFeatures =
                skgpu::GetExtensionFeatureStruct<VkPhysicalDeviceSamplerYcbcrConversionFeatures>(
                        *features,
                        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES);
        if (ycbcrFeatures && ycbcrFeatures->samplerYcbcrConversion) {
            fSupportsYcbcrConversion = true;
        }
    }

    if (extensions->hasExtension(VK_EXT_DEVICE_FAULT_EXTENSION_NAME, 1)) {
        fSupportsDeviceFaultInfo = true;
    }

    // Note that format table initialization should be performed at the end of this method to ensure
    // all capability determinations are completed prior to populating the format tables.
    this->initFormatTable(vkInterface, physDev, physDevProperties);
    this->initDepthStencilFormatTable(vkInterface, physDev, physDevProperties);

    this->finishInitialization(contextOptions);
}

void VulkanCaps::applyDriverCorrectnessWorkarounds(const VkPhysicalDeviceProperties& properties) {
    // By default, we initialize the Android API version to 0 since we consider certain things
    // "fixed" only once above a certain version. This way, we default to enabling the workarounds.
    int androidAPIVersion = 0;
#if defined(SK_BUILD_FOR_ANDROID)
    char androidAPIVersionStr[PROP_VALUE_MAX];
    int strLength = __system_property_get("ro.build.version.sdk", androidAPIVersionStr);
    // Defaults to zero since most checks care if it is greater than a specific value. So this will
    // just default to it being less.
    androidAPIVersion = (strLength == 0) ? 0 : atoi(androidAPIVersionStr);
#endif

    // On Mali galaxy s7 we see lots of rendering issues when we suballocate VkImages.
    if (kARM_VkVendor == properties.vendorID && androidAPIVersion <= 28) {
        fShouldAlwaysUseDedicatedImageMemory = true;
    }
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
// These are all the valid depth/stencil formats that we support in Skia.
static constexpr VkFormat kDepthStencilVkFormats[] = {
    VK_FORMAT_S8_UINT,
    VK_FORMAT_D16_UNORM,
    VK_FORMAT_D32_SFLOAT,
    VK_FORMAT_D24_UNORM_S8_UINT,
    VK_FORMAT_D32_SFLOAT_S8_UINT,
};

TextureInfo VulkanCaps::getDefaultSampledTextureInfo(SkColorType ct,
                                                     Mipmapped mipmapped,
                                                     Protected isProtected,
                                                     Renderable isRenderable) const {
    VkFormat format = this->getFormatFromColorType(ct);
    const FormatInfo& formatInfo = this->getFormatInfo(format);
    static constexpr int defaultSampleCount = 1;
    if ((isProtected == Protected::kYes && !this->protectedSupport()) ||
        !formatInfo.isTexturable(VK_IMAGE_TILING_OPTIMAL) ||
        (isRenderable == Renderable::kYes &&
         !formatInfo.isRenderable(VK_IMAGE_TILING_OPTIMAL, defaultSampleCount)) ) {
        return {};
    }

    VulkanTextureInfo info;
    info.fSampleCount = defaultSampleCount;
    info.fMipmapped = mipmapped;
    info.fFlags = (isProtected == Protected::kYes) ? VK_IMAGE_CREATE_PROTECTED_BIT : 0;
    info.fFormat = format;
    info.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
    info.fImageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT |
                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                            VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (isRenderable == Renderable::kYes) {
        // We make all renderable images support being used as input attachment
        info.fImageUsageFlags = info.fImageUsageFlags |
                                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    }
    info.fSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.fAspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    return info;
}

TextureInfo VulkanCaps::getTextureInfoForSampledCopy(const TextureInfo& textureInfo,
                                                     Mipmapped mipmapped) const {
    VulkanTextureInfo info;
    if (!textureInfo.getVulkanTextureInfo(&info)) {
        return {};
    }

    info.fSampleCount = 1;
    info.fMipmapped = mipmapped;
    info.fFlags = (textureInfo.fProtected == Protected::kYes) ? VK_IMAGE_CREATE_PROTECTED_BIT : 0;
    info.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
    info.fImageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                            VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    info.fSharingMode = VK_SHARING_MODE_EXCLUSIVE;

    return info;
}

namespace {
VkFormat format_from_compression(SkTextureCompressionType compression) {
    switch (compression) {
        case SkTextureCompressionType::kETC2_RGB8_UNORM:
            return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
        case SkTextureCompressionType::kBC1_RGB8_UNORM:
            return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
        case SkTextureCompressionType::kBC1_RGBA8_UNORM:
            return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        default:
            return VK_FORMAT_UNDEFINED;
    }
}
}

TextureInfo VulkanCaps::getDefaultCompressedTextureInfo(SkTextureCompressionType compression,
                                                        Mipmapped mipmapped,
                                                        Protected isProtected) const {
    VkFormat format = format_from_compression(compression);
    const FormatInfo& formatInfo = this->getFormatInfo(format);
    static constexpr int defaultSampleCount = 1;
    if ((isProtected == Protected::kYes && !this->protectedSupport()) ||
        !formatInfo.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
        return {};
    }

    VulkanTextureInfo info;
    info.fSampleCount = defaultSampleCount;
    info.fMipmapped = mipmapped;
    info.fFlags = (isProtected == Protected::kYes) ? VK_IMAGE_CREATE_PROTECTED_BIT : 0;
    info.fFormat = format;
    info.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
    info.fImageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT |
                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                            VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    info.fSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.fAspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    return info;
}

TextureInfo VulkanCaps::getDefaultMSAATextureInfo(const TextureInfo& singleSampledInfo,
                                                  Discardable discardable) const {
    if (fDefaultMSAASamples <= 1) {
        return {};
    }

    const VkFormat singleSpecFormat = singleSampledInfo.vulkanTextureSpec().fFormat;
    const FormatInfo& formatInfo = this->getFormatInfo(singleSpecFormat);
    if ((singleSampledInfo.isProtected() == Protected::kYes && !this->protectedSupport()) ||
        !formatInfo.isRenderable(VK_IMAGE_TILING_OPTIMAL, fDefaultMSAASamples)) {
        return {};
    }

    VulkanTextureInfo info;
    info.fSampleCount = fDefaultMSAASamples;
    info.fMipmapped = Mipmapped::kNo;
    info.fFlags = (singleSampledInfo.isProtected() == Protected::kYes) ?
        VK_IMAGE_CREATE_PROTECTED_BIT : 0;
    info.fFormat = singleSpecFormat;
    info.fImageTiling = VK_IMAGE_TILING_OPTIMAL;

    /**
     * Graphite, unlike ganesh, does not require a dedicated MSAA attachment on every surface.
     * MSAA textures now get resolved within the scope of a render pass, which can be done simply
     * with the color attachment usage flag. So we no longer require transfer src/dst usage flags.
    */
    VkImageUsageFlags flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (discardable == Discardable::kYes && fSupportsMemorylessAttachments) {
        flags = flags | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
    }

    info.fImageUsageFlags = flags;
    info.fSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.fAspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    return info;
}

TextureInfo VulkanCaps::getDefaultDepthStencilTextureInfo(SkEnumBitMask<DepthStencilFlags> flags,
                                                          uint32_t sampleCount,
                                                          Protected isProtected) const {
    VkFormat format = this->getFormatFromDepthStencilFlags(flags);
    const DepthStencilFormatInfo& formatInfo = this->getDepthStencilFormatInfo(format);
    if ( (isProtected == Protected::kYes && !this->protectedSupport()) ||
         !formatInfo.isDepthStencilSupported(formatInfo.fFormatProperties.optimalTilingFeatures) ||
         !formatInfo.fSupportedSampleCounts.isSampleCountSupported(sampleCount)) {
        return {};
    }

    VulkanTextureInfo info;
    info.fSampleCount = sampleCount;
    info.fMipmapped = Mipmapped::kNo;
    info.fFlags = (isProtected == Protected::kYes) ? VK_IMAGE_CREATE_PROTECTED_BIT : 0;
    info.fFormat = format;
    info.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
    // TODO: Passing in a discardable flag to this method, and if true, add the TRANSIENT bit.
    info.fImageUsageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    info.fSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.fAspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    return info;
}

TextureInfo VulkanCaps::getDefaultStorageTextureInfo(SkColorType colorType) const {
    VkFormat format = this->getFormatFromColorType(colorType);
    const FormatInfo& formatInfo = this->getFormatInfo(format);
    if (!formatInfo.isTexturable(VK_IMAGE_TILING_OPTIMAL) ||
        !formatInfo.isStorage(VK_IMAGE_TILING_OPTIMAL)) {
        return {};
    }

    VulkanTextureInfo info;
    info.fSampleCount = 1;
    info.fMipmapped = Mipmapped::kNo;
    info.fFlags = 0;
    info.fFormat = format;
    info.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
    // Storage textures are currently always sampleable from a shader
    info.fImageUsageFlags = VK_IMAGE_USAGE_STORAGE_BIT |
                            VK_IMAGE_USAGE_SAMPLED_BIT |
                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    info.fSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.fAspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    return info;
}

uint32_t VulkanCaps::channelMask(const TextureInfo& textureInfo) const {
    return skgpu::VkFormatChannels(textureInfo.vulkanTextureSpec().fFormat);
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
         if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
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
         if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
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
         if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
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
         if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
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
         if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
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
         if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
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
         if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R8G8B8_UNORM, Surface: kRGB_888x
            {
                constexpr SkColorType ct = SkColorType::kRGB_888x_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                // This SkColorType is a lie, but we don't have a kRGB_888_SkColorType. The Vulkan
                // format is 3 bpp so we must manualy convert to/from this and kRGB_888x when doing
                // transfers. We signal this need for manual conversions in the
                // supportedRead/WriteColorType calls.
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
         if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
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
         if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
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
         if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
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
         if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
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
         if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
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
         if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
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
         if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
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
         if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
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
         if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R16G16B16A16_UNORM, Surface: kRGBA_16161616
            {
                constexpr SkColorType ct = SkColorType::kR16G16B16A16_unorm_SkColorType;
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
         if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
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
        }
        if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
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
            SkDEBUGCODE(info.fIsWrappedOnly = true;)
        }
    }
    // Format: VK_FORMAT_G8_B8R8_2PLANE_420_UNORM
    {
        constexpr VkFormat format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
        auto& info = this->getFormatInfo(format);
        if (fSupportsYcbcrConversion) {
            info.init(interface, physDev, properties, format);
        }
        if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
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
            SkDEBUGCODE(info.fIsWrappedOnly = true;)
        }
    }
    // Format: VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK
    {
        constexpr VkFormat format = VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK, Surface: kRGB_888x
            {
                constexpr SkColorType ct = SkColorType::kRGB_888x_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag;
            }
        }
    }

    // Format: VK_FORMAT_BC1_RGB_UNORM_BLOCK
    {
        constexpr VkFormat format = VK_FORMAT_BC1_RGB_UNORM_BLOCK;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_BC1_RGB_UNORM_BLOCK, Surface: kRGB_888x
            {
                constexpr SkColorType ct = SkColorType::kRGB_888x_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag;
            }
        }
    }

    // Format: VK_FORMAT_BC1_RGBA_UNORM_BLOCK
    {
        constexpr VkFormat format = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_BC1_RGBA_UNORM_BLOCK, Surface: kRGB_888x
            {
                constexpr SkColorType ct = SkColorType::kRGBA_8888_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag;
            }
        }
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

namespace {
void set_ds_flags_to_format(VkFormat& slot, VkFormat format) {
    if (slot == VK_FORMAT_UNDEFINED) {
        slot = format;
    }
}
} // namespace

void VulkanCaps::initDepthStencilFormatTable(const skgpu::VulkanInterface* interface,
                                             VkPhysicalDevice physDev,
                                             const VkPhysicalDeviceProperties& properties) {
    static_assert(std::size(kDepthStencilVkFormats) == VulkanCaps::kNumDepthStencilVkFormats,
                  "Size of DepthStencilVkFormats array must match static value in header");

    using DSFlags = SkEnumBitMask<DepthStencilFlags>;
    constexpr DSFlags stencilFlags = DepthStencilFlags::kStencil;
    constexpr DSFlags depthFlags = DepthStencilFlags::kDepth;
    constexpr DSFlags dsFlags = DepthStencilFlags::kDepthStencil;

    std::fill_n(fDepthStencilFlagsToFormatTable, kNumDepthStencilFlags, VK_FORMAT_UNDEFINED);
    // Format: VK_FORMAT_S8_UINT
    {
        constexpr VkFormat format = VK_FORMAT_S8_UINT;
        auto& info = this->getDepthStencilFormatInfo(format);
        info.init(interface, physDev, properties, format);
        if (info.fFormatProperties.optimalTilingFeatures &
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            set_ds_flags_to_format(fDepthStencilFlagsToFormatTable[stencilFlags.value()], format);
        }
    }
    // Format: VK_FORMAT_D16_UNORM
    {
        // Qualcomm drivers will report OUT_OF_HOST_MEMORY when binding memory to a VkImage with
        // D16_UNORM in a protected context. Using D32_SFLOAT succeeds, so clearly it's not actually
        // out of memory. D16_UNORM appears to function correctly in unprotected contexts.
        const bool disableD16InProtected = this->protectedSupport() &&
                                           kQualcomm_VkVendor == properties.vendorID;
        if (!disableD16InProtected) {
            constexpr VkFormat format = VK_FORMAT_D16_UNORM;
            auto& info = this->getDepthStencilFormatInfo(format);
            info.init(interface, physDev, properties, format);
            if (info.fFormatProperties.optimalTilingFeatures &
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
                set_ds_flags_to_format(fDepthStencilFlagsToFormatTable[depthFlags.value()], format);
            }
        }
    }
    // Format: VK_FORMAT_D32_SFLOAT
    {
        constexpr VkFormat format = VK_FORMAT_D32_SFLOAT;
        auto& info = this->getDepthStencilFormatInfo(format);
        info.init(interface, physDev, properties, format);
        if (info.fFormatProperties.optimalTilingFeatures &
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            set_ds_flags_to_format(fDepthStencilFlagsToFormatTable[depthFlags.value()], format);
        }
    }
    // Format: VK_FORMAT_D24_UNORM_S8_UINT
    {
        constexpr VkFormat format = VK_FORMAT_D24_UNORM_S8_UINT;
        auto& info = this->getDepthStencilFormatInfo(format);
        info.init(interface, physDev, properties, format);
        if (info.fFormatProperties.optimalTilingFeatures &
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            set_ds_flags_to_format(fDepthStencilFlagsToFormatTable[stencilFlags.value()], format);
            set_ds_flags_to_format(fDepthStencilFlagsToFormatTable[depthFlags.value()], format);
            set_ds_flags_to_format(fDepthStencilFlagsToFormatTable[dsFlags.value()], format);
        }
    }
    // Format: VK_FORMAT_D32_SFLOAT_S8_UINT
    {
        constexpr VkFormat format = VK_FORMAT_D32_SFLOAT_S8_UINT;
        auto& info = this->getDepthStencilFormatInfo(format);
        info.init(interface, physDev, properties, format);
        if (info.fFormatProperties.optimalTilingFeatures &
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            set_ds_flags_to_format(fDepthStencilFlagsToFormatTable[stencilFlags.value()], format);
            set_ds_flags_to_format(fDepthStencilFlagsToFormatTable[depthFlags.value()], format);
            set_ds_flags_to_format(fDepthStencilFlagsToFormatTable[dsFlags.value()], format);
        }
    }
}

void VulkanCaps::SupportedSampleCounts::initSampleCounts(const skgpu::VulkanInterface* interface,
        VkPhysicalDevice physDev,
        const VkPhysicalDeviceProperties& physProps,
        VkFormat format,
        VkImageUsageFlags usage) {
    VkImageFormatProperties properties;

    VkResult result;
    // VULKAN_CALL_RESULT requires a VulkanSharedContext for tracking DEVICE_LOST, but VulkanCaps
    // are initialized before a VulkanSharedContext is available. The _NOCHECK variant only requires
    // a VulkanInterface, so we can use that and log failures manually.
    VULKAN_CALL_RESULT_NOCHECK(interface,
                               result,
                               GetPhysicalDeviceImageFormatProperties(physDev,
                                                                      format,
                                                                      VK_IMAGE_TYPE_2D,
                                                                      VK_IMAGE_TILING_OPTIMAL,
                                                                      usage,
                                                                      0,  // createFlags
                                                                      &properties));
    if (result != VK_SUCCESS) {
        SKGPU_LOG_W("Vulkan call GetPhysicalDeviceImageFormatProperties failed: %d", result);
        return;
    }

    VkSampleCountFlags flags = properties.sampleCounts;
    if (flags & VK_SAMPLE_COUNT_1_BIT) {
        fSampleCounts.push_back(1);
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
        fSampleCounts.push_back(2);
    }
    if (flags & VK_SAMPLE_COUNT_4_BIT) {
        fSampleCounts.push_back(4);
    }
    if (flags & VK_SAMPLE_COUNT_8_BIT) {
        fSampleCounts.push_back(8);
    }
    if (flags & VK_SAMPLE_COUNT_16_BIT) {
        fSampleCounts.push_back(16);
    }
    // Standard sample locations are not defined for more than 16 samples, and we don't need more
    // than 16. Omit 32 and 64.
}

bool VulkanCaps::SupportedSampleCounts::isSampleCountSupported(int requestedCount) const {
    requestedCount = std::max(1, requestedCount);
    for (int i = 0; i < fSampleCounts.size(); i++) {
        if (fSampleCounts[i] == requestedCount) {
            return true;
        } else if (requestedCount < fSampleCounts[i]) {
            return false;
        }
    }
    return false;
}


namespace {
bool is_texturable(VkFormatFeatureFlags flags) {
    return SkToBool(VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT & flags) &&
           SkToBool(VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT & flags);
}

bool is_renderable(VkFormatFeatureFlags flags) {
    return SkToBool(VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT & flags);
}

bool is_storage(VkFormatFeatureFlags flags) {
    return SkToBool(VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT & flags);
}

bool is_transfer_src(VkFormatFeatureFlags flags) {
    return SkToBool(VK_FORMAT_FEATURE_TRANSFER_SRC_BIT & flags);
}

bool is_transfer_dst(VkFormatFeatureFlags flags) {
    return SkToBool(VK_FORMAT_FEATURE_TRANSFER_DST_BIT & flags);
}
}

void VulkanCaps::FormatInfo::init(const skgpu::VulkanInterface* interface,
                                  VkPhysicalDevice physDev,
                                  const VkPhysicalDeviceProperties& properties,
                                  VkFormat format) {
    memset(&fFormatProperties, 0, sizeof(VkFormatProperties));
    VULKAN_CALL(interface, GetPhysicalDeviceFormatProperties(physDev, format, &fFormatProperties));

    if (is_renderable(fFormatProperties.optimalTilingFeatures)) {
        // We make all renderable images support being used as input attachment
        VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                       VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                       VK_IMAGE_USAGE_SAMPLED_BIT |
                                       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                       VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        this->fSupportedSampleCounts.initSampleCounts(interface, physDev, properties, format,
                                                      usageFlags);
    }
}

bool VulkanCaps::FormatInfo::isTexturable(VkImageTiling imageTiling) const {
    switch (imageTiling) {
        case VK_IMAGE_TILING_OPTIMAL:
            return is_texturable(fFormatProperties.optimalTilingFeatures);
        case VK_IMAGE_TILING_LINEAR:
            return is_texturable(fFormatProperties.linearTilingFeatures);
        default:
            return false;
    }
    SkUNREACHABLE;
}

bool VulkanCaps::FormatInfo::isRenderable(VkImageTiling imageTiling,
                                          uint32_t sampleCount) const {
    if (!fSupportedSampleCounts.isSampleCountSupported(sampleCount)) {
        return false;
    }
    switch (imageTiling) {
        case VK_IMAGE_TILING_OPTIMAL:
            return is_renderable(fFormatProperties.optimalTilingFeatures);
        case VK_IMAGE_TILING_LINEAR:
            return is_renderable(fFormatProperties.linearTilingFeatures);
        default:
            return false;
    }
    SkUNREACHABLE;
}

bool VulkanCaps::FormatInfo::isStorage(VkImageTiling imageTiling) const {
    switch (imageTiling) {
        case VK_IMAGE_TILING_OPTIMAL:
            return is_storage(fFormatProperties.optimalTilingFeatures);
        case VK_IMAGE_TILING_LINEAR:
            return is_storage(fFormatProperties.linearTilingFeatures);
        default:
            return false;
    }
    SkUNREACHABLE;
}

bool VulkanCaps::FormatInfo::isTransferSrc(VkImageTiling imageTiling) const {
    switch (imageTiling) {
        case VK_IMAGE_TILING_OPTIMAL:
            return is_transfer_src(fFormatProperties.optimalTilingFeatures);
        case VK_IMAGE_TILING_LINEAR:
            return is_transfer_src(fFormatProperties.linearTilingFeatures);
        default:
            return false;
    }
    SkUNREACHABLE;
}

bool VulkanCaps::FormatInfo::isTransferDst(VkImageTiling imageTiling) const {
    switch (imageTiling) {
        case VK_IMAGE_TILING_OPTIMAL:
            return is_transfer_dst(fFormatProperties.optimalTilingFeatures);
        case VK_IMAGE_TILING_LINEAR:
            return is_transfer_dst(fFormatProperties.linearTilingFeatures);
        default:
            return false;
    }
    SkUNREACHABLE;
}

void VulkanCaps::setColorType(SkColorType colorType, std::initializer_list<VkFormat> formats) {
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

VkFormat VulkanCaps::getFormatFromColorType(SkColorType colorType) const {
    int idx = static_cast<int>(colorType);
    return fColorTypeToFormatTable[idx];
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

const VulkanCaps::FormatInfo& VulkanCaps::getFormatInfo(VkFormat format) const {
    VulkanCaps* nonConstThis = const_cast<VulkanCaps*>(this);
    return nonConstThis->getFormatInfo(format);
}

void VulkanCaps::DepthStencilFormatInfo::init(const skgpu::VulkanInterface* interface,
                                             VkPhysicalDevice physDev,
                                             const VkPhysicalDeviceProperties& properties,
                                             VkFormat format) {
    memset(&fFormatProperties, 0, sizeof(VkFormatProperties));
    VULKAN_CALL(interface, GetPhysicalDeviceFormatProperties(physDev, format, &fFormatProperties));

    if (this->isDepthStencilSupported(fFormatProperties.optimalTilingFeatures)) {
        VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        fSupportedSampleCounts.initSampleCounts(interface, physDev, properties, format, usageFlags);
    }
}

bool VulkanCaps::DepthStencilFormatInfo::isDepthStencilSupported(VkFormatFeatureFlags flags) const {
    return SkToBool(VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT & flags);
}

VkFormat VulkanCaps::getFormatFromDepthStencilFlags(const SkEnumBitMask<DepthStencilFlags>& flags)
        const {
    return fDepthStencilFlagsToFormatTable[flags.value()];
}

VulkanCaps::DepthStencilFormatInfo& VulkanCaps::getDepthStencilFormatInfo(VkFormat format) {
    static_assert(std::size(kDepthStencilVkFormats) == VulkanCaps::kNumDepthStencilVkFormats,
                  "Size of VkFormats array must match static value in header");
    for (size_t i = 0; i < std::size(kDepthStencilVkFormats); ++i) {
        if (kVkFormats[i] == format) {
            return fDepthStencilFormatTable[i];
        }
    }
    static DepthStencilFormatInfo kInvalidFormat;
    return kInvalidFormat;
}

const VulkanCaps::DepthStencilFormatInfo& VulkanCaps::getDepthStencilFormatInfo(VkFormat format)
        const {
    VulkanCaps* nonConstThis = const_cast<VulkanCaps*>(this);
    return nonConstThis->getDepthStencilFormatInfo(format);
}

const Caps::ColorTypeInfo* VulkanCaps::getColorTypeInfo(SkColorType ct,
                                                        const TextureInfo& textureInfo) const {
    VkFormat vkFormat = textureInfo.vulkanTextureSpec().fFormat;
    if (vkFormat == VK_FORMAT_UNDEFINED) {
        return nullptr;
    }

    const FormatInfo& info = this->getFormatInfo(vkFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        const ColorTypeInfo& ctInfo = info.fColorTypeInfos[i];
        if (ctInfo.fColorType == ct) {
            return &ctInfo;
        }
    }

    return nullptr;
}

bool VulkanCaps::onIsTexturable(const TextureInfo& texInfo) const {
    VulkanTextureInfo vkInfo;
    if (!texInfo.getVulkanTextureInfo(&vkInfo)) {
        return false;
    }

    // TODO:
    // Once we support external formats with associated YCbCr conversion info, check for that
    // and return true here because we can always texture from an external format.

    const FormatInfo& info = this->getFormatInfo(vkInfo.fFormat);
    return info.isTexturable(vkInfo.fImageTiling);
}


bool VulkanCaps::isRenderable(const TextureInfo& texInfo) const {
    VulkanTextureInfo vkInfo;
    if (!texInfo.getVulkanTextureInfo(&vkInfo)) {
        return false;
    }

    const FormatInfo& info = this->getFormatInfo(vkInfo.fFormat);
    return info.isRenderable(vkInfo.fImageTiling, texInfo.numSamples());
}

bool VulkanCaps::isStorage(const TextureInfo& texInfo) const {
    VulkanTextureInfo vkInfo;
    if (!texInfo.getVulkanTextureInfo(&vkInfo)) {
        return false;
    }

    const FormatInfo& info = this->getFormatInfo(vkInfo.fFormat);
    return info.isStorage(vkInfo.fImageTiling);
}

bool VulkanCaps::isTransferSrc(const VulkanTextureInfo& vkInfo) const {
    const FormatInfo& info = this->getFormatInfo(vkInfo.fFormat);
    return info.isTransferSrc(vkInfo.fImageTiling);
}

bool VulkanCaps::isTransferDst(const VulkanTextureInfo& vkInfo) const {
    const FormatInfo& info = this->getFormatInfo(vkInfo.fFormat);
    return info.isTransferDst(vkInfo.fImageTiling);
}

bool VulkanCaps::supportsWritePixels(const TextureInfo& texInfo) const {
    VulkanTextureInfo vkInfo;
    if (!texInfo.getVulkanTextureInfo(&vkInfo)) {
        return false;
    }

    // Can't write if it needs a YCbCr sampler
    if (VkFormatNeedsYcbcrSampler(vkInfo.fFormat)) {
        return false;
    }

    if (vkInfo.fSampleCount > 1) {
        return false;
    }

    if (!SkToBool(vkInfo.fImageUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
        return false;
    }

    return true;
}

bool VulkanCaps::supportsReadPixels(const TextureInfo& texInfo) const {
    if (texInfo.isProtected() == Protected::kYes) {
        return false;
    }

    VulkanTextureInfo vkInfo;
    if (!texInfo.getVulkanTextureInfo(&vkInfo)) {
        return false;
    }

    // Can't read if it needs a YCbCr sampler
    if (VkFormatNeedsYcbcrSampler(vkInfo.fFormat)) {
        return false;
    }

    if (VkFormatIsCompressed(vkInfo.fFormat)) {
        return false;
    }

    if (vkInfo.fSampleCount > 1) {
        return false;
    }

    if (!SkToBool(vkInfo.fImageUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)) {
        return false;
    }

    return true;
}

std::pair<SkColorType, bool /*isRGBFormat*/> VulkanCaps::supportedWritePixelsColorType(
        SkColorType dstColorType,
        const TextureInfo& dstTextureInfo,
        SkColorType srcColorType) const {
    VulkanTextureInfo vkInfo;
    if (!dstTextureInfo.getVulkanTextureInfo(&vkInfo)) {
        return {kUnknown_SkColorType, false};
    }

    // Can't write to YCbCr formats
    // TODO: Can't write to external formats, either
    if (VkFormatNeedsYcbcrSampler(vkInfo.fFormat)) {
        return {kUnknown_SkColorType, false};
    }

    const FormatInfo& info = this->getFormatInfo(vkInfo.fFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        const auto& ctInfo = info.fColorTypeInfos[i];
        if (ctInfo.fColorType == dstColorType) {
            return {ctInfo.fTransferColorType, vkInfo.fFormat == VK_FORMAT_R8G8B8_UNORM};
        }
    }

    return {kUnknown_SkColorType, false};
}

std::pair<SkColorType, bool /*isRGBFormat*/> VulkanCaps::supportedReadPixelsColorType(
        SkColorType srcColorType,
        const TextureInfo& srcTextureInfo,
        SkColorType dstColorType) const {
    VulkanTextureInfo vkInfo;
    if (!srcTextureInfo.getVulkanTextureInfo(&vkInfo)) {
        return {kUnknown_SkColorType, false};
    }

    // Can't read from YCbCr formats
    // TODO: external formats?
    if (VkFormatNeedsYcbcrSampler(vkInfo.fFormat)) {
        return {kUnknown_SkColorType, false};
    }

    // TODO: handle compressed formats
    if (VkFormatIsCompressed(vkInfo.fFormat)) {
        SkASSERT(this->isTexturable(vkInfo));
        return {kUnknown_SkColorType, false};
    }

    const FormatInfo& info = this->getFormatInfo(vkInfo.fFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        const auto& ctInfo = info.fColorTypeInfos[i];
        if (ctInfo.fColorType == srcColorType) {
            return {ctInfo.fTransferColorType, vkInfo.fFormat == VK_FORMAT_R8G8B8_UNORM};
        }
    }

    return {kUnknown_SkColorType, false};
}

UniqueKey VulkanCaps::makeGraphicsPipelineKey(const GraphicsPipelineDesc& pipelineDesc,
                                              const RenderPassDesc& renderPassDesc) const {
    UniqueKey pipelineKey;
    {
        static const skgpu::UniqueKey::Domain kGraphicsPipelineDomain =
            UniqueKey::GenerateDomain();

        VulkanRenderPass::VulkanRenderPassMetaData rpMetaData {renderPassDesc};

        // Reserve 3 uint32s for the render step id, paint id, and write swizzle.
        static constexpr int kUint32sNeededForPipelineInfo = 3;
        // The uint32s needed for a RenderPass is variable number, so consult rpMetaData to
        // determine how many to reserve.
        UniqueKey::Builder builder(&pipelineKey,
                                   kGraphicsPipelineDomain,
                                   kUint32sNeededForPipelineInfo + rpMetaData.fUint32DataCnt,
                                   "GraphicsPipeline");

        int idx = 0;
        // Add GraphicsPipelineDesc information
        builder[idx++] = pipelineDesc.renderStepID();
        builder[idx++] = pipelineDesc.paintParamsID().asUInt();
        // Add RenderPass info relevant for pipeline creation that's not captured in RenderPass keys
        builder[idx++] = renderPassDesc.fWriteSwizzle.asKey();
        // Add RenderPassDesc information
        VulkanRenderPass::AddRenderPassInfoToKey(rpMetaData, builder, idx, /*compatibleOnly=*/true);

        builder.finish();
    }

    return pipelineKey;
}

GraphiteResourceKey VulkanCaps::makeSamplerKey(const SamplerDesc& samplerDesc) const {
    GraphiteResourceKey samplerKey;
    // Non-zero conversion information means the sampler utilizes a ycbcr conversion.
    uint32_t nonFormatYcbcrInfo =
            (uint32_t)(samplerDesc.desc() >> SamplerDesc::kImmutableSamplerInfoShift);
    bool usesYcbcrConversion = nonFormatYcbcrInfo != 0;

    int uint32Quantity = 1;
    bool usesExternalFormat = false;
    if (usesYcbcrConversion) {
        // If using a YCbCr conversion, check nonFormatYcbcrInfo to see if we are using an external
        // format and update uint32Quantity accordingly.
        usesExternalFormat = static_cast<bool>(
                ((nonFormatYcbcrInfo & ycbcrPackaging::kUseExternalFormatMask) >>
                        ycbcrPackaging::kUsesExternalFormatShift));
        // Reassign uint32Quantity. This is an assignment instead of an additive operation because
        // non-format ycbcr information and sampler information cam be fit into just one int32.
        uint32Quantity = usesExternalFormat ? ycbcrPackaging::kInt32sNeededExternalFormat
                                            : ycbcrPackaging::kInt32sNeededKnownFormat;
    }
    static const ResourceType kSamplerType = GraphiteResourceKey::GenerateResourceType();
    GraphiteResourceKey::Builder builder(&samplerKey, kSamplerType, uint32Quantity,
                                         Shareable::kYes);
    int i = 0;
    builder[i++] = samplerDesc.desc();
    if (usesYcbcrConversion) {
        if (usesExternalFormat) {
            builder[i++] = samplerDesc.externalFormatMSBs();
        }
        builder[i++] = samplerDesc.format();
    }
    SkASSERT(i == uint32Quantity);

    builder.finish();
    return samplerKey;
}

void VulkanCaps::buildKeyForTexture(SkISize dimensions,
                                    const TextureInfo& info,
                                    ResourceType type,
                                    Shareable shareable,
                                    GraphiteResourceKey* key) const {
    SkASSERT(!dimensions.isEmpty());

    const VulkanTextureSpec& vkSpec = info.vulkanTextureSpec();
    // We expect that the VkFormat enum is at most a 32-bit value.
    static_assert(VK_FORMAT_MAX_ENUM == 0x7FFFFFFF);
    // We should either be using a known VkFormat or have a valid ycbcr conversion.
    SkASSERT(vkSpec.fFormat != VK_FORMAT_UNDEFINED || vkSpec.fYcbcrConversionInfo.isValid());

    uint32_t format = static_cast<uint32_t>(vkSpec.fFormat);
    uint32_t samples = SamplesToKey(info.numSamples());
    // We don't have to key the number of mip levels because it is inherit in the combination of
    // isMipped and dimensions.
    bool isMipped = info.mipmapped() == Mipmapped::kYes;
    Protected isProtected = info.isProtected();

    // Confirm all the below parts of the key can fit in a single uint32_t. The sum of the shift
    // amounts in the asserts must be less than or equal to 32. vkSpec.fFlags will go into its
    // own 32-bit block.
    SkASSERT(samples                            < (1u << 3));  // sample key is first 3 bits
    SkASSERT(static_cast<uint32_t>(isMipped)    < (1u << 1));  // isMapped is 4th bit
    SkASSERT(static_cast<uint32_t>(isProtected) < (1u << 1));  // isProtected is 5th bit
    SkASSERT(vkSpec.fImageTiling                < (1u << 1));  // imageTiling is 6th bit
    SkASSERT(vkSpec.fSharingMode                < (1u << 1));  // sharingMode is 7th bit
    SkASSERT(vkSpec.fAspectMask                 < (1u << 11)); // aspectMask is bits 8 - 19
    SkASSERT(vkSpec.fImageUsageFlags            < (1u << 12)); // imageUsageFlags are bits 20-32

    // We need two uint32_ts for dimensions, 1 for format, and 2 for the rest of the information.
    static constexpr int kNum32DataCntNoYcbcr =  2 + 1 + 2;
    int num32DataCnt = kNum32DataCntNoYcbcr;

    // If a texture w/ an external format is being used, that information must also be appended.
    const VulkanYcbcrConversionInfo& ycbcrInfo = info.vulkanTextureSpec().fYcbcrConversionInfo;
    num32DataCnt += ycbcrPackaging::numInt32sNeeded(ycbcrInfo);

    GraphiteResourceKey::Builder builder(key, type, num32DataCnt, shareable);

    int i = 0;
    builder[i++] = dimensions.width();
    builder[i++] = dimensions.height();

    if (ycbcrInfo.isValid()) {
        SkASSERT(ycbcrInfo.fFormat != VK_FORMAT_UNDEFINED || ycbcrInfo.fExternalFormat != 0);
        bool useExternalFormat = ycbcrInfo.fFormat == VK_FORMAT_UNDEFINED;
        builder[i++] = ycbcrPackaging::nonFormatInfoAsUInt32(ycbcrInfo);
        if (useExternalFormat) {
            builder[i++] = (uint32_t)ycbcrInfo.fExternalFormat;
            builder[i++] = (uint32_t)(ycbcrInfo.fExternalFormat >> 32);
        } else {
            builder[i++] =  ycbcrInfo.fFormat;
        }
    } else {
        builder[i++] = format;
    }

    builder[i++] = (static_cast<uint32_t>(vkSpec.fFlags));
    builder[i++] = (samples                                            << 0 ) |
                   (static_cast<uint32_t>(isMipped)                    << 3 ) |
                   (static_cast<uint32_t>(isProtected)                 << 4 ) |
                   (static_cast<uint32_t>(vkSpec.fImageTiling)         << 5 ) |
                   (static_cast<uint32_t>(vkSpec.fSharingMode)         << 6 ) |
                   (static_cast<uint32_t>(vkSpec.fAspectMask)          << 7 ) |
                   (static_cast<uint32_t>(vkSpec.fImageUsageFlags)     << 19);
    SkASSERT(i == num32DataCnt);
}

ImmutableSamplerInfo VulkanCaps::getImmutableSamplerInfo(sk_sp<TextureProxy> proxy) const {
    if (proxy) {
        const skgpu::VulkanYcbcrConversionInfo& ycbcrConversionInfo =
                proxy->textureInfo().vulkanTextureSpec().fYcbcrConversionInfo;

        if (ycbcrConversionInfo.isValid()) {
            ImmutableSamplerInfo immutableSamplerInfo;
            // ycbcrConversionInfo's fFormat being VK_FORMAT_UNDEFINED indicates we are using an
            // external format rather than a known VkFormat.
            immutableSamplerInfo.fFormat = ycbcrConversionInfo.fFormat == VK_FORMAT_UNDEFINED
                    ? ycbcrConversionInfo.fExternalFormat
                    : ycbcrConversionInfo.fFormat;
            immutableSamplerInfo.fNonFormatYcbcrConversionInfo =
                    ycbcrPackaging::nonFormatInfoAsUInt32(ycbcrConversionInfo);
            return immutableSamplerInfo;
        }
    }

    // If the proxy is null or the YCbCr conversion for that proxy is invalid, then return a
    // default ImmutableSamplerInfo struct.
    return {};
}

} // namespace skgpu::graphite
