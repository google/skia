/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/vk/VulkanUtilsPriv.h"

#include "include/gpu/vk/VulkanBackendContext.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTFitsIn.h"
#include "include/private/base/SkTo.h"
#include "src/gpu/vk/VulkanInterface.h"

#include <algorithm>
#include <vector>

namespace skgpu {

/**
 * Define a macro that both ganesh and graphite can use to make simple calls into Vulkan so we can
 * share more code between them.
*/
#define SHARED_GR_VULKAN_CALL(IFACE, X) (IFACE)->fFunctions.f##X

/**
 * Parse the driver version number in VkPhysicalDeviceProperties::driverVersion according to the
 * driver ID.
 */
DriverVersion ParseVulkanDriverVersion(VkDriverId driverId, uint32_t driverVersion) {
    // Most drivers follow the VK_MAKE_API_VERSION convention.  The exceptions are documented in the
    // switch cases below.
    switch (driverId) {
        case VK_DRIVER_ID_INTEL_PROPRIETARY_WINDOWS:
            // Windows Intel driver versions are built in the following format:
            //
            //     Major (18 bits) | Minor (14 bits)
            //
            return DriverVersion(driverVersion >> 14, driverVersion & 0x3FFF);
        case VK_DRIVER_ID_NVIDIA_PROPRIETARY:
            // Nvidia proprietary driver version is in the following format:
            //
            //     Major (10 bits) | Minor (8 bits) | SubMinor (8 bits) | Patch (6 bits)
            //
            return DriverVersion(driverVersion >> 22, driverVersion >> 14 & 0xFF);
        case VK_DRIVER_ID_QUALCOMM_PROPRIETARY:
            // Qualcomm proprietary driver version has changed over time.  In the new format, it's
            // almost following the VK_MAKE_API_VERSION convention, except the top bit is set.  With
            // VK_API_VERSION_MAJOR, this bit is masked out (corresponding to a value of 512), which
            // is typically expected to be visible in the version, i.e. the version is 512.NNNN. The
            // old format is unknown, and is considered 0.NNNN.
            if ((driverVersion & 0x80000000) != 0) {
                return DriverVersion(VK_API_VERSION_MAJOR(driverVersion) | 512,
                                     VK_API_VERSION_MINOR(driverVersion));
            }

            return DriverVersion(0, driverVersion);
        case VK_DRIVER_ID_MOLTENVK:
            // MoltenVK driver version is in the following format:
            //
            //     Major * 10000 + Minor * 100 + patch
            //
            return DriverVersion(driverVersion / 10000, (driverVersion / 100) % 100);
        default:
            return DriverVersion(VK_API_VERSION_MAJOR(driverVersion),
                                 VK_API_VERSION_MINOR(driverVersion));
    }
}

#ifdef SK_BUILD_FOR_ANDROID

/**
 * Shared Vulkan AHardwareBuffer utility functions between graphite and ganesh
*/
void GetYcbcrConversionInfoFromFormatProps(
        VulkanYcbcrConversionInfo* outConversionInfo,
        const VkAndroidHardwareBufferFormatPropertiesANDROID& formatProps) {
    VkFilter chromaFilter;
    if (VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT &
        formatProps.formatFeatures) {
        chromaFilter = VK_FILTER_LINEAR;
    } else {
        chromaFilter = VK_FILTER_NEAREST;
    }

    *outConversionInfo = VulkanYcbcrConversionInfo(formatProps.externalFormat,
                                                   formatProps.suggestedYcbcrModel,
                                                   formatProps.suggestedYcbcrRange,
                                                   formatProps.suggestedXChromaOffset,
                                                   formatProps.suggestedYChromaOffset,
                                                   chromaFilter,
                                                   /*forceExplicitReconstruction=*/VK_FALSE,
                                                   formatProps.samplerYcbcrConversionComponents,
                                                   formatProps.formatFeatures);
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
        // The spec suggests VK_ERROR_OUT_OF_HOST_MEMORY and VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR
        // are the only failure codes, but some platforms may report others, such as
        // VK_ERROR_FORMAT_NOT_SUPPORTED (-11).
        SkDebugf("Failed to get AndroidHardwareBufferProperties (result:%d)", result);
#if __ANDROID_API__ >= 26
        AHardwareBuffer_Desc hwbDesc;
        AHardwareBuffer_describe(hwBuffer, &hwbDesc);
        SkDebugf("^ %" PRIu32 "x%" PRIu32 " AHB -- format:%" PRIu32 ", usage:%" PRIu64
                 ", layers:%" PRIu32,
                 hwbDesc.width,
                 hwbDesc.height,
                 hwbDesc.format,
                 hwbDesc.usage,
                 hwbDesc.layers);
#endif
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

// Note: since this is called from Vulkan result-checking functions, any Vk calls this function
// makes must NOT be checked with those same functions to avoid infinite recursion.
void InvokeDeviceLostCallback(const skgpu::VulkanInterface* vulkanInterface,
                              VkDevice vkDevice,
                              skgpu::VulkanDeviceLostContext deviceLostContext,
                              skgpu::VulkanDeviceLostProc deviceLostProc,
                              bool supportsDeviceFaultInfoExtension) {
    if (!deviceLostProc) {
        return;
    }

    std::vector<VkDeviceFaultAddressInfoEXT> addressInfos = {};
    std::vector<VkDeviceFaultVendorInfoEXT> vendorInfos = {};
    std::vector<std::byte> vendorBinaryData = {};

    if (!supportsDeviceFaultInfoExtension) {
        deviceLostProc(deviceLostContext,
                       "No details: VK_EXT_device_fault not available/enabled.",
                       addressInfos,
                       vendorInfos,
                       vendorBinaryData);
        return;
    }

    // Query counts
    VkDeviceFaultCountsEXT faultCounts = {};
    faultCounts.sType = VK_STRUCTURE_TYPE_DEVICE_FAULT_COUNTS_EXT;
    VkResult result = SHARED_GR_VULKAN_CALL(vulkanInterface,
                                            GetDeviceFaultInfo(vkDevice, &faultCounts, NULL));
    if (result != VK_SUCCESS) {
        deviceLostProc(
                deviceLostContext,
                "No details: VK_EXT_device_fault error counting failed: " + std::to_string(result),
                addressInfos,
                vendorInfos,
                vendorBinaryData);
        return;
    }

    // Prepare storage
    addressInfos.resize(faultCounts.addressInfoCount);
    vendorInfos.resize(faultCounts.vendorInfoCount);
    vendorBinaryData.resize(faultCounts.vendorBinarySize);

    // Query fault info
    VkDeviceFaultInfoEXT faultInfo = {};
    faultInfo.sType             = VK_STRUCTURE_TYPE_DEVICE_FAULT_INFO_EXT;
    faultInfo.pAddressInfos     = addressInfos.data();
    faultInfo.pVendorInfos      = vendorInfos.data();
    faultInfo.pVendorBinaryData =
            faultCounts.vendorBinarySize > 0 ? vendorBinaryData.data() : nullptr;
    result = SHARED_GR_VULKAN_CALL(vulkanInterface,
                                   GetDeviceFaultInfo(vkDevice, &faultCounts, &faultInfo));
    if (result != VK_SUCCESS) {
        deviceLostProc(
                deviceLostContext,
                "No details: VK_EXT_device_fault info dumping failed: " + std::to_string(result),
                addressInfos,
                vendorInfos,
                vendorBinaryData);
        return;
    }

    deviceLostProc(deviceLostContext,
                   std::string(faultInfo.description),
                   addressInfos,
                   vendorInfos,
                   vendorBinaryData);
}

sk_sp<skgpu::VulkanInterface> MakeInterface(const skgpu::VulkanBackendContext& context,
                                            const skgpu::VulkanExtensions* extOverride,
                                            uint32_t* instanceVersionOut,
                                            uint32_t* physDevVersionOut) {
    if (!extOverride) {
        extOverride = context.fVkExtensions;
    }
    SkASSERT(extOverride);
    PFN_vkEnumerateInstanceVersion localEnumerateInstanceVersion =
            reinterpret_cast<PFN_vkEnumerateInstanceVersion>(
                    context.fGetProc("vkEnumerateInstanceVersion", VK_NULL_HANDLE, VK_NULL_HANDLE));
    uint32_t instanceVersion = 0;
    // Vulkan 1.1 is required, so vkEnumerateInstanceVersion should always be available.
    SkASSERT(localEnumerateInstanceVersion != nullptr);
    VkResult err = localEnumerateInstanceVersion(&instanceVersion);
    if (err) {
        return nullptr;
    }

    PFN_vkGetPhysicalDeviceProperties localGetPhysicalDeviceProperties =
            reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(context.fGetProc(
                    "vkGetPhysicalDeviceProperties", context.fInstance, VK_NULL_HANDLE));
    SkASSERT(localGetPhysicalDeviceProperties != nullptr);

    VkPhysicalDeviceProperties physDeviceProperties;
    localGetPhysicalDeviceProperties(context.fPhysicalDevice, &physDeviceProperties);
    uint32_t physDevVersion = physDeviceProperties.apiVersion;

    uint32_t apiVersion = context.fMaxAPIVersion ? context.fMaxAPIVersion : instanceVersion;

    instanceVersion = std::min(instanceVersion, apiVersion);
    physDevVersion = std::min(physDevVersion, apiVersion);

    if (instanceVersion < VK_API_VERSION_1_1 || physDevVersion < VK_API_VERSION_1_1) {
        SK_ABORT("Vulkan 1.1 is required but not available. "
                 "Instance version: %#08X, Device version: %#08X",
                 instanceVersion, physDevVersion);
    }

    sk_sp<skgpu::VulkanInterface> interface(new skgpu::VulkanInterface(context.fGetProc,
                                                                       context.fInstance,
                                                                       context.fDevice,
                                                                       instanceVersion,
                                                                       physDevVersion,
                                                                       extOverride));
    if (!interface->validate(instanceVersion, physDevVersion, extOverride)) {
        return nullptr;
    }
    if (physDevVersionOut) {
        *physDevVersionOut = physDevVersion;
    }
    if (instanceVersionOut) {
        *instanceVersionOut = instanceVersion;
    }
    return interface;
}

VulkanYcbcrConversionInfo::VulkanYcbcrConversionInfo(VkFormat format,
                                                     uint64_t externalFormat,
                                                     VkSamplerYcbcrModelConversion ycbcrModel,
                                                     VkSamplerYcbcrRange ycbcrRange,
                                                     VkChromaLocation xChromaOffset,
                                                     VkChromaLocation yChromaOffset,
                                                     VkFilter chromaFilter,
                                                     VkBool32 forceExplicitReconstruction,
                                                     VkComponentMapping components,
                                                     VkFormatFeatureFlags formatFeatures)
        : fFormat(format)
        , fExternalFormat(externalFormat)
        , fYcbcrModel(ycbcrModel)
        , fYcbcrRange(ycbcrRange)
        , fXChromaOffset(xChromaOffset)
        , fYChromaOffset(yChromaOffset)
        , fChromaFilter(chromaFilter)
        , fForceExplicitReconstruction(forceExplicitReconstruction)
        , fComponents(components) {
#ifdef SK_DEBUG
    // Format feature flags are only representative of an external format's capabilities, so
    // skip these checks in the case of using a known format or if the featureFlags were clearly
    // not filled in.
    if (fFormat == VK_FORMAT_UNDEFINED && formatFeatures) {
        if (fXChromaOffset == VK_CHROMA_LOCATION_MIDPOINT ||
            fYChromaOffset == VK_CHROMA_LOCATION_MIDPOINT) {
            SkASSERT(formatFeatures & VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT);
        }
        if (fXChromaOffset == VK_CHROMA_LOCATION_COSITED_EVEN ||
            fYChromaOffset == VK_CHROMA_LOCATION_COSITED_EVEN) {
            SkASSERT(formatFeatures & VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT);
        }
        if (fChromaFilter == VK_FILTER_LINEAR) {
            SkASSERT(formatFeatures &
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT);
        }
        if (fForceExplicitReconstruction) {
            SkASSERT(formatFeatures &
                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT);
        }
    }
#endif
    fSamplerFilterMustMatchChromaFilter = !SkToBool(
        formatFeatures &
        VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT);
    fSupportsLinearFilter = SkToBool(formatFeatures &
                                     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT);
    if (fSamplerFilterMustMatchChromaFilter && !fSupportsLinearFilter) {
        // Because we don't have separate reconstruction filter, the min, mag and  filter must
        // all match. However, we also don't support linear sampling so the min/mag filter have to
        // be nearest. Therefore, we force the chroma filter to be nearest regardless of support for
        // the feature VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT.
        fChromaFilter = VK_FILTER_NEAREST;
    }
}


void VulkanYcbcrConversionInfo::toVkSamplerYcbcrConversionCreateInfo(
        VkSamplerYcbcrConversionCreateInfo* outInfo,
        std::optional<VkFilter>* requiredSamplerFilter) const {
    outInfo->sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO;
    outInfo->pNext = nullptr;
    outInfo->format = fFormat;
    outInfo->ycbcrModel = fYcbcrModel;
    outInfo->ycbcrRange = fYcbcrRange;
    outInfo->components = fComponents;
    outInfo->xChromaOffset = fXChromaOffset;
    outInfo->yChromaOffset = fYChromaOffset;
    outInfo->chromaFilter = fChromaFilter;
    outInfo->forceExplicitReconstruction = fForceExplicitReconstruction;

    if (fSamplerFilterMustMatchChromaFilter) {
        *requiredSamplerFilter = fChromaFilter;
    } else if (!fSupportsLinearFilter) {
        *requiredSamplerFilter = VK_FILTER_NEAREST;
    }
}

} // namespace skgpu
