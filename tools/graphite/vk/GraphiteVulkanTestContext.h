/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skiatest_graphite_VulkanTestContext_DEFINED
#define skiatest_graphite_VulkanTestContext_DEFINED

#include "tools/graphite/GraphiteTestContext.h"

#include "include/gpu/vk/VulkanBackendContext.h"

namespace skiatest::graphite {

class VulkanTestContext : public GraphiteTestContext {
public:
    ~VulkanTestContext() override;

    static std::unique_ptr<GraphiteTestContext> Make();

    skgpu::BackendApi backend() override { return skgpu::BackendApi::kVulkan; }

    skgpu::ContextType contextType() override;

    std::unique_ptr<skgpu::graphite::Context> makeContext(const TestOptions&) override;

    const skgpu::VulkanBackendContext& getBackendContext() const {
        return fVulkan;
    }

private:
    VulkanTestContext(const skgpu::VulkanBackendContext& vulkan,
                      const skgpu::VulkanExtensions* extensions,
                      VkPhysicalDeviceFeatures2* features,
                      VkDebugUtilsMessengerEXT debugMessenger,
                      PFN_vkDestroyDebugUtilsMessengerEXT destroyCallback)
            : fVulkan(vulkan)
            , fExtensions(extensions)
            , fFeatures(features)
            , fDebugMessenger(debugMessenger)
            , fDestroyDebugUtilsMessengerEXT(destroyCallback) {}

    skgpu::VulkanBackendContext fVulkan;
    const skgpu::VulkanExtensions* fExtensions;
    const VkPhysicalDeviceFeatures2* fFeatures;
    VkDebugUtilsMessengerEXT fDebugMessenger = VK_NULL_HANDLE;
    PFN_vkDestroyDebugUtilsMessengerEXT fDestroyDebugUtilsMessengerEXT = nullptr;
};

}  // namespace skiatest::graphite

#endif // skiatest_graphite_VulkanTestContext_DEFINED
