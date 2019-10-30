/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef VkTestContext_DEFINED
#define VkTestContext_DEFINED

#include "tools/gpu/TestContext.h"

#ifdef SK_VULKAN

#include "include/gpu/vk/GrVkBackendContext.h"
#include "tools/gpu/vk/GrVulkanDefines.h"

class GrVkExtensions;

namespace sk_gpu_test {
class VkTestContext : public TestContext {
public:
    virtual GrBackendApi backend() override { return GrBackendApi::kVulkan; }

    const GrVkBackendContext& getVkBackendContext() const {
        return fVk;
    }

    const GrVkExtensions* getVkExtensions() const {
        return fExtensions;
    }

    const VkPhysicalDeviceFeatures2* getVkFeatures() const {
        return fFeatures;
    }

protected:
    VkTestContext(const GrVkBackendContext& vk, const GrVkExtensions* extensions,
                  const VkPhysicalDeviceFeatures2* features, bool ownsContext,
                  VkDebugReportCallbackEXT debugCallback,
                  PFN_vkDestroyDebugReportCallbackEXT destroyCallback)
            : fVk(vk)
            , fExtensions(extensions)
            , fFeatures(features)
            , fOwnsContext(ownsContext)
            , fDebugCallback(debugCallback)
            , fDestroyDebugReportCallbackEXT(destroyCallback) {}

    GrVkBackendContext                  fVk;
    const GrVkExtensions*               fExtensions;
    const VkPhysicalDeviceFeatures2*    fFeatures;
    bool                                fOwnsContext;
    VkDebugReportCallbackEXT            fDebugCallback = VK_NULL_HANDLE;
    PFN_vkDestroyDebugReportCallbackEXT fDestroyDebugReportCallbackEXT = nullptr;

private:
    typedef TestContext INHERITED;
};

/**
 * Creates Vk context object bound to the native Vk library.
 */
VkTestContext* CreatePlatformVkTestContext(VkTestContext*);

}  // namespace sk_gpu_test

#endif

#endif
