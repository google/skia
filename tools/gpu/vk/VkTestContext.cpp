/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/vk/VkTestContext.h"

#ifdef SK_VULKAN

#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/vk/GrVkDirectContext.h"
#include "include/gpu/vk/VulkanExtensions.h"
#include "tools/gpu/vk/VkTestUtils.h"

extern bool gCreateProtectedContext;

namespace {

class VkTestContextImpl : public sk_gpu_test::VkTestContext {
public:
    static VkTestContext* Create(VkTestContext* sharedContext) {
        GrVkBackendContext backendContext;
        skgpu::VulkanExtensions* extensions;
        VkPhysicalDeviceFeatures2* features;
        bool ownsContext = true;
        VkDebugReportCallbackEXT debugCallback = VK_NULL_HANDLE;
        PFN_vkDestroyDebugReportCallbackEXT destroyCallback = nullptr;
        if (sharedContext) {
            backendContext = sharedContext->getVkBackendContext();
            extensions = const_cast<skgpu::VulkanExtensions*>(sharedContext->getVkExtensions());
            features = const_cast<VkPhysicalDeviceFeatures2*>(sharedContext->getVkFeatures());
            // We always delete the parent context last so make sure the child does not think they
            // own the vulkan context.
            ownsContext = false;
        } else {
            PFN_vkGetInstanceProcAddr instProc;
            if (!sk_gpu_test::LoadVkLibraryAndGetProcAddrFuncs(&instProc)) {
                return nullptr;
            }

            extensions = new skgpu::VulkanExtensions();
            features = new VkPhysicalDeviceFeatures2;
            memset(features, 0, sizeof(VkPhysicalDeviceFeatures2));
            if (!sk_gpu_test::CreateVkBackendContext(instProc, &backendContext, extensions,
                                                     features, &debugCallback,
                                                     nullptr, sk_gpu_test::CanPresentFn(),
                                                     gCreateProtectedContext)) {
                sk_gpu_test::FreeVulkanFeaturesStructs(features);
                delete features;
                delete extensions;
                return nullptr;
            }
            if (debugCallback != VK_NULL_HANDLE) {
                destroyCallback = (PFN_vkDestroyDebugReportCallbackEXT) instProc(
                        backendContext.fInstance, "vkDestroyDebugReportCallbackEXT");
            }
        }
        return new VkTestContextImpl(backendContext, extensions, features, ownsContext,
                                     debugCallback, destroyCallback);
    }

    ~VkTestContextImpl() override { this->teardown(); }

    void testAbandon() override {}

    void finish() override {}

    sk_sp<GrDirectContext> makeContext(const GrContextOptions& options) override {
        return GrDirectContexts::MakeVulkan(fVk, options);
    }

protected:
#define ACQUIRE_VK_PROC_LOCAL(name, inst)                                            \
    PFN_vk##name grVk##name =                                                        \
            reinterpret_cast<PFN_vk##name>(fVk.fGetProc("vk" #name, inst, nullptr)); \
    do {                                                                             \
        if (grVk##name == nullptr) {                                                 \
            SkDebugf("Function ptr for vk%s could not be acquired\n", #name);        \
            return;                                                                  \
        }                                                                            \
    } while (0)

    void teardown() override {
        INHERITED::teardown();
        fVk.fMemoryAllocator.reset();
        if (fOwnsContext) {
            ACQUIRE_VK_PROC_LOCAL(DeviceWaitIdle, fVk.fInstance);
            ACQUIRE_VK_PROC_LOCAL(DestroyDevice, fVk.fInstance);
            ACQUIRE_VK_PROC_LOCAL(DestroyInstance, fVk.fInstance);
            grVkDeviceWaitIdle(fVk.fDevice);
            grVkDestroyDevice(fVk.fDevice, nullptr);
#ifdef SK_ENABLE_VK_LAYERS
            if (fDebugCallback != VK_NULL_HANDLE) {
                fDestroyDebugReportCallbackEXT(fVk.fInstance, fDebugCallback, nullptr);
            }
#endif
            grVkDestroyInstance(fVk.fInstance, nullptr);
            delete fExtensions;

            sk_gpu_test::FreeVulkanFeaturesStructs(fFeatures);
            delete fFeatures;
        }
    }

private:
    VkTestContextImpl(const GrVkBackendContext& backendContext,
                      const skgpu::VulkanExtensions* extensions,
                      VkPhysicalDeviceFeatures2* features,
                      bool ownsContext,
                      VkDebugReportCallbackEXT debugCallback,
                      PFN_vkDestroyDebugReportCallbackEXT destroyCallback)
            : VkTestContext(backendContext, extensions, features, ownsContext, debugCallback,
                            destroyCallback) {
        fFenceSupport = true;
    }

    void onPlatformMakeNotCurrent() const override {}
    void onPlatformMakeCurrent() const override {}
    std::function<void()> onPlatformGetAutoContextRestore() const override  { return nullptr; }

    using INHERITED = sk_gpu_test::VkTestContext;
};
}  // anonymous namespace

namespace sk_gpu_test {
VkTestContext* CreatePlatformVkTestContext(VkTestContext* sharedContext) {
    return VkTestContextImpl::Create(sharedContext);
}
}  // namespace sk_gpu_test

#endif
