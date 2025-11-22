/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/ganesh/vk/VkTestContext.h"

#ifdef SK_VULKAN

#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/vk/GrVkDirectContext.h"
#include "include/gpu/vk/VulkanBackendContext.h"
#include "include/gpu/vk/VulkanExtensions.h"
#include "include/gpu/vk/VulkanMemoryAllocator.h"
#include "tools/gpu/vk/VkTestUtils.h"

// gCreateProtectedContext lives in GrContextFactory.cpp and so isn't defined if !SK_GANESH
#if defined(SK_GANESH)
extern bool gCreateProtectedContext;
static const bool& createProtectedContext = gCreateProtectedContext;
#else
static constexpr bool createProtectedContext = false;
#endif

namespace {

class VkTestContextImpl : public sk_gpu_test::VkTestContext {
public:
    static VkTestContext* Create(VkTestContext* sharedContext) {
        skgpu::VulkanBackendContext backendContext;
        const skgpu::VulkanExtensions* extensions;
        const sk_gpu_test::TestVkFeatures* features;
        bool ownsContext = true;
        VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
        PFN_vkDestroyDebugUtilsMessengerEXT destroyCallback = nullptr;
        if (sharedContext) {
            backendContext = sharedContext->getVkBackendContext();
            extensions = sharedContext->getVkExtensions();
            features = sharedContext->getVkFeatures();
            // We always delete the parent context last so make sure the child does not think they
            // own the vulkan context.
            ownsContext = false;
        } else {
            PFN_vkGetInstanceProcAddr instProc;
            if (!sk_gpu_test::LoadVkLibraryAndGetProcAddrFuncs(&instProc)) {
                return nullptr;
            }

            skgpu::VulkanExtensions* ownedExtensions = new skgpu::VulkanExtensions();
            sk_gpu_test::TestVkFeatures* ownedFeatures = new sk_gpu_test::TestVkFeatures;
            extensions = ownedExtensions;
            features = ownedFeatures;
            if (!sk_gpu_test::CreateVkBackendContext(instProc,
                                                     &backendContext,
                                                     ownedExtensions,
                                                     ownedFeatures,
                                                     &debugMessenger,
                                                     nullptr,
                                                     sk_gpu_test::CanPresentFn(),
                                                     createProtectedContext)) {
                delete ownedExtensions;
                delete ownedFeatures;
                return nullptr;
            }
            if (debugMessenger != VK_NULL_HANDLE) {
                destroyCallback = (PFN_vkDestroyDebugUtilsMessengerEXT)instProc(
                        backendContext.fInstance, "vkDestroyDebugUtilsMessengerEXT");
            }
        }
        return new VkTestContextImpl(
                backendContext, extensions, features, ownsContext, debugMessenger, destroyCallback);
    }

    ~VkTestContextImpl() override { this->teardown(); }

    void testAbandon() override {}

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
            if (fDebugMessenger != VK_NULL_HANDLE) {
                fDestroyDebugUtilsMessengerEXT(fVk.fInstance, fDebugMessenger, nullptr);
            }
#endif
            grVkDestroyInstance(fVk.fInstance, nullptr);

            delete fExtensions;
            delete fFeatures;
        }
    }

private:
    VkTestContextImpl(const skgpu::VulkanBackendContext& backendContext,
                      const skgpu::VulkanExtensions* extensions,
                      const sk_gpu_test::TestVkFeatures* features,
                      bool ownsContext,
                      VkDebugUtilsMessengerEXT debugMessenger,
                      PFN_vkDestroyDebugUtilsMessengerEXT destroyCallback)
            : VkTestContext(backendContext,
                            extensions,
                            features,
                            ownsContext,
                            debugMessenger,
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
