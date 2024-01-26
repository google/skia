/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/vk/VulkanUtilsPriv.h"

namespace skgpu {
/**
 * Returns a populated VkSamplerYcbcrConversionCreateInfo object based on VulkanYcbcrConversionInfo
*/
void SetupSamplerYcbcrConversionInfo(VkSamplerYcbcrConversionCreateInfo* outInfo,
                                     const VulkanYcbcrConversionInfo& conversionInfo) {
#ifdef SK_DEBUG
    const VkFormatFeatureFlags& featureFlags = conversionInfo.fFormatFeatures;
    if (conversionInfo.fXChromaOffset == VK_CHROMA_LOCATION_MIDPOINT ||
        conversionInfo.fYChromaOffset == VK_CHROMA_LOCATION_MIDPOINT) {
        SkASSERT(featureFlags & VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT);
    }
    if (conversionInfo.fXChromaOffset == VK_CHROMA_LOCATION_COSITED_EVEN ||
        conversionInfo.fYChromaOffset == VK_CHROMA_LOCATION_COSITED_EVEN) {
        SkASSERT(featureFlags & VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT);
    }
    if (conversionInfo.fChromaFilter == VK_FILTER_LINEAR) {
        SkASSERT(featureFlags & VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT);
    }
    if (conversionInfo.fForceExplicitReconstruction) {
        SkASSERT(featureFlags &
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT);
    }
#endif

    outInfo->sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO;
    outInfo->pNext = nullptr;
    outInfo->format = conversionInfo.fFormat;
    outInfo->ycbcrModel = conversionInfo.fYcbcrModel;
    outInfo->ycbcrRange = conversionInfo.fYcbcrRange;

    // Components is ignored for external format conversions. For all other formats identity swizzle
    // is used. It can be added to VulkanYcbcrConversionInfo if necessary.
    outInfo->components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                           VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
    outInfo->xChromaOffset = conversionInfo.fXChromaOffset;
    outInfo->yChromaOffset = conversionInfo.fYChromaOffset;
    outInfo->chromaFilter = conversionInfo.fChromaFilter;
    outInfo->forceExplicitReconstruction = conversionInfo.fForceExplicitReconstruction;
}

#ifdef SK_BUILD_FOR_ANDROID

/**
 * Define a macro that both ganesh and graphite can use to make simple calls into Vulkan so we can
 * share more code between them.
*/
#define SHARED_GR_VULKAN_CALL(IFACE, X) (IFACE)->fFunctions.f##X

/**
 * Shared Vulkan AHardwareBuffer utility functions between graphite and ganesh
*/
void GetYcbcrConversionInfoFromFormatProps(
        VulkanYcbcrConversionInfo* outConversionInfo,
        const VkAndroidHardwareBufferFormatPropertiesANDROID& formatProps) {
    outConversionInfo->fYcbcrModel = formatProps.suggestedYcbcrModel;
    outConversionInfo->fYcbcrRange = formatProps.suggestedYcbcrRange;
    outConversionInfo->fXChromaOffset = formatProps.suggestedXChromaOffset;
    outConversionInfo->fYChromaOffset = formatProps.suggestedYChromaOffset;
    outConversionInfo->fForceExplicitReconstruction = VK_FALSE;
    outConversionInfo->fExternalFormat = formatProps.externalFormat;
    outConversionInfo->fFormatFeatures = formatProps.formatFeatures;
    if (VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT &
        formatProps.formatFeatures) {
        outConversionInfo->fChromaFilter = VK_FILTER_LINEAR;
    } else {
        outConversionInfo->fChromaFilter = VK_FILTER_NEAREST;
    }
}

bool GetAHardwareBufferProperties(
        VkAndroidHardwareBufferFormatPropertiesANDROID* outHwbFormatProps,
        VkAndroidHardwareBufferPropertiesANDROID* outHwbProps,
        const skgpu::VulkanInterface* interface,
        const AHardwareBuffer* hwBuffer,
        VkDevice device) {
    outHwbFormatProps->sType =
            VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID;
    outHwbFormatProps->pNext = nullptr;

    outHwbProps->sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID;
    outHwbProps->pNext = outHwbFormatProps;

    VkResult result =
            SHARED_GR_VULKAN_CALL(interface,
                                  GetAndroidHardwareBufferProperties(device,
                                                                     hwBuffer,
                                                                     outHwbProps));
    if (result != VK_SUCCESS) {
        SkDebugf("Failed to get AndroidHardwareBufferProperties\n");
        return false;
    }
    return true;
}

bool AllocateAndBindImageMemory(skgpu::VulkanAlloc* outVulkanAlloc,
                                VkImage image,
                                const VkPhysicalDeviceMemoryProperties2& phyDevMemProps,
                                const VkAndroidHardwareBufferPropertiesANDROID& hwbProps,
                                AHardwareBuffer* hardwareBuffer,
                                const skgpu::VulkanInterface* interface,
                                VkDevice device) {
    VkResult result;
    uint32_t typeIndex = 0;
    bool foundHeap = false;
    uint32_t memTypeCnt = phyDevMemProps.memoryProperties.memoryTypeCount;
    for (uint32_t i = 0; i < memTypeCnt && !foundHeap; ++i) {
        if (hwbProps.memoryTypeBits & (1 << i)) {
            const VkPhysicalDeviceMemoryProperties& pdmp = phyDevMemProps.memoryProperties;
            uint32_t supportedFlags = pdmp.memoryTypes[i].propertyFlags &
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            if (supportedFlags == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
                typeIndex = i;
                foundHeap = true;
            }
        }
    }

    /**
     * Fallback to use any available memory type for AHB.
     *
     * For external memory import, compatible memory types are decided by the Vulkan driver since
     * the memory has been allocated externally. There are usually special requirements against
     * external memory. e.g. AHB allocated with CPU R/W often usage bits is only importable for
     * non-device-local heap on some AMD systems.
    */
    if (!foundHeap && hwbProps.memoryTypeBits) {
        typeIndex = ffs(hwbProps.memoryTypeBits) - 1;
        foundHeap = true;
    }
    if (!foundHeap) {
        return false;
    }

    VkImportAndroidHardwareBufferInfoANDROID hwbImportInfo;
    hwbImportInfo.sType = VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID;
    hwbImportInfo.pNext = nullptr;
    hwbImportInfo.buffer = hardwareBuffer;

    VkMemoryDedicatedAllocateInfo dedicatedAllocInfo;
    dedicatedAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO;
    dedicatedAllocInfo.pNext = &hwbImportInfo;
    dedicatedAllocInfo.image = image;
    dedicatedAllocInfo.buffer = VK_NULL_HANDLE;

    VkMemoryAllocateInfo allocInfo = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,      // sType
        &dedicatedAllocInfo,                         // pNext
        hwbProps.allocationSize,                     // allocationSize
        typeIndex,                                   // memoryTypeIndex
    };

    VkDeviceMemory memory;
    result = SHARED_GR_VULKAN_CALL(interface,
                                   AllocateMemory(device, &allocInfo, nullptr, &memory));
    if (result != VK_SUCCESS) {
        return false;
    }

    VkBindImageMemoryInfo bindImageInfo;
    bindImageInfo.sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO;
    bindImageInfo.pNext = nullptr;
    bindImageInfo.image = image;
    bindImageInfo.memory = memory;
    bindImageInfo.memoryOffset = 0;

    result = SHARED_GR_VULKAN_CALL(interface, BindImageMemory2(device, 1, &bindImageInfo));
    if (result != VK_SUCCESS) {
        SHARED_GR_VULKAN_CALL(interface, FreeMemory(device, memory, nullptr));
        return false;
    }

    outVulkanAlloc->fMemory = memory;
    outVulkanAlloc->fOffset = 0;
    outVulkanAlloc->fSize = hwbProps.allocationSize;
    outVulkanAlloc->fFlags = 0;
    outVulkanAlloc->fBackendMemory = 0;
    return true;
}

#endif // SK_BUILD_FOR_ANDROID

} // namespace skgpu
