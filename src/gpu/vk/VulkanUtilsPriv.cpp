/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/vk/VulkanUtilsPriv.h"

#include "include/core/SkStream.h"
#include "include/gpu/vk/VulkanBackendContext.h"
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

/**
 * Returns a populated VkSamplerYcbcrConversionCreateInfo object based on VulkanYcbcrConversionInfo
*/
void SetupSamplerYcbcrConversionInfo(VkSamplerYcbcrConversionCreateInfo* outInfo,
                                     std::optional<VkFilter>* requiredSamplerFilter,
                                     const VulkanYcbcrConversionInfo& conversionInfo) {
#ifdef SK_DEBUG
    const VkFormatFeatureFlags& featureFlags = conversionInfo.fFormatFeatures;

    // Format feature flags are only representative of an external format's capabilities, so skip
    // these checks in the case of using a known format or if the featureFlags were clearly not
    // filled in.
    if (conversionInfo.fFormat == VK_FORMAT_UNDEFINED && featureFlags) {
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
    }
#endif

    VkFilter chromaFilter = conversionInfo.fChromaFilter;
    if (!(conversionInfo.fFormatFeatures &
          VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT)) {
        if (!(conversionInfo.fFormatFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
            // Because we don't have have separate reconstruction filter, the min, mag and
            // chroma filter must all match. However, we also don't support linear sampling so
            // the min/mag filter have to be nearest. Therefore, we force the chroma filter to
            // be nearest regardless of support for the feature
            // VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT.
            chromaFilter = VK_FILTER_NEAREST;
        }

        // Let the caller know that it must match min and mag filters with the chroma filter.
        *requiredSamplerFilter = chromaFilter;
    }

    outInfo->sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO;
    outInfo->pNext = nullptr;
    outInfo->format = conversionInfo.fFormat;
    outInfo->ycbcrModel = conversionInfo.fYcbcrModel;
    outInfo->ycbcrRange = conversionInfo.fYcbcrRange;
    outInfo->components = conversionInfo.fComponents;
    outInfo->xChromaOffset = conversionInfo.fXChromaOffset;
    outInfo->yChromaOffset = conversionInfo.fYChromaOffset;
    outInfo->chromaFilter = chromaFilter;
    outInfo->forceExplicitReconstruction = conversionInfo.fForceExplicitReconstruction;
}

bool SerializeVkYCbCrInfo(SkWStream* stream, const VulkanYcbcrConversionInfo& info) {
    SkASSERT(SkTFitsIn<uint64_t>(info.fFormat));
    // fExternalFormat is already a uint64_t
    SkASSERT(SkTFitsIn<uint8_t>(info.fYcbcrModel));
    SkASSERT(SkTFitsIn<uint8_t>(info.fYcbcrRange));
    SkASSERT(SkTFitsIn<uint8_t>(info.fXChromaOffset));
    SkASSERT(SkTFitsIn<uint8_t>(info.fYChromaOffset));
    SkASSERT(SkTFitsIn<uint64_t>(info.fChromaFilter));
    SkASSERT(SkTFitsIn<uint64_t>(info.fFormatFeatures));
    SkASSERT(SkTFitsIn<uint8_t>(info.fComponents.r));
    SkASSERT(SkTFitsIn<uint8_t>(info.fComponents.g));
    SkASSERT(SkTFitsIn<uint8_t>(info.fComponents.b));
    SkASSERT(SkTFitsIn<uint8_t>(info.fComponents.a));
    // fForceExplicitReconstruction is a VkBool32

    // TODO(robertphillips): this isn't as densely packed as possible
    if (!stream->write64(static_cast<uint64_t>(info.fFormat)))           { return false; }
    if (!stream->write64(info.fExternalFormat))                          { return false; }
    if (!stream->write8(static_cast<uint8_t>(info.fYcbcrModel)))         { return false; }
    if (!info.isValid()) {
        return true;
    }

    if (!stream->write8(static_cast<uint8_t>(info.fYcbcrRange)))         { return false; }
    if (!stream->write8(static_cast<uint8_t>(info.fXChromaOffset)))      { return false; }
    if (!stream->write8(static_cast<uint8_t>(info.fYChromaOffset)))      { return false; }
    if (!stream->write64(static_cast<uint64_t>(info.fChromaFilter)))     { return false; }
    if (!stream->write64(static_cast<uint64_t>(info.fFormatFeatures)))   { return false; }
    if (!stream->write8(static_cast<uint8_t>(info.fComponents.r)))       { return false; }
    if (!stream->write8(static_cast<uint8_t>(info.fComponents.g)))       { return false; }
    if (!stream->write8(static_cast<uint8_t>(info.fComponents.b)))       { return false; }
    if (!stream->write8(static_cast<uint8_t>(info.fComponents.a)))       { return false; }
    if (!stream->writeBool(SkToBool(info.fForceExplicitReconstruction))) { return false;}

    return true;
}

bool DeserializeVkYCbCrInfo(SkStream* stream, VulkanYcbcrConversionInfo* out) {
    uint64_t tmp64;
    uint8_t tmp8;

    if (!stream->readU64(&tmp64)) { return false; }
    out->fFormat = static_cast<VkFormat>(tmp64);

    if (!stream->readU64(&tmp64)) { return false; }
    out->fExternalFormat = tmp64;

    if (!stream->readU8(&tmp8)) { return false; }
    out->fYcbcrModel = static_cast<VkSamplerYcbcrModelConversion>(tmp8);

    if (!out->isValid()) {
        return true;
    }

    if (!stream->readU8(&tmp8)) { return false; }
    out->fYcbcrRange = static_cast<VkSamplerYcbcrRange>(tmp8);

    if (!stream->readU8(&tmp8)) { return false; }
    out->fXChromaOffset = static_cast<VkChromaLocation>(tmp8);

    if (!stream->readU8(&tmp8)) { return false; }
    out->fYChromaOffset = static_cast<VkChromaLocation>(tmp8);

    if (!stream->readU64(&tmp64)) { return false; }
    out->fChromaFilter = static_cast<VkFilter>(tmp64);

    if (!stream->readU64(&tmp64)) { return false; }
    out->fFormatFeatures = static_cast<VkFormatFeatureFlags>(tmp64);

    if (!stream->readU8(&tmp8)) { return false; }
    out->fComponents.r = static_cast<VkComponentSwizzle>(tmp8);

    if (!stream->readU8(&tmp8)) { return false; }
    out->fComponents.g = static_cast<VkComponentSwizzle>(tmp8);

    if (!stream->readU8(&tmp8)) { return false; }
    out->fComponents.b = static_cast<VkComponentSwizzle>(tmp8);

    if (!stream->readU8(&tmp8)) { return false; }
    out->fComponents.a = static_cast<VkComponentSwizzle>(tmp8);

    bool tmpBool;
    if (!stream->readBool(&tmpBool)) { return false; }
    out->fForceExplicitReconstruction = tmpBool;

    return false;
}

#ifdef SK_BUILD_FOR_ANDROID

/**
 * Shared Vulkan AHardwareBuffer utility functions between graphite and ganesh
*/
void GetYcbcrConversionInfoFromFormatProps(
        VulkanYcbcrConversionInfo* outConversionInfo,
        const VkAndroidHardwareBufferFormatPropertiesANDROID& formatProps) {
    outConversionInfo->fYcbcrModel = formatProps.suggestedYcbcrModel;
    outConversionInfo->fYcbcrRange = formatProps.suggestedYcbcrRange;
    outConversionInfo->fComponents = formatProps.samplerYcbcrConversionComponents;
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
        SkDebugf("Vulkan 1.1 is required but not available. "
                 "Instance version: %#08X, Device version: %#08X",
                 instanceVersion, physDevVersion);
        return nullptr;
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

} // namespace skgpu
