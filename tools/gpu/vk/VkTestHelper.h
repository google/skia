/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef VkTestHelper_DEFINED
#define VkTestHelper_DEFINED

#include "include/core/SkTypes.h"

#ifdef SK_VULKAN

#include "include/core/SkRefCnt.h"
#include "include/gpu/vk/GrVkBackendContext.h"
#include "include/gpu/vk/GrVkExtensions.h"

class GrContext;
class SkSurface;

#define DECLARE_VK_PROC(name) PFN_vk##name fVk##name

class VkTestHelper {
public:
    VkTestHelper(bool isProtected) : fIsProtected(isProtected) {}

    ~VkTestHelper() {
        this->cleanup();
    }

    bool init();

    GrContext* grContext() { return fGrContext.get(); }

private:
    void cleanup();

    DECLARE_VK_PROC(DestroyInstance);
    DECLARE_VK_PROC(DeviceWaitIdle);
    DECLARE_VK_PROC(DestroyDevice);

    bool fIsProtected = false;
    VkDevice fDevice = VK_NULL_HANDLE;

    GrVkExtensions* fExtensions = nullptr;
    VkPhysicalDeviceFeatures2* fFeatures = nullptr;
    VkDebugReportCallbackEXT fDebugCallback = VK_NULL_HANDLE;
    PFN_vkDestroyDebugReportCallbackEXT fDestroyDebugCallback = nullptr;
    GrVkBackendContext fBackendContext;
    sk_sp<GrContext> fGrContext;
};

#undef DECLARE_VK_PROC

#endif // SK_VULKAN
#endif // VkTestHelper_DEFINED
