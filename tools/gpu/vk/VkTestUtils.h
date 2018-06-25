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

#include "vk/GrVkDefines.h"
#include "vk/GrVkInterface.h"

struct GrVkBackendContext;

namespace sk_gpu_test {
    bool LoadVkLibraryAndGetProcAddrFuncs(PFN_vkGetInstanceProcAddr*, PFN_vkGetDeviceProcAddr*);

    using CanPresentFn = std::function<bool(VkInstance, VkPhysicalDevice,
                                            uint32_t queueFamilyIndex)>;

    bool CreateVkBackendContext(const GrVkInterface::GetInstanceProc& getInstanceProc,
                                const GrVkInterface::GetDeviceProc& getDeviceProc,
                                GrVkBackendContext* ctx,
                                VkDebugReportCallbackEXT* debugCallback,
                                uint32_t* presentQueueIndexPtr = nullptr,
                                CanPresentFn canPresent = CanPresentFn());
}

#endif
#endif

