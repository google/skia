/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_VulkanUtilsPriv_DEFINED
#define skgpu_VulkanUtilsPriv_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkRefCnt.h"
#include "include/gpu/vk/VulkanTypes.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkMath.h"
#include "include/private/gpu/vk/SkiaVulkan.h"
#include "src/gpu/SkSLToBackend.h"
#include "src/sksl/codegen/SkSLSPIRVCodeGenerator.h"

#ifdef SK_BUILD_FOR_ANDROID
#include <android/hardware_buffer.h>
#endif

#include <cstddef>
#include <cstdint>
#include <string>
#include <type_traits>

namespace SkSL {

struct NativeShader;
enum class ProgramKind : int8_t;
struct ProgramInterface;
struct ProgramSettings;
struct ShaderCaps;

}  // namespace SkSL

namespace skgpu {

class ShaderErrorHandler;
struct VulkanInterface;
struct VulkanBackendContext;
class VulkanExtensions;

enum VkVendor {
    kAMD_VkVendor = 0x1002,
    kARM_VkVendor = 0x13B5,
    kBroadcom_VkVendor = 0x14E4,
    kGoogle_VkVendor = 0x1AE0,
    kImagination_VkVendor = 0x1010,
    kIntel_VkVendor = 0x8086,
    kKazan_VkVendor = 0x10003,
    kMesa_VkVendor = 0x10005,
    kNvidia_VkVendor = 0x10DE,
    kQualcomm_VkVendor = 0x5143,
    kSamsung_VkVendor = 0x144D,
    kVeriSilicon_VkVendor = 0x10002,
    kVivante_VkVendor = 0x10001,
};

// GPU driver version, used for driver bug workarounds.
struct DriverVersion {
    constexpr DriverVersion() {}

    constexpr DriverVersion(int major, int minor) : fMajor(major), fMinor(minor) {}

