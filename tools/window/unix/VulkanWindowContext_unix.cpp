/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/vk/GrVkUtil.h"

#include "tools/gpu/vk/VkTestUtils.h"

#include "tools/window/VulkanWindowContext.h"
#include "tools/window/unix/WindowContextFactory_unix.h"

#include <X11/Xlib-xcb.h>

namespace skwindow {

std::unique_ptr<WindowContext> MakeVulkanForXlib(const XlibWindowInfo& info,
                                                 const DisplayParams& displayParams) {
    PFN_vkGetInstanceProcAddr instProc;
    if (!sk_gpu_test::LoadVkLibraryAndGetProcAddrFuncs(&instProc)) {
        SkDebugf("Could not load vulkan library\n");
        return nullptr;
    }

    auto createVkSurface = [&info, instProc](VkInstance instance) -> VkSurfaceKHR {
        static PFN_vkCreateXcbSurfaceKHR createXcbSurfaceKHR = nullptr;
        if (!createXcbSurfaceKHR) {
            createXcbSurfaceKHR =
                    (PFN_vkCreateXcbSurfaceKHR) instProc(instance, "vkCreateXcbSurfaceKHR");
        }

        VkSurfaceKHR surface;

        VkXcbSurfaceCreateInfoKHR surfaceCreateInfo;
        memset(&surfaceCreateInfo, 0, sizeof(VkXcbSurfaceCreateInfoKHR));
        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
        surfaceCreateInfo.pNext = nullptr;
        surfaceCreateInfo.flags = 0;
        surfaceCreateInfo.connection = XGetXCBConnection(info.fDisplay);
        surfaceCreateInfo.window = info.fWindow;

        VkResult res = createXcbSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface);
        if (VK_SUCCESS != res) {
            return VK_NULL_HANDLE;
        }

        return surface;
    };

    auto canPresent = [&info, instProc](VkInstance instance, VkPhysicalDevice physDev,
                              uint32_t queueFamilyIndex) {
        static PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR
                                                getPhysicalDeviceXcbPresentationSupportKHR = nullptr;
        if (!getPhysicalDeviceXcbPresentationSupportKHR) {
            getPhysicalDeviceXcbPresentationSupportKHR =
                (PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR)
                    instProc(instance, "vkGetPhysicalDeviceXcbPresentationSupportKHR");
        }

        Display* display = info.fDisplay;
        VisualID visualID = XVisualIDFromVisual(DefaultVisual(info.fDisplay,
                                                              DefaultScreen(info.fDisplay)));
        VkBool32 check = getPhysicalDeviceXcbPresentationSupportKHR(physDev,
                                                                    queueFamilyIndex,
                                                                    XGetXCBConnection(display),
                                                                    visualID);
        return (VK_FALSE != check);
    };
    std::unique_ptr<WindowContext> ctx(
            new internal::VulkanWindowContext(displayParams,
                                              createVkSurface,
                                              canPresent,
                                              instProc));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}

}  // namespace skwindow
