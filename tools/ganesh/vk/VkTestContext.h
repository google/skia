/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef VkTestContext_DEFINED
#define VkTestContext_DEFINED

#include "tools/ganesh/TestContext.h"

#ifdef SK_VULKAN

#include "include/gpu/vk/VulkanBackendContext.h"
#include "tools/gpu/vk/VkTestUtils.h"
#include "tools/gpu/vk/VulkanDefines.h"

namespace skgpu { class VulkanExtensions; }

namespace sk_gpu_test {
class VkTestContext : public TestContext {
public:
    GrBackendApi backend() override { return GrBackendApi::kVulkan; }

    const skgpu::VulkanBackendContext& getVkBackendContext() const { return fVk; }

    const skgpu::VulkanExtensions* getVkExtensions() const {
        return fExtensions;
    }

    const sk_gpu_test::TestVkFeatures* getVkFeatures() const { return fFeatures; }

protected:
    VkTestContext(const skgpu::VulkanBackendContext& vk,
                  const skgpu::VulkanExtensions* extensions,
                  const sk_gpu_test::TestVkFeatures* features,
                  bool ownsContext,
                  VkDebugUtilsMessengerEXT debugMessenger,
                  PFN_vkDestroyDebugUtilsMessengerEXT destroyCallback)
            : fVk(vk)
            , fExtensions(extensions)
            , fFeatures(features)
            , fOwnsContext(ownsContext)
            , fDebugMessenger(debugMessenger)
            , fDestroyDebugUtilsMessengerEXT(destroyCallback) {}

    skgpu::VulkanBackendContext         fVk;
    const skgpu::VulkanExtensions*      fExtensions;
    const sk_gpu_test::TestVkFeatures*  fFeatures;
    bool                                fOwnsContext;
    VkDebugUtilsMessengerEXT fDebugMessenger = VK_NULL_HANDLE;
    PFN_vkDestroyDebugUtilsMessengerEXT fDestroyDebugUtilsMessengerEXT = nullptr;

private:
    using INHERITED = TestContext;
};

/**
 * Creates Vk context object bound to the native Vk library.
 */
VkTestContext* CreatePlatformVkTestContext(VkTestContext*);

}  // namespace sk_gpu_test

#endif

#endif
