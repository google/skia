
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "VulkanTestContext_android.h"

#include "vk/GrVkInterface.h"
#include "../../src/gpu/vk/GrVkUtil.h"

VkSurfaceKHR VulkanTestContext::createVkSurface(void* platformData) {
    // need better error handling here
    SkASSERT(platformData);
    ContextPlatformData_android* androidPlatformData =
                                           reinterpret_cast<ContextPlatformData_android*>(platformData);
    VkSurfaceKHR surface;

    VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo;
    memset(&surfaceCreateInfo, 0, sizeof(VkAndroidSurfaceCreateInfoKHR));
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.pNext = nullptr;
    surfaceCreateInfo.flags = 0;
    surfaceCreateInfo.window = androidPlatformData->fNativeWindow;

    VkResult res = GR_VK_CALL(fBackendContext->fInterface,
                              CreateAndroidSurfaceKHR(fBackendContext->fInstance,
                                                      &surfaceCreateInfo,
                                                      nullptr, &surface));
    return (VK_SUCCESS == res) ? surface : VK_NULL_HANDLE;
}

bool VulkanTestContext::canPresent(uint32_t queueFamilyIndex) {
    return true;
}
