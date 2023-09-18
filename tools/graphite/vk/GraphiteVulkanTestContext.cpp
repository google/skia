/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/graphite/vk/GraphiteVulkanTestContext.h"

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/vk/VulkanGraphiteUtils.h"
#include "include/gpu/vk/VulkanExtensions.h"
#include "include/private/gpu/graphite/ContextOptionsPriv.h"
#include "tools/gpu/ContextType.h"
#include "tools/gpu/vk/VkTestUtils.h"

namespace skiatest::graphite {

std::unique_ptr<GraphiteTestContext> VulkanTestContext::Make() {
    skgpu::VulkanBackendContext backendContext;
    skgpu::VulkanExtensions* extensions;
    VkPhysicalDeviceFeatures2* features;
    VkDebugReportCallbackEXT debugCallback = VK_NULL_HANDLE;
    PFN_vkDestroyDebugReportCallbackEXT destroyCallback = nullptr;

    PFN_vkGetInstanceProcAddr instProc;
    if (!sk_gpu_test::LoadVkLibraryAndGetProcAddrFuncs(&instProc)) {
        return nullptr;
    }

    extensions = new skgpu::VulkanExtensions();
    features = new VkPhysicalDeviceFeatures2;
    memset(features, 0, sizeof(VkPhysicalDeviceFeatures2));
    if (!sk_gpu_test::CreateVkBackendContext(instProc, &backendContext, extensions,
                                             features, &debugCallback)) {
        sk_gpu_test::FreeVulkanFeaturesStructs(features);
        delete features;
        delete extensions;
        return nullptr;
    }
    if (debugCallback != VK_NULL_HANDLE) {
        destroyCallback = (PFN_vkDestroyDebugReportCallbackEXT) instProc(
                backendContext.fInstance, "vkDestroyDebugReportCallbackEXT");
    }

    return std::unique_ptr<GraphiteTestContext>(new VulkanTestContext(backendContext,
                                                                      extensions,
                                                                      features,
                                                                      debugCallback,
                                                                      destroyCallback));
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
    if (fDebugCallback != VK_NULL_HANDLE) {
        fDestroyDebugReportCallbackEXT(fVulkan.fInstance, fDebugCallback, nullptr);
    }
#else
    // Surpress unused private member variable warning
    (void)fDebugCallback;
    (void)fDestroyDebugReportCallbackEXT;
#endif
    localVkDestroyInstance(fVulkan.fInstance, nullptr);
    delete fExtensions;

    sk_gpu_test::FreeVulkanFeaturesStructs(fFeatures);
    delete fFeatures;
}

skgpu::ContextType VulkanTestContext::contextType() {
    return skgpu::ContextType::kVulkan;
}

std::unique_ptr<skgpu::graphite::Context> VulkanTestContext::makeContext(
        const skgpu::graphite::ContextOptions& options) {
    skgpu::graphite::ContextOptions revisedOptions(options);
    skgpu::graphite::ContextOptionsPriv optionsPriv;
    if (!options.fOptionsPriv) {
        revisedOptions.fOptionsPriv = &optionsPriv;
    }
    // Needed to make synchronous readPixels work
    revisedOptions.fOptionsPriv->fStoreContextRefInRecorder = true;

    return skgpu::graphite::ContextFactory::MakeVulkan(fVulkan, revisedOptions);
}

}  // namespace skiatest::graphite
