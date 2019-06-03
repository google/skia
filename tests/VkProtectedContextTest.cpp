/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a Vulkan protected memory specific test.

#include "include/core/SkTypes.h"

#if SK_SUPPORT_GPU && defined(SK_VULKAN)

#include "include/gpu/vk/GrVkBackendContext.h"
#include "include/gpu/vk/GrVkExtensions.h"
#include "tests/Test.h"
#include "tools/gpu/GrContextFactory.h"
#include "tools/gpu/vk/VkTestUtils.h"

namespace {

#define DECLARE_VK_PROC(name) PFN_vk##name fVk##name

#define ACQUIRE_INST_VK_PROC(name)                                                           \
    fVk##name = reinterpret_cast<PFN_vk##name>(getProc("vk" #name, fBackendContext.fInstance,\
                                                       VK_NULL_HANDLE));                     \
    if (fVk##name == nullptr) {                                                              \
        ERRORF(reporter, "Function ptr for vk%s could not be acquired\n", #name);            \
        return false;                                                                        \
    }

#define ACQUIRE_DEVICE_VK_PROC(name)                                                          \
    fVk##name = reinterpret_cast<PFN_vk##name>(getProc("vk" #name, VK_NULL_HANDLE, fDevice)); \
    if (fVk##name == nullptr) {                                                               \
        ERRORF(reporter, "Function ptr for vk%s could not be acquired\n", #name);             \
        return false;                                                                         \
    }

class VulkanTestHelper {
public:
    VulkanTestHelper(bool isProtected) : fIsProtected(isProtected) {}

    ~VulkanTestHelper() {
        cleanup();
    }

    bool init(skiatest::Reporter* reporter);

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

} // namespace

bool VulkanTestHelper::init(skiatest::Reporter* reporter) {
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

    fExtensions = new GrVkExtensions();
    fFeatures = new VkPhysicalDeviceFeatures2;
    memset(fFeatures, 0, sizeof(VkPhysicalDeviceFeatures2));
    fFeatures->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    fFeatures->pNext = nullptr;

    fBackendContext.fInstance = VK_NULL_HANDLE;
    fBackendContext.fDevice = VK_NULL_HANDLE;

    if (!sk_gpu_test::CreateVkBackendContext(getProc, &fBackendContext, fExtensions,
                                             fFeatures, &fDebugCallback, nullptr,
                                             sk_gpu_test::CanPresentFn(), fIsProtected)) {
        return false;
    }
    fDevice = fBackendContext.fDevice;

    if (fDebugCallback != VK_NULL_HANDLE) {
        fDestroyDebugCallback = (PFN_vkDestroyDebugReportCallbackEXT) instProc(
                fBackendContext.fInstance, "vkDestroyDebugReportCallbackEXT");
    }
    ACQUIRE_INST_VK_PROC(DestroyInstance)
    ACQUIRE_INST_VK_PROC(DeviceWaitIdle)
    ACQUIRE_INST_VK_PROC(DestroyDevice)

    fGrContext = GrContext::MakeVulkan(fBackendContext);
    REPORTER_ASSERT(reporter, fGrContext.get());
    if (!fGrContext) {
        return false;
    }

    return true;
}

void VulkanTestHelper::cleanup() {
    fGrContext.reset();

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

    delete fExtensions;

    sk_gpu_test::FreeVulkanFeaturesStructs(fFeatures);
    delete fFeatures;
}

DEF_GPUTEST(VkProtectedContext_CreateNonprotectedContext, reporter, options) {
    auto nonprotectedTestHelper = std::make_unique<VulkanTestHelper>(false);
    REPORTER_ASSERT(reporter, nonprotectedTestHelper->init(reporter));
}


DEF_GPUTEST(VkProtectedContext_CreateProtectedContext, reporter, options) {
    auto protectedTestHelper = std::make_unique<VulkanTestHelper>(true);
    if (!protectedTestHelper->init(reporter)) {
        SkDebugf("Cannot create protected vk context\n");
        return;
    }
}

DEF_GPUTEST(VkProtectedContext_CreateSkSurface, reporter, options) {
    auto protectedTestHelper = std::make_unique<VulkanTestHelper>(true);
    if (!protectedTestHelper->init(reporter)) {
        SkDebugf("Cannot create protected vk context\n");
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->grContext() != nullptr);

    const int kW = 8;
    const int kH = 8;
    GrBackendTexture backendTex =
        protectedTestHelper->grContext()->createBackendTexture(
            kW, kH, kRGBA_8888_SkColorType, GrMipMapped::kNo, GrRenderable::kNo);
    REPORTER_ASSERT(reporter, backendTex.isValid());
}

#endif  // SK_SUPPORT_GPU && defined(SK_VULKAN)
