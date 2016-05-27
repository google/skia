
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../VulkanWindowContext.h"
#include "Window_unix.h"

#include "vk/GrVkInterface.h"
#include "vk/GrVkUtil.h"

#include <X11/Xlib-xcb.h>

namespace sk_app {

// Platform dependant call
VkSurfaceKHR VulkanWindowContext::createVkSurface(VkInstance instance, void* platformData) {
    static PFN_vkCreateXcbSurfaceKHR createXcbSurfaceKHR = nullptr;
    if (!createXcbSurfaceKHR) {
        createXcbSurfaceKHR = (PFN_vkCreateXcbSurfaceKHR) vkGetInstanceProcAddr(instance,
                                                                        "vkCreateXcbSurfaceKHR");
    }

    if (!platformData) {
        return VK_NULL_HANDLE;
    }
    ContextPlatformData_unix* unixPlatformData =
                                          reinterpret_cast<ContextPlatformData_unix*>(platformData);


    VkSurfaceKHR surface;

    VkXcbSurfaceCreateInfoKHR surfaceCreateInfo;
    memset(&surfaceCreateInfo, 0, sizeof(VkXcbSurfaceCreateInfoKHR));
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.pNext = nullptr;
    surfaceCreateInfo.flags = 0;
    surfaceCreateInfo.connection = XGetXCBConnection(unixPlatformData->fDisplay);
    surfaceCreateInfo.window = unixPlatformData->fWindow;

    VkResult res = createXcbSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface);
    if (VK_SUCCESS != res) {
        return VK_NULL_HANDLE;
    }

    return surface;
}

// Platform dependant call
bool VulkanWindowContext::canPresent(VkInstance instance, VkPhysicalDevice physDev,
                                     uint32_t queueFamilyIndex, void* platformData) {
    static PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR
                                            getPhysicalDeviceXcbPresentationSupportKHR = nullptr;
    if (!getPhysicalDeviceXcbPresentationSupportKHR) {
        getPhysicalDeviceXcbPresentationSupportKHR =
            (PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR) vkGetInstanceProcAddr(instance,
                                                 "vkGetPhysicalDeviceXcbPresentationSupportKHR");
    }

    ContextPlatformData_unix* unixPlatformData =
                                          reinterpret_cast<ContextPlatformData_unix*>(platformData);

    Display* display = unixPlatformData->fDisplay;
    VisualID visualID = unixPlatformData->fVisualInfo->visualid;
    VkBool32 check = getPhysicalDeviceXcbPresentationSupportKHR(physDev, 
                                                                queueFamilyIndex, 
                                                                XGetXCBConnection(display),
                                                                visualID);
    return (VK_FALSE != check);
}

}   // namespace sk_app
