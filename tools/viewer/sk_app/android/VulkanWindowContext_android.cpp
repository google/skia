
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "WindowContextFactory_android.h"
#include "../VulkanWindowContext.h"

namespace sk_app {

namespace window_context_factory {

WindowContext* NewVulkanForAndroid(ANativeWindow* window, const DisplayParams& params) {
    auto createVkSurface = [window] (VkInstance instance) -> VkSurfaceKHR {
        PFN_vkCreateAndroidSurfaceKHR createAndroidSurfaceKHR =
                (PFN_vkCreateAndroidSurfaceKHR)vkGetInstanceProcAddr(instance,
                                                                     "vkCreateAndroidSurfaceKHR");

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

    WindowContext* ctx = new VulkanWindowContext(params, createVkSurface, canPresent);
    if (!ctx->isValid()) {
        delete ctx;
        return nullptr;
    }
    return ctx;
}

}  // namespace window_context_factory
}  // namespace sk_app
