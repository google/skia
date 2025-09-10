/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef VkTestUtils_DEFINED
#define VkTestUtils_DEFINED

#include "include/core/SkTypes.h"

#ifdef SK_VULKAN

#include <functional>
#include "include/gpu/vk/VulkanBackendContext.h"
#include "include/gpu/vk/VulkanPreferredFeatures.h"
#include "tools/gpu/vk/VulkanDefines.h"

namespace skgpu {
struct VulkanBackendContext;
class VulkanExtensions;
}

namespace sk_gpu_test {
    struct TestVkFeatures {
        VkPhysicalDeviceFeatures2 deviceFeatures;

        // protectedMemoryFeatures and structs from skiaFeatures may be chained into deviceFeatures,
        // so must share the same lifetime.
        skgpu::VulkanPreferredFeatures skiaFeatures;
        VkPhysicalDeviceProtectedMemoryFeatures protectedMemoryFeatures;
    };

    bool LoadVkLibraryAndGetProcAddrFuncs(PFN_vkGetInstanceProcAddr*);

    using CanPresentFn = std::function<bool(VkInstance, VkPhysicalDevice,
                                            uint32_t queueFamilyIndex)>;

    bool CreateVkBackendContext(PFN_vkGetInstanceProcAddr getInstProc,
                                skgpu::VulkanBackendContext* ctx,
                                skgpu::VulkanExtensions*,
                                TestVkFeatures*,
                                VkDebugUtilsMessengerEXT* debugMessenger,
                                uint32_t* presentQueueIndexPtr = nullptr,
                                const CanPresentFn& canPresent = CanPresentFn(),
                                bool isProtected = false);

}  // namespace sk_gpu_test

#endif
#endif
