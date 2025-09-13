/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/graphite/vk/GraphiteVulkanTestContext.h"

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/vk/VulkanGraphiteContext.h"
#include "include/gpu/vk/VulkanExtensions.h"
#include "include/gpu/vk/VulkanMemoryAllocator.h"
#include "src/gpu/graphite/ContextOptionsPriv.h"
#include "tools/gpu/ContextType.h"
#include "tools/gpu/vk/VkTestUtils.h"
#include "tools/graphite/TestOptions.h"

extern bool gCreateProtectedContext;

namespace skiatest::graphite {

std::unique_ptr<GraphiteTestContext> VulkanTestContext::Make() {
    skgpu::VulkanBackendContext backendContext;
    skgpu::VulkanExtensions* extensions;
    sk_gpu_test::TestVkFeatures* features;
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
    PFN_vkDestroyDebugUtilsMessengerEXT destroyCallback = nullptr;

    PFN_vkGetInstanceProcAddr instProc;
    if (!sk_gpu_test::LoadVkLibraryAndGetProcAddrFuncs(&instProc)) {
        return nullptr;
    }

    extensions = new skgpu::VulkanExtensions();
    features = new sk_gpu_test::TestVkFeatures;
    memset(&features->deviceFeatures, 0, sizeof(VkPhysicalDeviceFeatures2));
    if (!sk_gpu_test::CreateVkBackendContext(instProc,
                                             &backendContext,
                                             extensions,
                                             features,
                                             &debugMessenger,
                                             nullptr,
                                             sk_gpu_test::CanPresentFn(),
                                             gCreateProtectedContext)) {
        delete features;
        delete extensions;
        return nullptr;
    }
    if (debugMessenger != VK_NULL_HANDLE) {
        destroyCallback = (PFN_vkDestroyDebugUtilsMessengerEXT)instProc(
                backendContext.fInstance, "vkDestroyDebugUtilsMessengerEXT");
    }

    return std::unique_ptr<GraphiteTestContext>(new VulkanTestContext(
            backendContext, extensions, features, debugMessenger, destroyCallback));
}

#define ACQUIRE_VK_PROC_LOCAL(name, inst)                                                \
    PFN_vk##name localVk##name =                                                         \
            reinterpret_cast<PFN_vk##name>(fVulkan.fGetProc("vk" #name, inst, nullptr)); \
    do {                                                                                 \
        if (localVk##name == nullptr) {                                                  \
            SkDebugf("Function ptr for vk%s could not be acquired\n", #name);            \
            return;                                                                      \
        }                                                                                \
    } while (0)

VulkanTestContext::~VulkanTestContext() {
    fVulkan.fMemoryAllocator.reset();
    ACQUIRE_VK_PROC_LOCAL(DeviceWaitIdle, fVulkan.fInstance);
    ACQUIRE_VK_PROC_LOCAL(DestroyDevice, fVulkan.fInstance);
    ACQUIRE_VK_PROC_LOCAL(DestroyInstance, fVulkan.fInstance);
    localVkDeviceWaitIdle(fVulkan.fDevice);
    localVkDestroyDevice(fVulkan.fDevice, nullptr);
#ifdef SK_ENABLE_VK_LAYERS
    if (fDebugMessenger != VK_NULL_HANDLE) {
        fDestroyDebugUtilsMessengerEXT(fVulkan.fInstance, fDebugMessenger, nullptr);
    }
#else
    // Surpress unused private member variable warning
    (void)fDebugMessenger;
    (void)fDestroyDebugUtilsMessengerEXT;
#endif
    localVkDestroyInstance(fVulkan.fInstance, nullptr);
    delete fExtensions;
    delete fFeatures;
}

skgpu::ContextType VulkanTestContext::contextType() {
    return skgpu::ContextType::kVulkan;
}

std::unique_ptr<skgpu::graphite::Context> VulkanTestContext::makeContext(
        const TestOptions& options) {
    SkASSERT(!options.hasDawnOptions());
    skgpu::graphite::ContextOptions revisedContextOptions(options.fContextOptions);
    skgpu::graphite::ContextOptionsPriv contextOptionsPriv;
    if (!options.fContextOptions.fOptionsPriv) {
        revisedContextOptions.fOptionsPriv = &contextOptionsPriv;
    }
    // Needed to make synchronous readPixels work
    revisedContextOptions.fOptionsPriv->fStoreContextRefInRecorder = true;
    SkASSERT(fVulkan.fMemoryAllocator);
    return skgpu::graphite::ContextFactory::MakeVulkan(fVulkan, revisedContextOptions);
}

}  // namespace skiatest::graphite