    uint32_t fMajor = 0;
    uint32_t fMinor = 0;
};

inline constexpr bool operator==(const DriverVersion& a, const DriverVersion& b) {
    return a.fMajor == b.fMajor && a.fMinor == b.fMinor;
}
inline constexpr bool operator!=(const DriverVersion& a, const DriverVersion& b) {
    return !(a == b);
}
inline constexpr bool operator<(const DriverVersion& a, const DriverVersion& b) {
    return a.fMajor < b.fMajor || (a.fMajor == b.fMajor && a.fMinor < b.fMinor);
}
inline constexpr bool operator>=(const DriverVersion& a, const DriverVersion& b) {
    return !(a < b);
}

static_assert(DriverVersion(3, 4) == DriverVersion(3, 4));
static_assert(DriverVersion(3, 4) != DriverVersion(4, 3));
static_assert(DriverVersion(2, 3)  < DriverVersion(2, 4));
static_assert(DriverVersion(2, 3)  < DriverVersion(3, 0));
static_assert(DriverVersion(2, 3) >= DriverVersion(2, 1));
static_assert(DriverVersion(2, 3) >= DriverVersion(2, 3));
static_assert(DriverVersion(2, 3) >= DriverVersion(1, 8));

DriverVersion ParseVulkanDriverVersion(VkDriverId driverId, uint32_t driverVersion);

inline bool SkSLToSPIRV(const SkSL::ShaderCaps* caps,
                        const std::string& sksl,
                        SkSL::ProgramKind programKind,
                        const SkSL::ProgramSettings& settings,
                        SkSL::NativeShader* spirv,
                        SkSL::ProgramInterface* outInterface,
                        ShaderErrorHandler* errorHandler) {
    return SkSLToBackend(caps,
                         &SkSL::ToSPIRV,
                         "SPIRV",
                         sksl,
                         programKind,
                         settings,
                         spirv,
                         outInterface,
                         errorHandler);
}

static constexpr uint32_t VkFormatChannels(VkFormat vkFormat) {
    switch (vkFormat) {
        case VK_FORMAT_R8G8B8A8_UNORM:           return kRGBA_SkColorChannelFlags;
        case VK_FORMAT_R8_UNORM:                 return kRed_SkColorChannelFlag;
        case VK_FORMAT_B8G8R8A8_UNORM:           return kRGBA_SkColorChannelFlags;
        case VK_FORMAT_R5G6B5_UNORM_PACK16:      return kRGB_SkColorChannelFlags;
        case VK_FORMAT_B5G6R5_UNORM_PACK16:      return kRGB_SkColorChannelFlags;
        case VK_FORMAT_R16G16B16A16_SFLOAT:      return kRGBA_SkColorChannelFlags;
        case VK_FORMAT_R16_SFLOAT:               return kRed_SkColorChannelFlag;
        case VK_FORMAT_R8G8B8_UNORM:             return kRGB_SkColorChannelFlags;
        case VK_FORMAT_R8G8_UNORM:               return kRG_SkColorChannelFlags;
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32: return kRGBA_SkColorChannelFlags;
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32: return kRGBA_SkColorChannelFlags;
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:    return kRGBA_SkColorChannelFlags;
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:    return kRGBA_SkColorChannelFlags;
        case VK_FORMAT_R8G8B8A8_SRGB:            return kRGBA_SkColorChannelFlags;
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:  return kRGB_SkColorChannelFlags;
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:      return kRGB_SkColorChannelFlags;
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:     return kRGBA_SkColorChannelFlags;
        case VK_FORMAT_R16_UNORM:                return kRed_SkColorChannelFlag;
        case VK_FORMAT_R16G16_UNORM:             return kRG_SkColorChannelFlags;
        case VK_FORMAT_R16G16B16A16_UNORM:       return kRGBA_SkColorChannelFlags;
        case VK_FORMAT_R16G16_SFLOAT:            return kRG_SkColorChannelFlags;
        case VK_FORMAT_S8_UINT:                  return 0;
        case VK_FORMAT_D16_UNORM:                return 0;
        case VK_FORMAT_D32_SFLOAT:               return 0;
        case VK_FORMAT_D24_UNORM_S8_UINT:        return 0;
        case VK_FORMAT_D32_SFLOAT_S8_UINT:       return 0;
        default:                                 return 0;
    }
}

static constexpr size_t VkFormatBytesPerBlock(VkFormat vkFormat) {
    switch (vkFormat) {
        case VK_FORMAT_R8G8B8A8_UNORM:            return 4;
        case VK_FORMAT_R8_UNORM:                  return 1;
        case VK_FORMAT_B8G8R8A8_UNORM:            return 4;
        case VK_FORMAT_R5G6B5_UNORM_PACK16:       return 2;
        case VK_FORMAT_B5G6R5_UNORM_PACK16:       return 2;
        case VK_FORMAT_R16G16B16A16_SFLOAT:       return 8;
        case VK_FORMAT_R16_SFLOAT:                return 2;
        case VK_FORMAT_R8G8B8_UNORM:              return 3;
        case VK_FORMAT_R8G8_UNORM:                return 2;
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:  return 4;
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32:  return 4;
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:     return 2;
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:     return 2;
        case VK_FORMAT_R8G8B8A8_SRGB:             return 4;
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:   return 8;
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:       return 8;
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:      return 8;
        case VK_FORMAT_R16_UNORM:                 return 2;
        case VK_FORMAT_R16G16_UNORM:              return 4;
        case VK_FORMAT_R16G16B16A16_UNORM:        return 8;
        case VK_FORMAT_R16G16_SFLOAT:             return 4;
        case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16: return 8;
        // Currently we are just over estimating this value to be used in gpu size calculations even
        // though the actually size is probably less. We should instead treat planar formats similar
        // to compressed textures that go through their own special query for calculating size.
        case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM: return 3;
        case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:  return 3;
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16: return 6;
        case VK_FORMAT_S8_UINT:                   return 1;
        case VK_FORMAT_D16_UNORM:                 return 2;
        case VK_FORMAT_D32_SFLOAT:                return 4;
        case VK_FORMAT_D24_UNORM_S8_UINT:         return 4;
        case VK_FORMAT_D32_SFLOAT_S8_UINT:        return 8;

        default:                                  return 0;
    }
}

static constexpr int VkFormatStencilBits(VkFormat format) {
    switch (format) {
        case VK_FORMAT_S8_UINT:
            return 8;
        case VK_FORMAT_D24_UNORM_S8_UINT:
            return 8;
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return 8;
        default:
            return 0;
    }
}

static constexpr bool VkFormatNeedsYcbcrSampler(VkFormat format)  {
    return format == VK_FORMAT_G8_B8R8_2PLANE_420_UNORM ||
           format == VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM ||
           format == VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16;
}

static constexpr bool SampleCountToVkSampleCount(uint32_t samples,
                                                 VkSampleCountFlagBits* vkSamples) {
    static_assert(VK_SAMPLE_COUNT_1_BIT == 1);
    static_assert(VK_SAMPLE_COUNT_2_BIT == 2);
    static_assert(VK_SAMPLE_COUNT_4_BIT == 4);
    static_assert(VK_SAMPLE_COUNT_8_BIT == 8);
    static_assert(VK_SAMPLE_COUNT_16_BIT == 16);
    SkASSERT(samples >= 1);

    if (!SkIsPow2(samples) || samples > 16) {
        return false;
    }

    *vkSamples = static_cast<VkSampleCountFlagBits>(samples);
    return true;
}

/**
 * Returns true if the format is compressed.
 */
static constexpr bool VkFormatIsCompressed(VkFormat vkFormat) {
    switch (vkFormat) {
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
            return true;
        default:
            return false;
    }
    SkUNREACHABLE;
}

/**
 * Prepend ptr to the pNext chain at chainStart
 */
template <typename VulkanStruct1, typename VulkanStruct2>
void AddToPNextChain(VulkanStruct1* chainStart, VulkanStruct2* ptr) {
    // Make sure this function is not called with `&pointer` instead of `pointer`.
    static_assert(!std::is_pointer<VulkanStruct1>::value);
    static_assert(!std::is_pointer<VulkanStruct2>::value);

    SkASSERT(ptr->pNext == nullptr);

    VkBaseOutStructure* localPtr = reinterpret_cast<VkBaseOutStructure*>(chainStart);
    ptr->pNext = localPtr->pNext;
    localPtr->pNext = reinterpret_cast<VkBaseOutStructure*>(ptr);
}

/**
 * Returns a ptr to the requested extension feature struct or nullptr if it is not present.
*/
template <typename T>
const T* GetExtensionFeatureStruct(const VkPhysicalDeviceFeatures2& features,
                                   VkStructureType type) {
    // All Vulkan structs that could be part of the features chain will start with the
    // structure type followed by the pNext pointer, as specified in VkBaseInStructure.
    const auto* pNext = static_cast<const VkBaseInStructure*>(features.pNext);
    while (pNext) {
        if (pNext->sType == type) {
            return reinterpret_cast<const T*>(pNext);
        }
        pNext = pNext->pNext;
    }
    return nullptr;
}

static constexpr const char* VkFormatToStr(VkFormat vkFormat) {
    switch (vkFormat) {
        case VK_FORMAT_R8G8B8A8_UNORM:           return "R8G8B8A8_UNORM";
        case VK_FORMAT_R8_UNORM:                 return "R8_UNORM";
        case VK_FORMAT_B8G8R8A8_UNORM:           return "B8G8R8A8_UNORM";
        case VK_FORMAT_R5G6B5_UNORM_PACK16:      return "R5G6B5_UNORM_PACK16";
        case VK_FORMAT_B5G6R5_UNORM_PACK16:      return "B5G6R5_UNORM_PACK16";
        case VK_FORMAT_R16G16B16A16_SFLOAT:      return "R16G16B16A16_SFLOAT";
        case VK_FORMAT_R16_SFLOAT:               return "R16_SFLOAT";
        case VK_FORMAT_R8G8B8_UNORM:             return "R8G8B8_UNORM";
        case VK_FORMAT_R8G8_UNORM:               return "R8G8_UNORM";
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32: return "A2B10G10R10_UNORM_PACK32";
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32: return "A2R10G10B10_UNORM_PACK32";
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:    return "B4G4R4A4_UNORM_PACK16";
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:    return "R4G4B4A4_UNORM_PACK16";
        case VK_FORMAT_R32G32B32A32_SFLOAT:      return "R32G32B32A32_SFLOAT";
        case VK_FORMAT_R8G8B8A8_SRGB:            return "R8G8B8A8_SRGB";
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:  return "ETC2_R8G8B8_UNORM_BLOCK";
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:      return "BC1_RGB_UNORM_BLOCK";
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:     return "BC1_RGBA_UNORM_BLOCK";
        case VK_FORMAT_R16_UNORM:                return "R16_UNORM";
        case VK_FORMAT_R16G16_UNORM:             return "R16G16_UNORM";
        case VK_FORMAT_R16G16B16A16_UNORM:       return "R16G16B16A16_UNORM";
        case VK_FORMAT_R16G16_SFLOAT:            return "R16G16_SFLOAT";
        case VK_FORMAT_S8_UINT:                  return "S8_UINT";
        case VK_FORMAT_D16_UNORM:                return "D16_UNORM";
        case VK_FORMAT_D32_SFLOAT:               return "D32_SFLOAT";
        case VK_FORMAT_D24_UNORM_S8_UINT:        return "D24_UNORM_S8_UINT";
        case VK_FORMAT_D32_SFLOAT_S8_UINT:       return "D32_SFLOAT_S8_UINT";

        default:                                 return "Unknown";
    }
}

static constexpr const char* VkModelToStr(VkSamplerYcbcrModelConversion c) {
    switch (c) {
        case VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY:   return "RGB-I";
        case VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_IDENTITY: return "YCbCr-I";
        case VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709:      return "709";
        case VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_601:      return "601";
        case VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_2020:     return "2020";
        default:                                               return "unknown";
    }
    SkUNREACHABLE;
}

static constexpr const char* VkRangeToStr(VkSamplerYcbcrRange r) {
    switch (r) {
        case VK_SAMPLER_YCBCR_RANGE_ITU_FULL:   return "full";
        case VK_SAMPLER_YCBCR_RANGE_ITU_NARROW: return "narrow";
        default:                                return "unknown";
    }
    SkUNREACHABLE;
}

static constexpr char VkSwizzleToStr(VkComponentSwizzle c, char identityAnswer) {
    switch (c) {
        case VK_COMPONENT_SWIZZLE_IDENTITY: return identityAnswer;
        case VK_COMPONENT_SWIZZLE_ZERO:     return '0';
        case VK_COMPONENT_SWIZZLE_ONE:      return '1';
        case VK_COMPONENT_SWIZZLE_R:        return 'r';
        case VK_COMPONENT_SWIZZLE_G:        return 'g';
        case VK_COMPONENT_SWIZZLE_B:        return 'b';
        case VK_COMPONENT_SWIZZLE_A:        return 'a';
        default:                            return '?';
    }
    SkUNREACHABLE;
}

#ifdef SK_BUILD_FOR_ANDROID
/**
 * Vulkan AHardwareBuffer utility functions shared between graphite and ganesh
*/
void GetYcbcrConversionInfoFromFormatProps(
        VulkanYcbcrConversionInfo* outConversionInfo,
        const VkAndroidHardwareBufferFormatPropertiesANDROID& formatProps);

bool GetAHardwareBufferProperties(
        VkAndroidHardwareBufferFormatPropertiesANDROID* outHwbFormatProps,
        VkAndroidHardwareBufferPropertiesANDROID* outHwbProps,
        const skgpu::VulkanInterface*,
        const AHardwareBuffer*,
        VkDevice);

bool AllocateAndBindImageMemory(skgpu::VulkanAlloc* outVulkanAlloc,
                                VkImage,
                                const VkPhysicalDeviceMemoryProperties2&,
                                const VkAndroidHardwareBufferPropertiesANDROID&,
                                AHardwareBuffer*,
                                const skgpu::VulkanInterface*,
                                VkDevice);

#endif // SK_BUILD_FOR_ANDROID

/**
 * Calls faultProc with faultContext; passes debug info if VK_EXT_device_fault is supported/enabled.
 *
 * Note: must only be called *after* receiving VK_ERROR_DEVICE_LOST.
 */
void InvokeDeviceLostCallback(const skgpu::VulkanInterface* vulkanInterface,
                              VkDevice vkDevice,
                              skgpu::VulkanDeviceLostContext faultContext,
                              skgpu::VulkanDeviceLostProc faultProc,
                              bool supportsDeviceFaultInfoExtension);

sk_sp<skgpu::VulkanInterface> MakeInterface(const skgpu::VulkanBackendContext&,
                                            const skgpu::VulkanExtensions* extOverride,
                                            uint32_t* physDevVersionOut,
                                            uint32_t* instanceVersionOut);

}  // namespace skgpu

#endif // skgpu_VulkanUtilsPriv_DEFINED
