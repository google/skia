/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef VkYcbcrSamplerHelper_DEFINED
#define VkYcbcrSamplerHelper_DEFINED

#include "include/core/SkTypes.h"

#ifdef SK_VULKAN

#include "include/core/SkRefCnt.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/vk/GrVkBackendContext.h"
#include "include/gpu/vk/GrVkExtensions.h"

class GrContext;
class SkImage;

#define DECLARE_VK_PROC(name) PFN_vk##name fVk##name

class VkYcbcrSamplerHelper {
public:
    VkYcbcrSamplerHelper();
    ~VkYcbcrSamplerHelper();

    bool init();

    bool isYCbCrSupported() const;

    sk_sp<SkImage> createI420Image(uint32_t width, uint32_t height);

    GrContext* getGrContext() { return fGrContext.get(); }

    static int GetExpectedY(int x, int y, int width, int height);
    static std::pair<int, int> GetExpectedUV(int x, int y, int width, int height);

private:
    GrVkExtensions fExtensions;
    VkPhysicalDeviceFeatures2 fFeatures = {};
    VkDebugReportCallbackEXT fDebugCallback = VK_NULL_HANDLE;

    DECLARE_VK_PROC(DestroyInstance);
    DECLARE_VK_PROC(DeviceWaitIdle);
    DECLARE_VK_PROC(DestroyDevice);

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

    VkDevice fDevice = VK_NULL_HANDLE;

    PFN_vkDestroyDebugReportCallbackEXT fDestroyDebugCallback = nullptr;

    GrVkBackendContext fBackendContext;
    sk_sp<GrContext> fGrContext;

    VkImage fImage = VK_NULL_HANDLE;
    VkDeviceMemory fImageMemory = VK_NULL_HANDLE;
    GrBackendTexture fTexture;
};

#undef DECLARE_VK_PROC

#endif // SK_VULKAN

#endif // VkYcbcrSamplerHelper_DEFINED
