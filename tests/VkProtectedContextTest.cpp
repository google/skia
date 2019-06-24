/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a Vulkan protected memory specific test.

#include "include/core/SkTypes.h"

#if SK_SUPPORT_GPU && defined(SK_VULKAN)

#include "include/core/SkCanvas.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/vk/GrVkBackendContext.h"
#include "include/gpu/vk/GrVkExtensions.h"
#include "tests/Test.h"
#include "tools/gpu/GrContextFactory.h"
#include "tools/gpu/vk/VkTestUtils.h"

namespace {

#define DECLARE_VK_PROC(name) PFN_vk##name fVk##name

#define ACQUIRE_INST_VK_PROC(name)                                                           \
    fVk##name = reinterpret_cast<PFN_vk##name>(getProc("vk" #name, fBackendContext.fInstance,\
                                                       VK_NULL_HANDLE));                     \
    if (fVk##name == nullptr) {                                                              \
        ERRORF(reporter, "Function ptr for vk%s could not be acquired\n", #name);            \
        return false;                                                                        \
    }

#define ACQUIRE_DEVICE_VK_PROC(name)                                                          \
    fVk##name = reinterpret_cast<PFN_vk##name>(getProc("vk" #name, VK_NULL_HANDLE, fDevice)); \
    if (fVk##name == nullptr) {                                                               \
        ERRORF(reporter, "Function ptr for vk%s could not be acquired\n", #name);             \
        return false;                                                                         \
    }

class VulkanTestHelper {
public:
    VulkanTestHelper(bool isProtected) : fIsProtected(isProtected) {}

    ~VulkanTestHelper() {
        cleanup();
    }

    bool init(skiatest::Reporter* reporter);

    GrContext* grContext() { return fGrContext.get(); }

    sk_sp<SkSurface> createSkSurface(skiatest::Reporter* reporter);

   private:
    void cleanup();

    DECLARE_VK_PROC(DestroyInstance);
    DECLARE_VK_PROC(DeviceWaitIdle);
    DECLARE_VK_PROC(DestroyDevice);

    bool fIsProtected = false;
    VkDevice fDevice = VK_NULL_HANDLE;

    GrVkExtensions* fExtensions = nullptr;
    VkPhysicalDeviceFeatures2* fFeatures = nullptr;
    VkDebugReportCallbackEXT fDebugCallback = VK_NULL_HANDLE;
    PFN_vkDestroyDebugReportCallbackEXT fDestroyDebugCallback = nullptr;
    GrVkBackendContext fBackendContext;
    sk_sp<GrContext> fGrContext;
};

} // namespace

bool VulkanTestHelper::init(skiatest::Reporter* reporter) {
    PFN_vkGetInstanceProcAddr instProc;
    PFN_vkGetDeviceProcAddr devProc;
    if (!sk_gpu_test::LoadVkLibraryAndGetProcAddrFuncs(&instProc, &devProc)) {
        return false;
    }
    auto getProc = [&instProc, &devProc](const char* proc_name,
                                         VkInstance instance, VkDevice device) {
        if (device != VK_NULL_HANDLE) {
            return devProc(device, proc_name);
        }
        return instProc(instance, proc_name);
    };

    fExtensions = new GrVkExtensions();
    fFeatures = new VkPhysicalDeviceFeatures2;
    memset(fFeatures, 0, sizeof(VkPhysicalDeviceFeatures2));
    fFeatures->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    fFeatures->pNext = nullptr;

    fBackendContext.fInstance = VK_NULL_HANDLE;
    fBackendContext.fDevice = VK_NULL_HANDLE;

    if (!sk_gpu_test::CreateVkBackendContext(getProc, &fBackendContext, fExtensions,
                                             fFeatures, &fDebugCallback, nullptr,
                                             sk_gpu_test::CanPresentFn(), fIsProtected)) {
        return false;
    }
    fDevice = fBackendContext.fDevice;

    if (fDebugCallback != VK_NULL_HANDLE) {
        fDestroyDebugCallback = (PFN_vkDestroyDebugReportCallbackEXT) instProc(
                fBackendContext.fInstance, "vkDestroyDebugReportCallbackEXT");
    }
    ACQUIRE_INST_VK_PROC(DestroyInstance)
    ACQUIRE_INST_VK_PROC(DeviceWaitIdle)
    ACQUIRE_INST_VK_PROC(DestroyDevice)

    fGrContext = GrContext::MakeVulkan(fBackendContext);
    if (!fGrContext) {
        return false;
    }

    return true;
}

void VulkanTestHelper::cleanup() {
    fGrContext.reset();

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

    delete fExtensions;

    sk_gpu_test::FreeVulkanFeaturesStructs(fFeatures);
    delete fFeatures;
}

sk_sp<SkSurface> VulkanTestHelper::createSkSurface(skiatest::Reporter* reporter) {
    const int kW = 8;
    const int kH = 8;
    GrBackendTexture backendTex = grContext()->createBackendTexture(
        kW, kH, kRGBA_8888_SkColorType, GrMipMapped::kNo, GrRenderable::kNo,
        fIsProtected ? GrProtected::kYes : GrProtected::kNo);
    REPORTER_ASSERT(reporter, backendTex.isValid());
    REPORTER_ASSERT(reporter, backendTex.isProtected() == fIsProtected);

    SkSurfaceProps surfaceProps =
        SkSurfaceProps(0, SkSurfaceProps::kLegacyFontHost_InitType);
    sk_sp<SkSurface> surface = SkSurface::MakeFromBackendTextureAsRenderTarget(
        grContext(), backendTex, kTopLeft_GrSurfaceOrigin, 1,
        kRGBA_8888_SkColorType, nullptr, &surfaceProps);
    REPORTER_ASSERT(reporter, surface);
    return surface;
}

DEF_GPUTEST(VkProtectedContext_CreateNonprotectedContext, reporter, options) {
    auto nonprotectedTestHelper = std::make_unique<VulkanTestHelper>(false);
    REPORTER_ASSERT(reporter, nonprotectedTestHelper->init(reporter));
}


DEF_GPUTEST(VkProtectedContext_CreateProtectedContext, reporter, options) {
    auto protectedTestHelper = std::make_unique<VulkanTestHelper>(true);
    if (!protectedTestHelper->init(reporter)) {
        return;
    }
}

DEF_GPUTEST(VkProtectedContext_CreateProtectedSkSurface, reporter, options) {
    auto protectedTestHelper = std::make_unique<VulkanTestHelper>(true);
    if (!protectedTestHelper->init(reporter)) {
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->grContext() != nullptr);

    const int kW = 8;
    const int kH = 8;
    GrBackendTexture backendTex =
        protectedTestHelper->grContext()->createBackendTexture(
            kW, kH, kRGBA_8888_SkColorType, GrMipMapped::kNo, GrRenderable::kNo,
            GrProtected::kYes);
    REPORTER_ASSERT(reporter, backendTex.isValid());
    REPORTER_ASSERT(reporter, backendTex.isProtected());

    SkSurfaceProps surfaceProps =
        SkSurfaceProps(0, SkSurfaceProps::kLegacyFontHost_InitType);
    sk_sp<SkSurface> surface = SkSurface::MakeFromBackendTextureAsRenderTarget(
        protectedTestHelper->grContext(), backendTex, kTopLeft_GrSurfaceOrigin, 1,
        kRGBA_8888_SkColorType, nullptr, &surfaceProps);
    REPORTER_ASSERT(reporter, surface);

    protectedTestHelper->grContext()->deleteBackendTexture(backendTex);
}

DEF_GPUTEST(VkProtectedContext_CreateNonprotectedTextureInProtectedContext, reporter, options) {
    auto protectedTestHelper = std::make_unique<VulkanTestHelper>(true);
    if (!protectedTestHelper->init(reporter)) {
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->grContext() != nullptr);

    const int kW = 8;
    const int kH = 8;
    GrBackendTexture backendTex =
        protectedTestHelper->grContext()->createBackendTexture(
            kW, kH, kRGBA_8888_SkColorType, GrMipMapped::kNo, GrRenderable::kNo,
            GrProtected::kNo);
    REPORTER_ASSERT(reporter, !backendTex.isValid());
}

DEF_GPUTEST(VkProtectedContext_CreateProtectedTextureInNonprotectedContext, reporter, options) {
    auto protectedTestHelper = std::make_unique<VulkanTestHelper>(false);
    if (!protectedTestHelper->init(reporter)) {
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->grContext() != nullptr);

    const int kW = 8;
    const int kH = 8;
    GrBackendTexture backendTex =
        protectedTestHelper->grContext()->createBackendTexture(
            kW, kH, kRGBA_8888_SkColorType, GrMipMapped::kNo, GrRenderable::kNo,
            GrProtected::kYes);
    REPORTER_ASSERT(reporter, !backendTex.isValid());
}

DEF_GPUTEST(VkProtectedContext_ReadFromProtectedSurface, reporter, options) {
    auto protectedTestHelper = std::make_unique<VulkanTestHelper>(true);
    if (!protectedTestHelper->init(reporter)) {
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->grContext() != nullptr);

    auto surface = protectedTestHelper->createSkSurface(reporter);
    REPORTER_ASSERT(reporter, surface);
    REPORTER_ASSERT(reporter,
                    !surface->readPixels(SkImageInfo(), nullptr, 8, 0, 0));

    protectedTestHelper->grContext()->deleteBackendTexture(
        surface->getBackendTexture(SkSurface::kFlushRead_BackendHandleAccess));
}

DEF_GPUTEST(VkProtectedContext_DrawRectangle, reporter, options) {
    auto protectedTestHelper = std::make_unique<VulkanTestHelper>(true);
    if (!protectedTestHelper->init(reporter)) {
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->grContext() != nullptr);

    auto surface = protectedTestHelper->createSkSurface(reporter);
    REPORTER_ASSERT(reporter, surface);
    SkCanvas* canvas = surface->getCanvas();
    REPORTER_ASSERT(reporter, canvas);
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    canvas->drawRect(SkRect::MakeWH(4, 4), paint);

    GrFlushInfo flushInfo;
    flushInfo.fFlags = kSyncCpu_GrFlushFlag;
    surface->flush(SkSurface::BackendSurfaceAccess::kNoAccess, flushInfo);
    protectedTestHelper->grContext()->deleteBackendTexture(
        surface->getBackendTexture(SkSurface::kFlushRead_BackendHandleAccess));
}

DEF_GPUTEST(VkProtectedContext_DrawRectangleWithAntiAlias, reporter, options) {
    auto protectedTestHelper = std::make_unique<VulkanTestHelper>(true);
    if (!protectedTestHelper->init(reporter)) {
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->grContext() != nullptr);

    auto surface = protectedTestHelper->createSkSurface(reporter);
    REPORTER_ASSERT(reporter, surface);
    SkCanvas* canvas = surface->getCanvas();
    REPORTER_ASSERT(reporter, canvas);
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    paint.setAntiAlias(true);
    canvas->drawRect(SkRect::MakeWH(4, 4), paint);

    GrFlushInfo flushInfo;
    flushInfo.fFlags = kSyncCpu_GrFlushFlag;
    surface->flush(SkSurface::BackendSurfaceAccess::kNoAccess, flushInfo);
    protectedTestHelper->grContext()->deleteBackendTexture(
        surface->getBackendTexture(SkSurface::kFlushRead_BackendHandleAccess));
}

DEF_GPUTEST(VkProtectedContext_DrawRectangleWithBlendMode, reporter, options) {
    auto protectedTestHelper = std::make_unique<VulkanTestHelper>(true);
    if (!protectedTestHelper->init(reporter)) {
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->grContext() != nullptr);

    auto surface = protectedTestHelper->createSkSurface(reporter);
    REPORTER_ASSERT(reporter, surface);
    SkCanvas* canvas = surface->getCanvas();
    REPORTER_ASSERT(reporter, canvas);
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    paint.setBlendMode(SkBlendMode::kColorDodge);
    canvas->drawRect(SkRect::MakeWH(4, 4), paint);

    GrFlushInfo flushInfo;
    flushInfo.fFlags = kSyncCpu_GrFlushFlag;
    surface->flush(SkSurface::BackendSurfaceAccess::kNoAccess, flushInfo);
    protectedTestHelper->grContext()->deleteBackendTexture(
        surface->getBackendTexture(SkSurface::kFlushRead_BackendHandleAccess));
}

DEF_GPUTEST(VkProtectedContext_DrawRectangleWithFilter, reporter, options) {
    auto protectedTestHelper = std::make_unique<VulkanTestHelper>(true);
    if (!protectedTestHelper->init(reporter)) {
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->grContext() != nullptr);

    auto surface = protectedTestHelper->createSkSurface(reporter);
    REPORTER_ASSERT(reporter, surface);
    SkCanvas* canvas = surface->getCanvas();
    REPORTER_ASSERT(reporter, canvas);
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    paint.setStyle(SkPaint::kFill_Style);
    paint.setMaskFilter(SkMaskFilter::MakeBlur(
          SkBlurStyle::kOuter_SkBlurStyle, 1.1f));
    canvas->drawRect(SkRect::MakeWH(4, 4), paint);

    GrFlushInfo flushInfo;
    flushInfo.fFlags = kSyncCpu_GrFlushFlag;
    surface->flush(SkSurface::BackendSurfaceAccess::kNoAccess, flushInfo);
    protectedTestHelper->grContext()->deleteBackendTexture(
        surface->getBackendTexture(SkSurface::kFlushRead_BackendHandleAccess));
}

DEF_GPUTEST(VkProtectedContext_DrawThinPath, reporter, options) {
    auto protectedTestHelper = std::make_unique<VulkanTestHelper>(true);
    if (!protectedTestHelper->init(reporter)) {
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->grContext() != nullptr);

    auto surface = protectedTestHelper->createSkSurface(reporter);
    REPORTER_ASSERT(reporter, surface);
    SkCanvas* canvas = surface->getCanvas();
    REPORTER_ASSERT(reporter, canvas);
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setAntiAlias(true);
    paint.setStrokeWidth(.4f);
    canvas->drawPath(SkPath().moveTo(4, 4).lineTo(6, 6), paint);

    GrFlushInfo flushInfo;
    flushInfo.fFlags = kSyncCpu_GrFlushFlag;
    surface->flush(SkSurface::BackendSurfaceAccess::kNoAccess, flushInfo);
    protectedTestHelper->grContext()->deleteBackendTexture(
        surface->getBackendTexture(SkSurface::kFlushRead_BackendHandleAccess));
}

DEF_GPUTEST(VkProtectedContext_SaveLayer, reporter, options) {
    auto protectedTestHelper = std::make_unique<VulkanTestHelper>(true);
    if (!protectedTestHelper->init(reporter)) {
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->grContext() != nullptr);

    auto surface = protectedTestHelper->createSkSurface(reporter);
    REPORTER_ASSERT(reporter, surface);
    SkCanvas* canvas = surface->getCanvas();
    REPORTER_ASSERT(reporter, canvas);
    canvas->saveLayer(nullptr, nullptr);
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    canvas->drawRect(SkRect::MakeWH(4, 4), paint);
    canvas->restore();

    GrFlushInfo flushInfo;
    flushInfo.fFlags = kSyncCpu_GrFlushFlag;
    surface->flush(SkSurface::BackendSurfaceAccess::kNoAccess, flushInfo);
    protectedTestHelper->grContext()->deleteBackendTexture(
        surface->getBackendTexture(SkSurface::kFlushRead_BackendHandleAccess));
}


DEF_GPUTEST(VkProtectedContext_DrawProtectedImageOnProtectedSurface, reporter, options) {
    auto protectedTestHelper = std::make_unique<VulkanTestHelper>(true);
    if (!protectedTestHelper->init(reporter)) {
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->grContext() != nullptr);

    // Create protected image.
    auto surface1 = protectedTestHelper->createSkSurface(reporter);
    REPORTER_ASSERT(reporter, surface1);
    auto image = surface1->makeImageSnapshot();
    REPORTER_ASSERT(reporter, image);

    // Create protected canvas.
    auto surface2 = protectedTestHelper->createSkSurface(reporter);
    REPORTER_ASSERT(reporter, surface2);
    SkCanvas* canvas = surface2->getCanvas();
    REPORTER_ASSERT(reporter, canvas);

    canvas->drawImage(image, 0, 0);

    GrFlushInfo flushInfo;
    flushInfo.fFlags = kSyncCpu_GrFlushFlag;
    surface1->flush(SkSurface::BackendSurfaceAccess::kNoAccess, flushInfo);
    protectedTestHelper->grContext()->deleteBackendTexture(
        surface1->getBackendTexture(SkSurface::kFlushRead_BackendHandleAccess));
    surface2->flush(SkSurface::BackendSurfaceAccess::kNoAccess, flushInfo);
    protectedTestHelper->grContext()->deleteBackendTexture(
        surface2->getBackendTexture(SkSurface::kFlushRead_BackendHandleAccess));
}

#endif  // SK_SUPPORT_GPU && defined(SK_VULKAN)
