/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/vk/VkTestHelper.h"

#ifdef SK_VULKAN

#include "include/core/SkSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "tools/gpu/vk/VkTestUtils.h"

#define ACQUIRE_INST_VK_PROC(name)                                                           \
    fVk##name = reinterpret_cast<PFN_vk##name>(getProc("vk" #name, fBackendContext.fInstance,\
                                                       VK_NULL_HANDLE));                     \
    if (fVk##name == nullptr) {                                                              \
        SkDebugf("Function ptr for vk%s could not be acquired\n", #name);                    \
        return false;                                                                        \
    }

#define ACQUIRE_DEVICE_VK_PROC(name)                                                          \
    fVk##name = reinterpret_cast<PFN_vk##name>(getProc("vk" #name, VK_NULL_HANDLE, fDevice)); \
    if (fVk##name == nullptr) {                                                               \
        SkDebugf("Function ptr for vk%s could not be acquired\n", #name);                     \
        return false;                                                                         \
    }

bool VkTestHelper::init() {
    PFN_vkGetInstanceProcAddr instProc;
    PFN_vkGetDeviceProcAddr devProc;
    if (!sk_gpu_test::LoadVkLibraryAndGetProcAddrFuncs(&instProc, &devProc)) {
        return false;
    }
    auto getProc = [&instProc, &devProc](const char* proc_name,
                                         VkInstance instance, VkDevice device) {
        if (device != VK_NULL_HANDLE) {
            return devProc(device, proc_name);
        }
        return instProc(instance, proc_name);
    };

    fFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    fFeatures.pNext = nullptr;

    fBackendContext.fInstance = VK_NULL_HANDLE;
    fBackendContext.fDevice = VK_NULL_HANDLE;

    if (!sk_gpu_test::CreateVkBackendContext(getProc, &fBackendContext, &fExtensions,
                                             &fFeatures, &fDebugCallback, nullptr,
                                             sk_gpu_test::CanPresentFn(), fIsProtected)) {
        return false;
    }
    fDevice = fBackendContext.fDevice;

    if (fDebugCallback != VK_NULL_HANDLE) {
        fDestroyDebugCallback = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
                instProc(fBackendContext.fInstance, "vkDestroyDebugReportCallbackEXT"));
    }
    ACQUIRE_INST_VK_PROC(DestroyInstance)
    ACQUIRE_INST_VK_PROC(DeviceWaitIdle)
    ACQUIRE_INST_VK_PROC(DestroyDevice)

    ACQUIRE_INST_VK_PROC(GetPhysicalDeviceFormatProperties)
    ACQUIRE_INST_VK_PROC(GetPhysicalDeviceMemoryProperties)

    ACQUIRE_DEVICE_VK_PROC(CreateImage)
    ACQUIRE_DEVICE_VK_PROC(DestroyImage)
    ACQUIRE_DEVICE_VK_PROC(GetImageMemoryRequirements)
    ACQUIRE_DEVICE_VK_PROC(AllocateMemory)
    ACQUIRE_DEVICE_VK_PROC(FreeMemory)
    ACQUIRE_DEVICE_VK_PROC(BindImageMemory)
    ACQUIRE_DEVICE_VK_PROC(MapMemory)
    ACQUIRE_DEVICE_VK_PROC(UnmapMemory)
    ACQUIRE_DEVICE_VK_PROC(FlushMappedMemoryRanges)
    ACQUIRE_DEVICE_VK_PROC(GetImageSubresourceLayout)

    fDirectContext = GrDirectContext::MakeVulkan(fBackendContext);
    if (!fDirectContext) {
        return false;
    }

    return true;
}

void VkTestHelper::cleanup() {
    // Make sure any work, release procs, etc left on the context are finished with before we start
    // tearing everything down.
    if (fDirectContext) {
        fDirectContext->flushAndSubmit(true);
    }

    fDirectContext.reset();

    fBackendContext.fMemoryAllocator.reset();
    if (fDevice != VK_NULL_HANDLE) {
        fVkDeviceWaitIdle(fDevice);
        fVkDestroyDevice(fDevice, nullptr);
        fDevice = VK_NULL_HANDLE;
    }
    if (fDebugCallback != VK_NULL_HANDLE) {
        fDestroyDebugCallback(fBackendContext.fInstance, fDebugCallback, nullptr);
    }

    if (fBackendContext.fInstance != VK_NULL_HANDLE) {
        fVkDestroyInstance(fBackendContext.fInstance, nullptr);
        fBackendContext.fInstance = VK_NULL_HANDLE;
    }

    sk_gpu_test::FreeVulkanFeaturesStructs(&fFeatures);
}

#endif // SK_VULKAN
