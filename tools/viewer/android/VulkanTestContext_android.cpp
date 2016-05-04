
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "VulkanTestContext_android.h"

#include "vk/GrVkInterface.h"
#include "vk/GrVkUtil.h"

VkSurfaceKHR VulkanTestContext::createVkSurface(VkInstance instance, void* platformData) {
    static PFN_vkCreateAndroidSurfaceKHR createAndroidSurfaceKHR = nullptr;
    if (!createAndroidSurfaceKHR) {
        createAndroidSurfaceKHR = (PFN_vkCreateAndroidSurfaceKHR)vkGetInstanceProcAddr(instance,
                                                                       "vkCreateAndroidSurfaceKHR");
    }

    if (!platformData) {
        return VK_NULL_HANDLE;
    }
    ContextPlatformData_android* androidPlatformData =
                                           reinterpret_cast<ContextPlatformData_android*>(platformData);
    VkSurfaceKHR surface;

    VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo;
    memset(&surfaceCreateInfo, 0, sizeof(VkAndroidSurfaceCreateInfoKHR));
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.pNext = nullptr;
    surfaceCreateInfo.flags = 0;
    surfaceCreateInfo.window = androidPlatformData->fNativeWindow;

    VkResult res = createAndroidSurfaceKHR(instance, &surfaceCreateInfo,
                                           nullptr, &surface);
    return (VK_SUCCESS == res) ? surface : VK_NULL_HANDLE;
}

bool VulkanTestContext::canPresent(VkInstance instance, VkPhysicalDevice physDev,
                                   uint32_t queueFamilyIndex) {
    return true;
}
