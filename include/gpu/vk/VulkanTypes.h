/*
 * Copyright 2022 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_VulkanTypes_DEFINED
#define skgpu_VulkanTypes_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/gpu/vk/SkiaVulkan.h"

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

#ifndef VK_VERSION_1_1
#error Skia requires the use of Vulkan 1.1 headers
#endif

namespace skgpu {

using VulkanGetProc = std::function<PFN_vkVoidFunction(
        const char*, // function name
        VkInstance,  // instance or VK_NULL_HANDLE
        VkDevice     // device or VK_NULL_HANDLE
        )>;

typedef intptr_t VulkanBackendMemory;

/**
 * Types for interacting with Vulkan resources created externally to Skia.
 */
struct VulkanAlloc {
    // can be VK_NULL_HANDLE iff is an RT and is borrowed
    VkDeviceMemory      fMemory = VK_NULL_HANDLE;
    VkDeviceSize        fOffset = 0;
    VkDeviceSize        fSize = 0;  // this can be indeterminate iff Tex uses borrow semantics
    uint32_t            fFlags = 0;
    // handle to memory allocated via skgpu::VulkanMemoryAllocator.
    VulkanBackendMemory fBackendMemory = 0;

    enum Flag {
        kNoncoherent_Flag     = 0x1,   // memory must be flushed to device after mapping
        kMappable_Flag        = 0x2,   // memory is able to be mapped.
        kLazilyAllocated_Flag = 0x4,   // memory was created with lazy allocation
    };

    bool operator==(const VulkanAlloc& that) const {
        return fMemory == that.fMemory && fOffset == that.fOffset && fSize == that.fSize &&
               fFlags == that.fFlags && fUsesSystemHeap == that.fUsesSystemHeap;
    }

private:
    bool fUsesSystemHeap = false;
};

// Used to pass in the necessary information to create a VkSamplerYcbcrConversion object for an
// VkExternalFormatANDROID.
struct VulkanYcbcrConversionInfo {
    bool operator==(const VulkanYcbcrConversionInfo& that) const {
        // Invalid objects are not required to have all other fields initialized or matching.
        if (!this->isValid() && !that.isValid()) {
            return true;
        }
        return this->fFormat == that.fFormat &&
               this->fExternalFormat == that.fExternalFormat &&
               this->fYcbcrModel == that.fYcbcrModel &&
               this->fYcbcrRange == that.fYcbcrRange &&
               this->fXChromaOffset == that.fXChromaOffset &&
               this->fYChromaOffset == that.fYChromaOffset &&
               this->fChromaFilter == that.fChromaFilter &&
               this->fForceExplicitReconstruction == that.fForceExplicitReconstruction &&
               this->fComponents.r == that.fComponents.r &&
               this->fComponents.g == that.fComponents.g &&
               this->fComponents.b == that.fComponents.b &&
               this->fComponents.a == that.fComponents.a;
    }
    bool operator!=(const VulkanYcbcrConversionInfo& that) const { return !(*this == that); }

    bool isValid() const {
        return fYcbcrModel != VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY ||
               fExternalFormat != 0;
    }

    // Format of the source image. Must be set to VK_FORMAT_UNDEFINED for external images or
    // a valid image format otherwise.
    VkFormat fFormat = VK_FORMAT_UNDEFINED;

    // The external format. Must be non-zero for external images, zero otherwise.
    // Should be compatible to be used in a VkExternalFormatANDROID struct.
    uint64_t fExternalFormat = 0;

    VkSamplerYcbcrModelConversion fYcbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
    VkSamplerYcbcrRange fYcbcrRange           = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
    VkChromaLocation fXChromaOffset           = VK_CHROMA_LOCATION_COSITED_EVEN;
    VkChromaLocation fYChromaOffset           = VK_CHROMA_LOCATION_COSITED_EVEN;
    VkFilter fChromaFilter                    = VK_FILTER_NEAREST;
    VkBool32 fForceExplicitReconstruction     = false;

    // For external images format features here should be those returned by a call to
    // vkAndroidHardwareBufferFormatPropertiesANDROID
    VkFormatFeatureFlags fFormatFeatures = 0;

    // This is ignored when fExternalFormat is non-zero.
    VkComponentMapping fComponents            = {VK_COMPONENT_SWIZZLE_IDENTITY,
                                                 VK_COMPONENT_SWIZZLE_IDENTITY,
                                                 VK_COMPONENT_SWIZZLE_IDENTITY,
                                                 VK_COMPONENT_SWIZZLE_IDENTITY};
};

typedef void* VulkanDeviceLostContext;
typedef void (*VulkanDeviceLostProc)(VulkanDeviceLostContext faultContext,
                                     const std::string& description,
                                     const std::vector<VkDeviceFaultAddressInfoEXT>& addressInfos,
                                     const std::vector<VkDeviceFaultVendorInfoEXT>& vendorInfos,
                                     const std::vector<std::byte>& vendorBinaryData);

} // namespace skgpu

#endif // skgpu_VulkanTypes_DEFINED
