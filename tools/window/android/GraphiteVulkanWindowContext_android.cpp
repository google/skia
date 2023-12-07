/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/window/android/WindowContextFactory_android.h"

#include "tools/window/GraphiteVulkanWindowContext.h"

#include "tools/gpu/vk/VkTestUtils.h"

namespace skwindow {

std::unique_ptr<WindowContext> MakeGraphiteVulkanForAndroid(ANativeWindow* window,
                                                            const DisplayParams& params) {
    PFN_vkGetInstanceProcAddr instProc;
    if (!sk_gpu_test::LoadVkLibraryAndGetProcAddrFuncs(&instProc)) {
        return nullptr;
    }

    auto createVkSurface = [window, instProc] (VkInstance instance) -> VkSurfaceKHR {
        PFN_vkCreateAndroidSurfaceKHR createAndroidSurfaceKHR =
                (PFN_vkCreateAndroidSurfaceKHR) instProc(instance, "vkCreateAndroidSurfaceKHR");

        if (!window) {
            return VK_NULL_HANDLE;
        }
        VkSurfaceKHR surface;

        VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo;
        memset(&surfaceCreateInfo, 0, sizeof(VkAndroidSurfaceCreateInfoKHR));
        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
        surfaceCreateInfo.pNext = nullptr;
        surfaceCreateInfo.flags = 0;
        surfaceCreateInfo.window = window;

        VkResult res = createAndroidSurfaceKHR(instance, &surfaceCreateInfo,
                                               nullptr, &surface);
        return (VK_SUCCESS == res) ? surface : VK_NULL_HANDLE;
    };

    auto canPresent = [](VkInstance, VkPhysicalDevice, uint32_t) { return true; };

    std::unique_ptr<WindowContext> ctx(
            new internal::GraphiteVulkanWindowContext(params,
                                                      createVkSurface,
                                                      canPresent,
                                                      instProc));
    if (!ctx->isValid()) {
        return nullptr;
    }

    return ctx;
}

}  // namespace skwindow
