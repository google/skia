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
    VkTestContext(const GrVkBackendContext& vk, bool ownsContext,
                  VkDebugReportCallbackEXT debugCallback,
                      PFN_vkDestroyDebugReportCallbackEXT destroyCallback)
            : fVk(vk)
            , fOwnsContext(ownsContext)
            , fDebugCallback(debugCallback) {
        fDestroyDebugReportCallbackEXT = destroyCallback;
    }

    GrVkBackendContext fVk;
    bool fOwnsContext;
    VkDebugReportCallbackEXT fDebugCallback = VK_NULL_HANDLE;

    // simple wrapper class that exists only to initialize a pointer to NULL
    template <typename FNPTR_TYPE> class VkPtr {
    public:
        VkPtr() : fPtr(NULL) {}
        VkPtr operator=(FNPTR_TYPE ptr) { fPtr = ptr; return *this; }
        operator FNPTR_TYPE() const { return fPtr; }
    private:
        FNPTR_TYPE fPtr;
    };
    VkPtr<PFN_vkDestroyDebugReportCallbackEXT> fDestroyDebugReportCallbackEXT;

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
