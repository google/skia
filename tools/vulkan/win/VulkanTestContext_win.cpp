
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "VulkanTestContext_win.h"

#include "vk/GrVkInterface.h"
#include "../../src/gpu/vk/GrVkUtil.h"

// Platform dependant call
VkSurfaceKHR VulkanTestContext::createVkSurface(void* platformData) {
    // need better error handling here
    SkASSERT(platformData);
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

    VkResult res = GR_VK_CALL(fBackendContext->fInterface,
                              CreateWin32SurfaceKHR(fBackendContext->fInstance, &surfaceCreateInfo,
                              nullptr, &surface));
    if (VK_SUCCESS != res) {
        return VK_NULL_HANDLE;
    }

    return surface;
}

// Platform dependant call
bool VulkanTestContext::canPresent(uint32_t queueFamilyIndex) {
    VkBool32 check = GR_VK_CALL(fBackendContext->fInterface,
                                GetPhysicalDeviceWin32PresentationSupportKHR(
                                                                   fBackendContext->fPhysicalDevice,
                                                                   queueFamilyIndex));
    return (VK_FALSE != check);
}
