/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef VkTestContext_DEFINED
#define VkTestContext_DEFINED

#include "TestContext.h"

#ifdef SK_VULKAN

#include "vk/GrVkBackendContext.h"

namespace sk_gpu_test {
class VkTestContext : public TestContext {
public:
    virtual GrBackend backend() override { return kVulkan_GrBackend; }

    const GrVkBackendContext& getVkBackendContext() {
        return fVk;
    }

    const GrVkInterface* vk() const { return fVk.fInterface.get(); }

protected:
    VkTestContext(const GrVkBackendContext& vk, bool ownsContext) : fOwnsContext(ownsContext) {
        fVk.fInstance = vk.fInstance;
        fVk.fPhysicalDevice = vk.fPhysicalDevice;
        fVk.fDevice = vk.fDevice;
        fVk.fQueue = vk.fQueue;
        fVk.fGraphicsQueueIndex = vk.fGraphicsQueueIndex;
        fVk.fMinAPIVersion = vk.fMinAPIVersion;
        fVk.fExtensions = vk.fExtensions;
        fVk.fFeatures = vk.fFeatures;
        fVk.fInterface = vk.fInterface;
        fVk.fMemoryAllocator = vk.fMemoryAllocator;
        fVk.fOwnsInstanceAndDevice = vk.fOwnsInstanceAndDevice;
    }

    GrVkBackendContext fVk;
    bool fOwnsContext;

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
