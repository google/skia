
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../VulkanWindowContext.h"
#include "Window_win.h"

#include "vk/GrVkInterface.h"
#include "vk/GrVkUtil.h"

namespace sk_app {

// Platform dependant call
VkSurfaceKHR VulkanWindowContext::createVkSurface(VkInstance instance, void* platformData) {
    static PFN_vkCreateWin32SurfaceKHR createWin32SurfaceKHR = nullptr;
    if (!createWin32SurfaceKHR) {
        createWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR) vkGetInstanceProcAddr(instance,
                                                                        "vkCreateWin32SurfaceKHR");
    }

    if (!platformData) {
        return VK_NULL_HANDLE;
    }
    ContextPlatformData_win* winPlatformData =
                                          reinterpret_cast<ContextPlatformData_win*>(platformData);
    VkSurfaceKHR surface;

    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo;
    memset(&surfaceCreateInfo, 0, sizeof(VkWin32SurfaceCreateInfoKHR));
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.pNext = nullptr;
    surfaceCreateInfo.flags = 0;
    surfaceCreateInfo.hinstance = winPlatformData->fHInstance;
    surfaceCreateInfo.hwnd = winPlatformData->fHWnd;

    VkResult res = createWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface);
    if (VK_SUCCESS != res) {
        return VK_NULL_HANDLE;
    }

    return surface;
}

// Platform dependant call
bool VulkanWindowContext::canPresent(VkInstance instance, VkPhysicalDevice physDev,
                                     uint32_t queueFamilyIndex, void*) {
    static PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR
                                            getPhysicalDeviceWin32PresentationSupportKHR = nullptr;
    if (!getPhysicalDeviceWin32PresentationSupportKHR) {
        getPhysicalDeviceWin32PresentationSupportKHR =
            (PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR) vkGetInstanceProcAddr(instance,
                                                 "vkGetPhysicalDeviceWin32PresentationSupportKHR");
    }

    VkBool32 check = getPhysicalDeviceWin32PresentationSupportKHR(physDev, queueFamilyIndex);
    return (VK_FALSE != check);
}

}   // namespace sk_app
