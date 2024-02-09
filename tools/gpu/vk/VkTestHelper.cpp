/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/vk/VkTestHelper.h"

#if defined(SK_VULKAN)

#include "include/core/SkSurface.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/vk/GrVkBackendContext.h"
#include "tests/TestType.h"
#include "tools/gpu/ProtectedUtils.h"
#include "tools/gpu/vk/VkTestUtils.h"

#if defined(SK_GANESH)
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/vk/GrVkDirectContext.h"
#endif

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/vk/VulkanGraphiteUtils.h"
#include "include/private/gpu/graphite/ContextOptionsPriv.h"
#endif

#define ACQUIRE_INST_VK_PROC(name)                                                               \
    fVk##name = reinterpret_cast<PFN_vk##name>(instProc(fBackendContext.fInstance, "vk" #name)); \
    if (fVk##name == nullptr) {                                                                  \
        SkDebugf("Function ptr for vk%s could not be acquired\n", #name);                        \
        return false;                                                                            \
    }

#define ACQUIRE_DEVICE_VK_PROC(name)                                                          \
    fVk##name = reinterpret_cast<PFN_vk##name>(fVkGetDeviceProcAddr(fDevice, "vk" #name));    \
    if (fVk##name == nullptr) {                                                               \
        SkDebugf("Function ptr for vk%s could not be acquired\n", #name);                     \
        return false;                                                                         \
    }

#if defined(SK_GANESH)

class GaneshVkTestHelper : public VkTestHelper {
public:
    GaneshVkTestHelper(bool isProtected) : VkTestHelper(isProtected) {}

    ~GaneshVkTestHelper() override {
        // Make sure any work, release procs, etc left on the context are finished with before we
        // start tearing everything down.
        if (fDirectContext) {
            fDirectContext->flushAndSubmit(GrSyncCpu::kYes);
        }

        fDirectContext.reset();
    }

    bool isValid() const override { return fDirectContext != nullptr; }

    sk_sp<SkSurface> createSurface(SkISize size, bool textureable, bool isProtected) override {
        // Make Ganesh use DMSAA to better match Graphite's behavior
        SkSurfaceProps props(SkSurfaceProps::kDynamicMSAA_Flag, kUnknown_SkPixelGeometry);

        return ProtectedUtils::CreateProtectedSkSurface(fDirectContext.get(), size,
                                                        textureable, isProtected,
                                                        &props);
    }

    void submitAndWaitForCompletion(bool* completionMarker) override {
        fDirectContext->submit();
        while (!*completionMarker) {
            fDirectContext->checkAsyncWorkCompletion();
        }
    }

    GrDirectContext* directContext() override { return fDirectContext.get(); }

protected:
    bool init() override {
        if (!this->setupBackendContext()) {
            return false;
        }

        GrVkBackendContext gr;
        sk_gpu_test::ConvertBackendContext(fBackendContext, &gr);
        fDirectContext = GrDirectContexts::MakeVulkan(gr);
        if (!fDirectContext) {
            return false;
        }

        SkASSERT(fDirectContext->supportsProtectedContent() == fIsProtected);
        return true;
    }

private:
    sk_sp<GrDirectContext> fDirectContext;
};

#endif // SK_GANESH

#if defined(SK_GRAPHITE)

class GraphiteVkTestHelper : public VkTestHelper {
public:
    GraphiteVkTestHelper(bool isProtected) : VkTestHelper(isProtected) {}

    ~GraphiteVkTestHelper() override {
        // Make sure any work, release procs, etc left on the context are finished with before we
        // start tearing everything down.

        std::unique_ptr<skgpu::graphite::Recording> recording;
        if (fRecorder) {
            recording = fRecorder->snap();
        }

        if (fContext) {
            fContext->insertRecording({ recording.get() });
            fContext->submit(skgpu::graphite::SyncToCpu::kYes);
        }

        recording.reset();
        fRecorder.reset();
        fContext.reset();
    }

    bool isValid() const override { return fContext != nullptr && fRecorder != nullptr; }

    sk_sp<SkSurface> createSurface(SkISize size,
                                   bool /* textureable */,
                                   bool isProtected) override {
        return ProtectedUtils::CreateProtectedSkSurface(fRecorder.get(), size,
                                                        skgpu::Protected(isProtected));
    }

    void submitAndWaitForCompletion(bool* completionMarker) override {
        fContext->submit();
        while (!*completionMarker) {
            fContext->checkAsyncWorkCompletion();
        }
    }

    skgpu::graphite::Recorder* recorder() override { return fRecorder.get(); }

protected:
    bool init() override {
        if (!this->setupBackendContext()) {
            return false;
        }

        skgpu::graphite::ContextOptions contextOptions;
        skgpu::graphite::ContextOptionsPriv contextOptionsPriv;
        // Needed to make ManagedGraphiteTexture::ReleaseProc (w/in CreateProtectedSkSurface) work
        contextOptionsPriv.fStoreContextRefInRecorder = true;
        contextOptions.fOptionsPriv = &contextOptionsPriv;

        fContext = skgpu::graphite::ContextFactory::MakeVulkan(fBackendContext, contextOptions);
        if (!fContext) {
            return false;
        }

        SkASSERT(fContext->supportsProtectedContent() == fIsProtected);

        fRecorder = fContext->makeRecorder();
        if (!fRecorder) {
            return false;
        }

        return true;
    }

private:
    std::unique_ptr<skgpu::graphite::Context> fContext;
    std::unique_ptr<skgpu::graphite::Recorder> fRecorder;
};

#endif // SK_GRAPHITE

std::unique_ptr<VkTestHelper> VkTestHelper::Make(skiatest::TestType testType,
                                                 bool isProtected) {
    std::unique_ptr<VkTestHelper> helper;

    switch (testType) {
#if defined(SK_GANESH)
        case skiatest::TestType::kGanesh:
            helper = std::make_unique<GaneshVkTestHelper>(isProtected);
            break;
#endif
#if defined(SK_GRAPHITE)
        case skiatest::TestType::kGraphite:
            helper = std::make_unique<GraphiteVkTestHelper>(isProtected);
            break;
#endif
        default:
            return nullptr;
    }
    if (!helper->init()) {
        return nullptr;
    }

    return helper;
}

bool VkTestHelper::setupBackendContext() {
    PFN_vkGetInstanceProcAddr instProc;
    if (!sk_gpu_test::LoadVkLibraryAndGetProcAddrFuncs(&instProc)) {
        return false;
    }

    fFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    fFeatures.pNext = nullptr;

    fBackendContext.fInstance = VK_NULL_HANDLE;
    fBackendContext.fDevice = VK_NULL_HANDLE;

    if (!sk_gpu_test::CreateVkBackendContext(instProc, &fBackendContext, &fExtensions,
                                             &fFeatures, &fDebugCallback, nullptr,
                                             sk_gpu_test::CanPresentFn(), fIsProtected)) {
        return false;
    }
    fDevice = fBackendContext.fDevice;

    if (fDebugCallback != VK_NULL_HANDLE) {
        fDestroyDebugCallback = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
                instProc(fBackendContext.fInstance, "vkDestroyDebugReportCallbackEXT"));
    }
    ACQUIRE_INST_VK_PROC(DestroyInstance)
    ACQUIRE_INST_VK_PROC(DeviceWaitIdle)
    ACQUIRE_INST_VK_PROC(DestroyDevice)

    ACQUIRE_INST_VK_PROC(GetPhysicalDeviceFormatProperties)
    ACQUIRE_INST_VK_PROC(GetPhysicalDeviceMemoryProperties)

    ACQUIRE_INST_VK_PROC(GetDeviceProcAddr)

    ACQUIRE_DEVICE_VK_PROC(CreateImage)
    ACQUIRE_DEVICE_VK_PROC(DestroyImage)
    ACQUIRE_DEVICE_VK_PROC(GetImageMemoryRequirements)
    ACQUIRE_DEVICE_VK_PROC(AllocateMemory)
    ACQUIRE_DEVICE_VK_PROC(FreeMemory)
    ACQUIRE_DEVICE_VK_PROC(BindImageMemory)
    ACQUIRE_DEVICE_VK_PROC(MapMemory)
    ACQUIRE_DEVICE_VK_PROC(UnmapMemory)
    ACQUIRE_DEVICE_VK_PROC(FlushMappedMemoryRanges)
    ACQUIRE_DEVICE_VK_PROC(GetImageSubresourceLayout)
    return true;
}

VkTestHelper::~VkTestHelper() {
    fBackendContext.fMemoryAllocator.reset();
    if (fDevice != VK_NULL_HANDLE) {
        fVkDeviceWaitIdle(fDevice);
        fVkDestroyDevice(fDevice, nullptr);
        fDevice = VK_NULL_HANDLE;
    }
    if (fDebugCallback != VK_NULL_HANDLE) {
        fDestroyDebugCallback(fBackendContext.fInstance, fDebugCallback, nullptr);
    }

    if (fBackendContext.fInstance != VK_NULL_HANDLE) {
        fVkDestroyInstance(fBackendContext.fInstance, nullptr);
        fBackendContext.fInstance = VK_NULL_HANDLE;
    }

    sk_gpu_test::FreeVulkanFeaturesStructs(&fFeatures);
}

#endif // SK_VULKAN
