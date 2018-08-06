/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef VkTestUtils_DEFINED
#define VkTestUtils_DEFINED

#include "SkTypes.h"

#ifdef SK_VULKAN

#include "GrVulkanDefines.h"
#include "vk/GrVkBackendContext.h"
#include "vk/GrVkTypes.h"
#include <functional>

class GrVkExtensions;
struct GrVkBackendContext;

namespace sk_gpu_test {
    bool LoadVkLibraryAndGetProcAddrFuncs(PFN_vkGetInstanceProcAddr*, PFN_vkGetDeviceProcAddr*);

    using CanPresentFn = std::function<bool(VkInstance, VkPhysicalDevice,
                                            uint32_t queueFamilyIndex)>;

    bool CreateVkBackendContext(GrVkGetProc getProc,
                                GrVkBackendContext* ctx,
                                GrVkExtensions*,
                                VkPhysicalDeviceFeatures2*,
                                VkDebugReportCallbackEXT* debugCallback,
                                uint32_t* presentQueueIndexPtr = nullptr,
                                CanPresentFn canPresent = CanPresentFn());

    void FreeVulkanFeaturesStructs(const VkPhysicalDeviceFeatures2*);
}

#endif
#endif

