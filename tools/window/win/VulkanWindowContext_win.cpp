
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/window/win/WindowContextFactory_win.h"

#include "tools/sk_app/win/Window_win.h"
#include "tools/window/VulkanWindowContext.h"

#include "src/gpu/ganesh/vk/GrVkUtil.h"

#include "tools/gpu/vk/VkTestUtils.h"

#include <Windows.h>

namespace skwindow {

std::unique_ptr<WindowContext> MakeVulkanForWin(HWND hwnd, const DisplayParams& params) {
    PFN_vkGetInstanceProcAddr instProc;
    if (!sk_gpu_test::LoadVkLibraryAndGetProcAddrFuncs(&instProc)) {
        return nullptr;
    }

    auto createVkSurface = [hwnd, instProc] (VkInstance instance) -> VkSurfaceKHR {
        static PFN_vkCreateWin32SurfaceKHR createWin32SurfaceKHR = nullptr;
        if (!createWin32SurfaceKHR) {
            createWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)
                instProc(instance, "vkCreateWin32SurfaceKHR");
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

    auto canPresent = [instProc] (VkInstance instance, VkPhysicalDevice physDev,
                                  uint32_t queueFamilyIndex) {
        static PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR
                                            getPhysicalDeviceWin32PresentationSupportKHR = nullptr;
        if (!getPhysicalDeviceWin32PresentationSupportKHR) {
            getPhysicalDeviceWin32PresentationSupportKHR =
                (PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR)
                    instProc(instance, "vkGetPhysicalDeviceWin32PresentationSupportKHR");
        }

        VkBool32 check = getPhysicalDeviceWin32PresentationSupportKHR(physDev, queueFamilyIndex);
        return (VK_FALSE != check);
    };

    std::unique_ptr<WindowContext> ctx(
            new internal::VulkanWindowContext(params, createVkSurface, canPresent, instProc));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}

}  // namespace skwindow
