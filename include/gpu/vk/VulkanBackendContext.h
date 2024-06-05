/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_VulkanBackendContext_DEFINED
#define skgpu_VulkanBackendContext_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/vk/VulkanMemoryAllocator.h"
#include "include/gpu/vk/VulkanTypes.h"
#include "include/private/base/SkAPI.h"
#include "include/private/gpu/vk/SkiaVulkan.h"

#include <cstdint>

namespace skgpu {

class VulkanExtensions;

// The VkBackendContext contains all of the base Vk objects needed by the skia Vulkan context.
struct SK_API VulkanBackendContext {
    VkInstance                       fInstance = VK_NULL_HANDLE;
    VkPhysicalDevice                 fPhysicalDevice = VK_NULL_HANDLE;
    VkDevice                         fDevice = VK_NULL_HANDLE;
    VkQueue                          fQueue = VK_NULL_HANDLE;
    uint32_t                         fGraphicsQueueIndex = 0;
    // The max api version set here should match the value set in VkApplicationInfo::apiVersion when
    // then VkInstance was created.
    uint32_t                         fMaxAPIVersion = 0;
    const skgpu::VulkanExtensions*   fVkExtensions = nullptr;
    // The client can create their VkDevice with either a VkPhysicalDeviceFeatures or
    // VkPhysicalDeviceFeatures2 struct, thus we have to support taking both. The
    // VkPhysicalDeviceFeatures2 struct is needed so we know if the client enabled any extension
    // specific features. If fDeviceFeatures2 is not null then we ignore fDeviceFeatures. If both
    // fDeviceFeatures and fDeviceFeatures2 are null we will assume no features are enabled.
    const VkPhysicalDeviceFeatures*  fDeviceFeatures = nullptr;
    const VkPhysicalDeviceFeatures2* fDeviceFeatures2 = nullptr;
    // Optional. The client may provide an inplementation of a VulkanMemoryAllocator for Skia to use
    // for allocating Vulkan resources that use VkDeviceMemory.
    sk_sp<VulkanMemoryAllocator>     fMemoryAllocator;
    skgpu::VulkanGetProc             fGetProc;
    Protected                        fProtectedContext = Protected::kNo;
    // Optional callback which will be invoked if a VK_ERROR_DEVICE_LOST error code is received from
    // the driver. Debug information from the driver will be provided to the callback if the
    // VK_EXT_device_fault extension is supported and enabled (VkPhysicalDeviceFaultFeaturesEXT must
    // be in the pNext chain of VkDeviceCreateInfo).
    skgpu::VulkanDeviceLostContext   fDeviceLostContext = nullptr;
    skgpu::VulkanDeviceLostProc      fDeviceLostProc = nullptr;
};

}  // namespace skgpu

#endif // skgpu_VulkanBackendContext_DEFINED
