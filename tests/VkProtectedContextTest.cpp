/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test. It relies on static intializers to work

#include "include/core/SkTypes.h"

#if SK_SUPPORT_GPU && defined(SK_VULKAN)

#include "include/gpu/vk/GrVkBackendContext.h"
#include "include/gpu/vk/GrVkExtensions.h"
#include "tests/Test.h"
#include "tools/gpu/GrContextFactory.h"
#include "tools/gpu/vk/VkTestUtils.h"

class VulkanTestHelper {
public:
    VulkanTestHelper(bool isProtected) : fProtected(isProtected) {}

    ~VulkanTestHelper() override {
        cleanup();
    }

    bool init(skiatest::Reporter* reporter);

private:
    void cleanup();

    bool fProtected = false;
    VkDevice fDevice = VK_NULL_HANDLE;
    GrVkBackendContext fBackendContext;
};

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
                                             fFeatures, &fDebugCallback)) {
        return false;
    }
    fDevice = fBackendContext.fDevice;

    if (fDebugCallback != VK_NULL_HANDLE) {
        fDestroyDebugCallback = (PFN_vkDestroyDebugReportCallbackEXT) instProc(
                fBackendContext.fInstance, "vkDestroyDebugReportCallbackEXT");
    }

    ACQUIRE_INST_VK_PROC(DestroyInstance);
    ACQUIRE_INST_VK_PROC(DeviceWaitIdle);
    ACQUIRE_INST_VK_PROC(DestroyDevice);

    if (!fExtensions->hasExtension(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME,
                                  2)) {
        return false;
    }
    if (!fExtensions->hasExtension(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME, 1)) {
        return false;
    }
    if (!fExtensions->hasExtension(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME, 1)) {
        return false;
    }
    if (!fExtensions->hasExtension(VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME, 1)) {
    //    return false;
    }

    ACQUIRE_INST_VK_PROC(GetPhysicalDeviceMemoryProperties2);
    ACQUIRE_INST_VK_PROC(GetPhysicalDeviceImageFormatProperties2);
    ACQUIRE_INST_VK_PROC(GetPhysicalDeviceExternalSemaphoreProperties);

    ACQUIRE_DEVICE_VK_PROC(GetAndroidHardwareBufferPropertiesANDROID);

    ACQUIRE_DEVICE_VK_PROC(CreateImage);
    ACQUIRE_DEVICE_VK_PROC(GetImageMemoryRequirements2);
    ACQUIRE_DEVICE_VK_PROC(DestroyImage);

    ACQUIRE_DEVICE_VK_PROC(AllocateMemory);
    ACQUIRE_DEVICE_VK_PROC(BindImageMemory2);
    ACQUIRE_DEVICE_VK_PROC(FreeMemory);

    ACQUIRE_DEVICE_VK_PROC(CreateSemaphore);
    ACQUIRE_DEVICE_VK_PROC(GetSemaphoreFdKHR);
    ACQUIRE_DEVICE_VK_PROC(ImportSemaphoreFdKHR);
    ACQUIRE_DEVICE_VK_PROC(DestroySemaphore);

    if (fProtected) {
      fBackendContext.fProtectedContext = true;
    }

    fGrContext = GrContext::MakeVulkan(fBackendContext);
    REPORTER_ASSERT(reporter, fGrContext.get());
    if (!fGrContext) {
        return false;
    }

    return this->checkOptimalHardwareBuffer(reporter);
}

DEF_GPUTEST(VkProtectedContext_CreateSkSurface, reporter, options) {
  auto srcHelper = std::make_unique<VulkanTestHelper>();
  srcHelper->setProtectedContext(true);
  REPORTER_ASSERT(reporter, srcHelper->init(reporter));

  const int kW = 32;
  const int kH = 32;
  GrVkGpu* gpu = static_cast<GrVkGpu*>(srcHelper->grContext()->priv().getGpu());
  GrBackendTexture backendTex = gpu->createTestingOnlyBackendTexture(
      nullptr, kW, kH, GrColorType::kRGBA_8888, false, GrMipMapped::kNo);
  REPORTER_ASSERT(reporter, backendTex.isValid());
  GrVkImageInfo info;
  REPORTER_ASSERT(reporter, backendTex.getVkImageInfo(&info));
  GrBackendRenderTarget renderTarget(kW, kH, 0, 0, info);
  SkSurfaceProps surfaceProps =
      SkSurfaceProps(0, SkSurfaceProps::kLegacyFontHost_InitType);
  info.fIsProtected = true;
  auto surface = SkSurface::MakeFromBackendRenderTarget(
      srcHelper->grContext(), renderTarget, kTopLeft_GrSurfaceOrigin,
      kBGRA_8888_SkColorType, nullptr, &surfaceProps);
  REPORTER_ASSERT(reporter, surface);

  info.fIsProtected = false;
  auto surface2 = SkSurface::MakeFromBackendRenderTarget(
      srcHelper->grContext(), renderTarget, kTopLeft_GrSurfaceOrigin,
      kBGRA_8888_SkColorType, nullptr, &surfaceProps);
  REPORTER_ASSERT(reporter, !surface2);
}

#endif  // SK_SUPPORT_GPU && defined(SK_VULKAN)
