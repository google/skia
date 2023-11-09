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
#include "include/gpu/vk/VulkanBackendContext.h"
#include "include/gpu/vk/VulkanExtensions.h"

class GrDirectContext;
class SkSurface;
struct SkISize;

#define DECLARE_VK_PROC(name) PFN_vk##name fVk##name

class VkTestHelper {
public:
    static std::unique_ptr<VkTestHelper> Make(bool isProtected);

    ~VkTestHelper() {
        this->cleanup();
    }

    bool isValid() const { return fDirectContext != nullptr; }

    sk_sp<SkSurface> createSurface(SkISize, bool textureable, bool isProtected);
    void submitAndWaitForCompletion(bool* completionMarker);

    GrDirectContext* directContext() { return fDirectContext.get(); }

private:
    VkTestHelper(bool isProtected) : fIsProtected(isProtected) {}

    bool init();
    void cleanup();

    DECLARE_VK_PROC(DestroyInstance);
    DECLARE_VK_PROC(DeviceWaitIdle);
    DECLARE_VK_PROC(DestroyDevice);
    DECLARE_VK_PROC(GetDeviceProcAddr);

    DECLARE_VK_PROC(GetPhysicalDeviceFormatProperties);
    DECLARE_VK_PROC(GetPhysicalDeviceMemoryProperties);

    DECLARE_VK_PROC(CreateImage);
    DECLARE_VK_PROC(DestroyImage);
    DECLARE_VK_PROC(GetImageMemoryRequirements);
    DECLARE_VK_PROC(AllocateMemory);
    DECLARE_VK_PROC(FreeMemory);
    DECLARE_VK_PROC(BindImageMemory);
    DECLARE_VK_PROC(MapMemory);
    DECLARE_VK_PROC(UnmapMemory);
    DECLARE_VK_PROC(FlushMappedMemoryRanges);
    DECLARE_VK_PROC(GetImageSubresourceLayout);

    bool fIsProtected = false;
    VkDevice fDevice = VK_NULL_HANDLE;

    skgpu::VulkanExtensions fExtensions;
    VkPhysicalDeviceFeatures2 fFeatures = {};
    VkDebugReportCallbackEXT fDebugCallback = VK_NULL_HANDLE;
    PFN_vkDestroyDebugReportCallbackEXT fDestroyDebugCallback = nullptr;
    skgpu::VulkanBackendContext fBackendContext;
    sk_sp<GrDirectContext> fDirectContext;
};

#undef DECLARE_VK_PROC

#endif // SK_VULKAN
#endif // VkTestHelper_DEFINED
