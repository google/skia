/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/vk/VkTestContext.h"

#ifdef SK_VULKAN

#include "include/gpu/GrContext.h"
#include "include/gpu/vk/GrVkExtensions.h"
#include "tools/gpu/vk/VkTestUtils.h"

namespace {

#define ACQUIRE_VK_PROC(name, device)                                               \
    f##name = reinterpret_cast<PFN_vk##name>(getProc("vk" #name, nullptr, device)); \
    SkASSERT(f##name)

class VkTestContextImpl : public sk_gpu_test::VkTestContext {
public:
    static VkTestContext* Create(VkTestContext* sharedContext) {
        GrVkBackendContext backendContext;
        GrVkExtensions* extensions;
        VkPhysicalDeviceFeatures2* features;
        bool ownsContext = true;
        VkDebugReportCallbackEXT debugCallback = VK_NULL_HANDLE;
        PFN_vkDestroyDebugReportCallbackEXT destroyCallback = nullptr;
        if (sharedContext) {
            backendContext = sharedContext->getVkBackendContext();
            extensions = const_cast<GrVkExtensions*>(sharedContext->getVkExtensions());
            features = const_cast<VkPhysicalDeviceFeatures2*>(sharedContext->getVkFeatures());
            // We always delete the parent context last so make sure the child does not think they
            // own the vulkan context.
            ownsContext = false;
        } else {
            PFN_vkGetInstanceProcAddr instProc;
            PFN_vkGetDeviceProcAddr devProc;
            if (!sk_gpu_test::LoadVkLibraryAndGetProcAddrFuncs(&instProc, &devProc)) {
                return nullptr;
            }
            auto getProc = [instProc, devProc](const char* proc_name,
                                               VkInstance instance, VkDevice device) {
                if (device != VK_NULL_HANDLE) {
                    return devProc(device, proc_name);
                }
                return instProc(instance, proc_name);
            };
            extensions = new GrVkExtensions();
            features = new VkPhysicalDeviceFeatures2;
            memset(features, 0, sizeof(VkPhysicalDeviceFeatures2));
            if (!sk_gpu_test::CreateVkBackendContext(getProc, &backendContext, extensions,
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
        }
        return new VkTestContextImpl(backendContext, extensions, features, ownsContext,
                                     debugCallback, destroyCallback);
    }

    ~VkTestContextImpl() override { this->teardown(); }

    void testAbandon() override {}

    void finish() override {}

    sk_sp<GrContext> makeGrContext(const GrContextOptions& options) override {
        return GrContext::MakeVulkan(fVk, options);
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
    VkTestContextImpl(const GrVkBackendContext& backendContext, const GrVkExtensions* extensions,
                      VkPhysicalDeviceFeatures2* features, bool ownsContext,
                      VkDebugReportCallbackEXT debugCallback,
                      PFN_vkDestroyDebugReportCallbackEXT destroyCallback)
            : VkTestContext(backendContext, extensions, features, ownsContext, debugCallback,
                            destroyCallback) {
        fFenceSupport = true;
    }

    void onPlatformMakeNotCurrent() const override {}
    void onPlatformMakeCurrent() const override {}
    std::function<void()> onPlatformGetAutoContextRestore() const override  { return nullptr; }

    typedef sk_gpu_test::VkTestContext INHERITED;
};
}  // anonymous namespace

namespace sk_gpu_test {
VkTestContext* CreatePlatformVkTestContext(VkTestContext* sharedContext) {
    return VkTestContextImpl::Create(sharedContext);
}
}  // namespace sk_gpu_test

#endif
