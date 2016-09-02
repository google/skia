
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <Windows.h>
#include "WindowContextFactory_win.h"

#include "../VulkanWindowContext.h"
#include "Window_win.h"

#include "vk/GrVkInterface.h"
#include "vk/GrVkUtil.h"

namespace sk_app {
namespace window_context_factory {

WindowContext* NewVulkanForWin(HWND hwnd, const DisplayParams& params) {
    auto createVkSurface = [hwnd] (VkInstance instance) -> VkSurfaceKHR {
        static PFN_vkCreateWin32SurfaceKHR createWin32SurfaceKHR = nullptr;
        if (!createWin32SurfaceKHR) {
            createWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)
                vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR");
        }
        HINSTANCE hinstance = GetModuleHandle(0);
        VkSurfaceKHR surface;

        VkWin32SurfaceCreateInfoKHR surfaceCreateInfo;
        memset(&surfaceCreateInfo, 0, sizeof(VkWin32SurfaceCreateInfoKHR));
        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        surfaceCreateInfo.pNext = nullptr;
        surfaceCreateInfo.flags = 0;
        surfaceCreateInfo.hinstance = hinstance;
        surfaceCreateInfo.hwnd = hwnd;

        VkResult res = createWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface);
        if (VK_SUCCESS != res) {
            return VK_NULL_HANDLE;
        }

        return surface;
    };

    auto canPresent = [hwnd] (VkInstance instance, VkPhysicalDevice physDev,
                             uint32_t queueFamilyIndex) {
        static PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR
                                            getPhysicalDeviceWin32PresentationSupportKHR = nullptr;
        if (!getPhysicalDeviceWin32PresentationSupportKHR) {
            getPhysicalDeviceWin32PresentationSupportKHR =
                (PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR)
                    vkGetInstanceProcAddr(instance,
                                          "vkGetPhysicalDeviceWin32PresentationSupportKHR");
        }

        VkBool32 check = getPhysicalDeviceWin32PresentationSupportKHR(physDev, queueFamilyIndex);
        return (VK_FALSE != check);
    };

    WindowContext* ctx = new VulkanWindowContext(params, createVkSurface, canPresent);
    if (!ctx->isValid()) {
        delete ctx;
        return nullptr;
    }
    return ctx;
}

}  // namespace window_context_factory
}  // namespace sk_app
