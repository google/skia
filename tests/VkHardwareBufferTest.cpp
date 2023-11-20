/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test. It relies on static initializers to work

#include "include/core/SkTypes.h"

#if defined(SK_GANESH) && defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26 && defined(SK_VULKAN)

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSemaphore.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/MutableTextureState.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/gpu/ganesh/vk/GrVkBackendSemaphore.h"
#include "include/gpu/ganesh/vk/GrVkBackendSurface.h"
#include "include/gpu/ganesh/vk/GrVkDirectContext.h"
#include "include/gpu/vk/GrVkBackendContext.h"
#include "include/gpu/vk/VulkanExtensions.h"
#include "src/base/SkAutoMalloc.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/gl/GrGLDefines.h"
#include "src/gpu/ganesh/gl/GrGLUtil.h"
#include "tests/Test.h"
#include "tools/gpu/GrContextFactory.h"
#include "tools/gpu/vk/VkTestUtils.h"

#include <android/hardware_buffer.h>
#include <cinttypes>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES/gl.h>
#include <GLES/glext.h>

static const int DEV_W = 16, DEV_H = 16;

class BaseTestHelper {
public:
    virtual ~BaseTestHelper() {}

    virtual bool init(skiatest::Reporter* reporter) = 0;

    virtual void cleanup() = 0;
    // This is used to release a surface back to the external queue in vulkan
    virtual void releaseSurfaceToExternal(SkSurface*) = 0;
    virtual void releaseImage() = 0;

    virtual sk_sp<SkImage> importHardwareBufferForRead(skiatest::Reporter* reporter,
                                                       AHardwareBuffer* buffer) = 0;
    virtual sk_sp<SkSurface> importHardwareBufferForWrite(skiatest::Reporter* reporter,
                                                          AHardwareBuffer* buffer) = 0;

    virtual void doClientSync() = 0;
    virtual bool flushSurfaceAndSignalSemaphore(skiatest::Reporter* reporter, sk_sp<SkSurface>) = 0;
    virtual bool importAndWaitOnSemaphore(skiatest::Reporter* reporter, int fdHandle,
                                          sk_sp<SkSurface>) = 0;

    virtual void makeCurrent() = 0;

    virtual GrDirectContext* directContext() = 0;

    int getFdHandle() { return fFdHandle; }

protected:
    BaseTestHelper() {}

    int fFdHandle = 0;
};

#ifdef SK_GL
class EGLTestHelper : public BaseTestHelper {
public:
    EGLTestHelper(const GrContextOptions& options) : fFactory(options) {}

    ~EGLTestHelper() override {}

    void releaseImage() override {
        this->makeCurrent();
        if (!fGLCtx) {
            return;
        }
        if (EGL_NO_IMAGE_KHR != fImage) {
            fGLCtx->destroyEGLImage(fImage);
            fImage = EGL_NO_IMAGE_KHR;
        }
        if (fTexID) {
            GR_GL_CALL(fGLCtx->gl(), DeleteTextures(1, &fTexID));
            fTexID = 0;
        }
    }

    void releaseSurfaceToExternal(SkSurface*) override {}

    void cleanup() override {
        this->releaseImage();
    }

    bool init(skiatest::Reporter* reporter) override;

    sk_sp<SkImage> importHardwareBufferForRead(skiatest::Reporter* reporter,
                                               AHardwareBuffer* buffer) override;
    sk_sp<SkSurface> importHardwareBufferForWrite(skiatest::Reporter* reporter,
                                                  AHardwareBuffer* buffer) override;

    void doClientSync() override;
    bool flushSurfaceAndSignalSemaphore(skiatest::Reporter* reporter, sk_sp<SkSurface>) override;
    bool importAndWaitOnSemaphore(skiatest::Reporter* reporter, int fdHandle,
                                  sk_sp<SkSurface>) override;

    void makeCurrent() override { fGLCtx->makeCurrent(); }

    GrDirectContext* directContext() override { return fDirectContext; }

private:
    bool importHardwareBuffer(skiatest::Reporter* reporter, AHardwareBuffer* buffer);

    typedef EGLClientBuffer (*EGLGetNativeClientBufferANDROIDProc)(const struct AHardwareBuffer*);
    typedef EGLImageKHR (*EGLCreateImageKHRProc)(EGLDisplay, EGLContext, EGLenum, EGLClientBuffer,
                                                 const EGLint*);
    typedef void (*EGLImageTargetTexture2DOESProc)(EGLenum, void*);
    EGLGetNativeClientBufferANDROIDProc fEGLGetNativeClientBufferANDROID;
    EGLCreateImageKHRProc fEGLCreateImageKHR;
    EGLImageTargetTexture2DOESProc fEGLImageTargetTexture2DOES;

    PFNEGLCREATESYNCKHRPROC              fEGLCreateSyncKHR;
    PFNEGLWAITSYNCKHRPROC                fEGLWaitSyncKHR;
    PFNEGLGETSYNCATTRIBKHRPROC           fEGLGetSyncAttribKHR;
    PFNEGLDUPNATIVEFENCEFDANDROIDPROC    fEGLDupNativeFenceFDANDROID;
    PFNEGLDESTROYSYNCKHRPROC             fEGLDestroySyncKHR;

    EGLImageKHR fImage = EGL_NO_IMAGE_KHR;
    GrGLuint fTexID = 0;

    sk_gpu_test::GrContextFactory fFactory;
    sk_gpu_test::ContextInfo fGLESContextInfo;

    sk_gpu_test::GLTestContext* fGLCtx = nullptr;
    GrDirectContext* fDirectContext = nullptr;
};

bool EGLTestHelper::init(skiatest::Reporter* reporter) {
    fGLESContextInfo = fFactory.getContextInfo(skgpu::ContextType::kGLES);
    fDirectContext = fGLESContextInfo.directContext();
    fGLCtx = fGLESContextInfo.glContext();
    if (!fDirectContext || !fGLCtx) {
        return false;
    }

    if (kGLES_GrGLStandard != fGLCtx->gl()->fStandard) {
        return false;
    }

    // Confirm we have egl and the needed extensions
    if (!fGLCtx->gl()->hasExtension("EGL_KHR_image") ||
        !fGLCtx->gl()->hasExtension("EGL_ANDROID_get_native_client_buffer") ||
        !fGLCtx->gl()->hasExtension("GL_OES_EGL_image_external") ||
        !fGLCtx->gl()->hasExtension("GL_OES_EGL_image") ||
        !fGLCtx->gl()->hasExtension("EGL_KHR_fence_sync") ||
        !fGLCtx->gl()->hasExtension("EGL_ANDROID_native_fence_sync")) {
        return false;
    }

    fEGLGetNativeClientBufferANDROID =
        (EGLGetNativeClientBufferANDROIDProc) eglGetProcAddress("eglGetNativeClientBufferANDROID");
    if (!fEGLGetNativeClientBufferANDROID) {
        ERRORF(reporter, "Failed to get the eglGetNativeClientBufferAndroid proc");
        return false;
    }

    fEGLCreateImageKHR = (EGLCreateImageKHRProc) eglGetProcAddress("eglCreateImageKHR");
    if (!fEGLCreateImageKHR) {
        ERRORF(reporter, "Failed to get the proc eglCreateImageKHR");
        return false;
    }

    fEGLImageTargetTexture2DOES =
            (EGLImageTargetTexture2DOESProc) eglGetProcAddress("glEGLImageTargetTexture2DOES");
    if (!fEGLImageTargetTexture2DOES) {
        ERRORF(reporter, "Failed to get the proc EGLImageTargetTexture2DOES");
        return false;
    }

    fEGLCreateSyncKHR = (PFNEGLCREATESYNCKHRPROC) eglGetProcAddress("eglCreateSyncKHR");
    if (!fEGLCreateSyncKHR) {
        ERRORF(reporter, "Failed to get the proc eglCreateSyncKHR");
        return false;

    }
    fEGLWaitSyncKHR = (PFNEGLWAITSYNCKHRPROC) eglGetProcAddress("eglWaitSyncKHR");
    if (!fEGLWaitSyncKHR) {
        ERRORF(reporter, "Failed to get the proc eglWaitSyncKHR");
        return false;

    }
    fEGLGetSyncAttribKHR = (PFNEGLGETSYNCATTRIBKHRPROC) eglGetProcAddress("eglGetSyncAttribKHR");
    if (!fEGLGetSyncAttribKHR) {
        ERRORF(reporter, "Failed to get the proc eglGetSyncAttribKHR");
        return false;

    }
    fEGLDupNativeFenceFDANDROID =
        (PFNEGLDUPNATIVEFENCEFDANDROIDPROC) eglGetProcAddress("eglDupNativeFenceFDANDROID");
    if (!fEGLDupNativeFenceFDANDROID) {
        ERRORF(reporter, "Failed to get the proc eglDupNativeFenceFDANDROID");
        return false;

    }
    fEGLDestroySyncKHR = (PFNEGLDESTROYSYNCKHRPROC) eglGetProcAddress("eglDestroySyncKHR");
    if (!fEGLDestroySyncKHR) {
        ERRORF(reporter, "Failed to get the proc eglDestroySyncKHR");
        return false;

    }

    return true;
}

bool EGLTestHelper::importHardwareBuffer(skiatest::Reporter* reporter, AHardwareBuffer* buffer) {
    while (fGLCtx->gl()->fFunctions.fGetError() != GR_GL_NO_ERROR) {}

    EGLClientBuffer eglClientBuffer = fEGLGetNativeClientBufferANDROID(buffer);
    EGLint eglAttribs[] = { EGL_IMAGE_PRESERVED_KHR, EGL_TRUE,
                            EGL_NONE };
    EGLDisplay eglDisplay = eglGetCurrentDisplay();
    fImage = fEGLCreateImageKHR(eglDisplay, EGL_NO_CONTEXT,
                                EGL_NATIVE_BUFFER_ANDROID,
                                eglClientBuffer, eglAttribs);
    if (EGL_NO_IMAGE_KHR == fImage) {
        SkDebugf("Could not create EGL image, err = (%#x)\n", (int) eglGetError() );
        return false;
    }

    GR_GL_CALL(fGLCtx->gl(), GenTextures(1, &fTexID));
    if (!fTexID) {
        ERRORF(reporter, "Failed to create GL Texture");
        return false;
    }
    GR_GL_CALL_NOERRCHECK(fGLCtx->gl(), BindTexture(GR_GL_TEXTURE_2D, fTexID));
    if (fGLCtx->gl()->fFunctions.fGetError() != GR_GL_NO_ERROR) {
        ERRORF(reporter, "Failed to bind GL Texture");
        return false;
    }

    fEGLImageTargetTexture2DOES(GL_TEXTURE_2D, fImage);
    if (GrGLenum error = fGLCtx->gl()->fFunctions.fGetError(); error != GR_GL_NO_ERROR) {
        ERRORF(reporter, "EGLImageTargetTexture2DOES failed (%#x)", (int) error);
        return false;
    }

    fDirectContext->resetContext(kTextureBinding_GrGLBackendState);
    return true;
}

sk_sp<SkImage> EGLTestHelper::importHardwareBufferForRead(skiatest::Reporter* reporter,
                                                          AHardwareBuffer* buffer) {
    if (!this->importHardwareBuffer(reporter, buffer)) {
        return nullptr;
    }
    GrGLTextureInfo textureInfo;
    textureInfo.fTarget = GR_GL_TEXTURE_2D;
    textureInfo.fID = fTexID;
    textureInfo.fFormat = GR_GL_RGBA8;

    auto backendTex = GrBackendTextures::MakeGL(DEV_W, DEV_H, skgpu::Mipmapped::kNo, textureInfo);
    REPORTER_ASSERT(reporter, backendTex.isValid());

    sk_sp<SkImage> image = SkImages::BorrowTextureFrom(fDirectContext,
                                                       backendTex,
                                                       kTopLeft_GrSurfaceOrigin,
                                                       kRGBA_8888_SkColorType,
                                                       kPremul_SkAlphaType,
                                                       nullptr);

    if (!image) {
        ERRORF(reporter, "Failed to make wrapped GL SkImage");
        return nullptr;
    }

    return image;
}

sk_sp<SkSurface> EGLTestHelper::importHardwareBufferForWrite(skiatest::Reporter* reporter,
                                                             AHardwareBuffer* buffer) {
    if (!this->importHardwareBuffer(reporter, buffer)) {
        return nullptr;
    }
    GrGLTextureInfo textureInfo;
    textureInfo.fTarget = GR_GL_TEXTURE_2D;
    textureInfo.fID = fTexID;
    textureInfo.fFormat = GR_GL_RGBA8;

    auto backendTex = GrBackendTextures::MakeGL(DEV_W, DEV_H, skgpu::Mipmapped::kNo, textureInfo);
    REPORTER_ASSERT(reporter, backendTex.isValid());

    sk_sp<SkSurface> surface = SkSurfaces::WrapBackendTexture(fDirectContext,
                                                              backendTex,
                                                              kTopLeft_GrSurfaceOrigin,
                                                              0,
                                                              kRGBA_8888_SkColorType,
                                                              nullptr,
                                                              nullptr);

    if (!surface) {
        ERRORF(reporter, "Failed to make wrapped GL SkSurface");
        return nullptr;
    }

    return surface;
}

bool EGLTestHelper::flushSurfaceAndSignalSemaphore(skiatest::Reporter* reporter,
                                                   sk_sp<SkSurface> surface) {
    skgpu::ganesh::FlushAndSubmit(surface);

    EGLDisplay eglDisplay = eglGetCurrentDisplay();
    EGLSyncKHR eglsync = fEGLCreateSyncKHR(eglDisplay, EGL_SYNC_NATIVE_FENCE_ANDROID, nullptr);
    if (EGL_NO_SYNC_KHR == eglsync) {
        ERRORF(reporter, "Failed to create EGLSync for EGL_SYNC_NATIVE_FENCE_ANDROID\n");
        return false;
    }

    GR_GL_CALL(fGLCtx->gl(), Flush());
    fFdHandle = fEGLDupNativeFenceFDANDROID(eglDisplay, eglsync);

    EGLint result = fEGLDestroySyncKHR(eglDisplay, eglsync);
    if (EGL_TRUE != result) {
        ERRORF(reporter, "Failed to delete EGLSync, error: %d\n", result);
        return false;
    }

    return true;
}

bool EGLTestHelper::importAndWaitOnSemaphore(skiatest::Reporter* reporter, int fdHandle,
                                             sk_sp<SkSurface> surface) {
    EGLDisplay eglDisplay = eglGetCurrentDisplay();
    EGLint attr[] = {
        EGL_SYNC_NATIVE_FENCE_FD_ANDROID, fdHandle,
        EGL_NONE
    };
    EGLSyncKHR eglsync = fEGLCreateSyncKHR(eglDisplay, EGL_SYNC_NATIVE_FENCE_ANDROID, attr);
    if (EGL_NO_SYNC_KHR == eglsync) {
        ERRORF(reporter,
               "Failed to create EGLSync when importing EGL_SYNC_NATIVE_FENCE_FD_ANDROID\n");
        return false;
    }
    EGLint result = fEGLWaitSyncKHR(eglDisplay, eglsync, 0);
    if (EGL_TRUE != result) {
        ERRORF(reporter, "Failed called to eglWaitSyncKHR, error: %d\n", result);
        // Don't return false yet, try to delete the sync first
    }
    result = fEGLDestroySyncKHR(eglDisplay, eglsync);
    if (EGL_TRUE != result) {
        ERRORF(reporter, "Failed to delete EGLSync, error: %d\n", result);
        return false;
    }
    return true;
}

void EGLTestHelper::doClientSync() {
    this->directContext()->flush();
    this->directContext()->submit(GrSyncCpu::kYes);
}
#endif  // SK_GL

#define DECLARE_VK_PROC(name) PFN_vk##name fVk##name

#define ACQUIRE_INST_VK_PROC(name)                                                           \
    do {                                                                                     \
    fVk##name = reinterpret_cast<PFN_vk##name>(getProc("vk" #name, fBackendContext.fInstance,\
                                                       VK_NULL_HANDLE));                     \
    if (fVk##name == nullptr) {                                                              \
        ERRORF(reporter, "Function ptr for vk%s could not be acquired\n", #name);            \
        return false;                                                                        \
    }                                                                                        \
    } while(false)

#define ACQUIRE_DEVICE_VK_PROC(name)                                                          \
    do {                                                                                      \
    fVk##name = reinterpret_cast<PFN_vk##name>(getProc("vk" #name, VK_NULL_HANDLE, fDevice)); \
    if (fVk##name == nullptr) {                                                               \
        ERRORF(reporter, "Function ptr for vk%s could not be acquired\n", #name);             \
        return false;                                                                         \
    }                                                                                         \
    } while(false)

class VulkanTestHelper : public BaseTestHelper {
public:
    VulkanTestHelper() {}

    ~VulkanTestHelper() override {}

    void releaseImage() override {
        if (VK_NULL_HANDLE == fDevice) {
            return;
        }
        if (fImage != VK_NULL_HANDLE) {
            fVkDestroyImage(fDevice, fImage, nullptr);
            fImage = VK_NULL_HANDLE;
        }

        if (fMemory != VK_NULL_HANDLE) {
            fVkFreeMemory(fDevice, fMemory, nullptr);
            fMemory = VK_NULL_HANDLE;
        }
    }

    void releaseSurfaceToExternal(SkSurface* surface) override {
        skgpu::MutableTextureState newState(VK_IMAGE_LAYOUT_UNDEFINED, VK_QUEUE_FAMILY_EXTERNAL);
        fDirectContext->flush(surface, {}, &newState);
    }

    void cleanup() override {
        fDirectContext.reset();
        this->releaseImage();
        if (fSignalSemaphore != VK_NULL_HANDLE) {
            fVkDestroySemaphore(fDevice, fSignalSemaphore, nullptr);
            fSignalSemaphore = VK_NULL_HANDLE;
        }
        fBackendContext.fMemoryAllocator.reset();
        if (fDevice != VK_NULL_HANDLE) {
            fVkDeviceWaitIdle(fDevice);
            fVkDestroyDevice(fDevice, nullptr);
            fDevice = VK_NULL_HANDLE;
        }
#ifdef SK_ENABLE_VK_LAYERS
        if (fDebugCallback != VK_NULL_HANDLE) {
            fDestroyDebugCallback(fBackendContext.fInstance, fDebugCallback, nullptr);
        }
#endif
        if (fBackendContext.fInstance != VK_NULL_HANDLE) {
            fVkDestroyInstance(fBackendContext.fInstance, nullptr);
            fBackendContext.fInstance = VK_NULL_HANDLE;
        }

        delete fExtensions;

        sk_gpu_test::FreeVulkanFeaturesStructs(fFeatures);
        delete fFeatures;
    }

    bool init(skiatest::Reporter* reporter) override;

    void doClientSync() override {
        if (!fDirectContext) {
            return;
        }

        fDirectContext->submit(GrSyncCpu::kYes);
    }

    bool flushSurfaceAndSignalSemaphore(skiatest::Reporter* reporter, sk_sp<SkSurface>) override;
    bool importAndWaitOnSemaphore(skiatest::Reporter* reporter, int fdHandle,
                                  sk_sp<SkSurface>) override;

    sk_sp<SkImage> importHardwareBufferForRead(skiatest::Reporter* reporter,
                                               AHardwareBuffer* buffer) override;

    sk_sp<SkSurface> importHardwareBufferForWrite(skiatest::Reporter* reporter,
                                                  AHardwareBuffer* buffer) override;

    void makeCurrent() override {}

    GrDirectContext* directContext() override { return fDirectContext.get(); }

private:
    bool checkOptimalHardwareBuffer(skiatest::Reporter* reporter);

    bool importHardwareBuffer(skiatest::Reporter* reporter, AHardwareBuffer* buffer, bool forWrite,
                              GrVkImageInfo* outImageInfo);

    bool setupSemaphoreForSignaling(skiatest::Reporter* reporter, GrBackendSemaphore*);
    bool exportSemaphore(skiatest::Reporter* reporter, const GrBackendSemaphore&);

    DECLARE_VK_PROC(DestroyInstance);
    DECLARE_VK_PROC(DeviceWaitIdle);
    DECLARE_VK_PROC(DestroyDevice);

    DECLARE_VK_PROC(GetPhysicalDeviceExternalSemaphoreProperties);
    DECLARE_VK_PROC(GetPhysicalDeviceImageFormatProperties2);
    DECLARE_VK_PROC(GetPhysicalDeviceMemoryProperties2);

    DECLARE_VK_PROC(GetAndroidHardwareBufferPropertiesANDROID);

    DECLARE_VK_PROC(CreateImage);
    DECLARE_VK_PROC(GetImageMemoryRequirements2);
    DECLARE_VK_PROC(DestroyImage);

    DECLARE_VK_PROC(AllocateMemory);
    DECLARE_VK_PROC(BindImageMemory2);
    DECLARE_VK_PROC(FreeMemory);

    DECLARE_VK_PROC(CreateSemaphore);
    DECLARE_VK_PROC(GetSemaphoreFdKHR);
    DECLARE_VK_PROC(ImportSemaphoreFdKHR);
    DECLARE_VK_PROC(DestroySemaphore);

    VkImage fImage = VK_NULL_HANDLE;
    VkDeviceMemory fMemory = VK_NULL_HANDLE;

    skgpu::VulkanExtensions*            fExtensions = nullptr;
    VkPhysicalDeviceFeatures2*          fFeatures = nullptr;
    VkDebugReportCallbackEXT            fDebugCallback = VK_NULL_HANDLE;
    PFN_vkDestroyDebugReportCallbackEXT fDestroyDebugCallback = nullptr;

    // We hold on to the semaphore so we can delete once the GPU is done.
    VkSemaphore fSignalSemaphore = VK_NULL_HANDLE;

    VkDevice fDevice = VK_NULL_HANDLE;

    GrVkBackendContext fBackendContext;
    sk_sp<GrDirectContext> fDirectContext;
};

bool VulkanTestHelper::init(skiatest::Reporter* reporter) {
    PFN_vkGetInstanceProcAddr instProc;
    if (!sk_gpu_test::LoadVkLibraryAndGetProcAddrFuncs(&instProc)) {
        return false;
    }

    fExtensions = new skgpu::VulkanExtensions();
    fFeatures = new VkPhysicalDeviceFeatures2;
    memset(fFeatures, 0, sizeof(VkPhysicalDeviceFeatures2));
    fFeatures->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    fFeatures->pNext = nullptr;

    fBackendContext.fInstance = VK_NULL_HANDLE;
    fBackendContext.fDevice = VK_NULL_HANDLE;

    if (!sk_gpu_test::CreateVkBackendContext(instProc, &fBackendContext, fExtensions,
                                             fFeatures, &fDebugCallback)) {
        return false;
    }
    fDevice = fBackendContext.fDevice;
    auto getProc = fBackendContext.fGetProc;

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

    fDirectContext = GrDirectContexts::MakeVulkan(fBackendContext);
    REPORTER_ASSERT(reporter, fDirectContext.get());
    if (!fDirectContext) {
        return false;
    }

    return this->checkOptimalHardwareBuffer(reporter);
}

bool VulkanTestHelper::checkOptimalHardwareBuffer(skiatest::Reporter* reporter) {
    VkResult err;

    VkPhysicalDeviceExternalImageFormatInfo externalImageFormatInfo;
    externalImageFormatInfo.sType =
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO;
    externalImageFormatInfo.pNext = nullptr;
    externalImageFormatInfo.handleType =
            VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;
    //externalImageFormatInfo.handType = 0x80;

    // We will create the hardware buffer with gpu sampled so these usages should all be valid
    VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT |
                                   VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                   VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkPhysicalDeviceImageFormatInfo2 imageFormatInfo;
    imageFormatInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
    imageFormatInfo.pNext = &externalImageFormatInfo;
    imageFormatInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageFormatInfo.type = VK_IMAGE_TYPE_2D;
    imageFormatInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageFormatInfo.usage = usageFlags;
    imageFormatInfo.flags = 0;

    VkAndroidHardwareBufferUsageANDROID hwbUsage;
    hwbUsage.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_USAGE_ANDROID;
    hwbUsage.pNext = nullptr;

    VkExternalImageFormatProperties externalImgFormatProps;
    externalImgFormatProps.sType = VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES;
    externalImgFormatProps.pNext = &hwbUsage;

    VkImageFormatProperties2 imgFormProps;
    imgFormProps.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;
    imgFormProps.pNext = &externalImgFormatProps;

    err = fVkGetPhysicalDeviceImageFormatProperties2(fBackendContext.fPhysicalDevice,
                                                     &imageFormatInfo, &imgFormProps);
    if (VK_SUCCESS != err) {
        ERRORF(reporter, "vkGetPhysicalDeviceImageFormatProperites failed, err: %d", err);
        return false;
    }

    const VkImageFormatProperties& imageFormatProperties = imgFormProps.imageFormatProperties;
    REPORTER_ASSERT(reporter, DEV_W <= imageFormatProperties.maxExtent.width);
    REPORTER_ASSERT(reporter, DEV_H <= imageFormatProperties.maxExtent.height);

    const VkExternalMemoryProperties& externalImageFormatProps =
            externalImgFormatProps.externalMemoryProperties;
    REPORTER_ASSERT(reporter, SkToBool(VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT &
                                       externalImageFormatProps.externalMemoryFeatures));
    REPORTER_ASSERT(reporter, SkToBool(VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT &
                                       externalImageFormatProps.externalMemoryFeatures));

    REPORTER_ASSERT(reporter, SkToBool(AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE &
                                       hwbUsage.androidHardwareBufferUsage));

    return true;
}

bool VulkanTestHelper::importHardwareBuffer(skiatest::Reporter* reporter,
                                            AHardwareBuffer* buffer,
                                            bool forWrite,
                                            GrVkImageInfo* outImageInfo) {
    VkResult err;

    VkAndroidHardwareBufferFormatPropertiesANDROID hwbFormatProps;
    hwbFormatProps.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID;
    hwbFormatProps.pNext = nullptr;

    VkAndroidHardwareBufferPropertiesANDROID hwbProps;
    hwbProps.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID;
    hwbProps.pNext = &hwbFormatProps;

    err = fVkGetAndroidHardwareBufferPropertiesANDROID(fDevice, buffer, &hwbProps);
    if (VK_SUCCESS != err) {
        ERRORF(reporter, "GetAndroidHardwareBufferPropertiesAndroid failed, err: %d", err);
        return false;
    }

    REPORTER_ASSERT(reporter, VK_FORMAT_R8G8B8A8_UNORM == hwbFormatProps.format);
    REPORTER_ASSERT(reporter,
                    SkToBool(VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT & hwbFormatProps.formatFeatures) &&
                    SkToBool(VK_FORMAT_FEATURE_TRANSFER_SRC_BIT & hwbFormatProps.formatFeatures) &&
                    SkToBool(VK_FORMAT_FEATURE_TRANSFER_DST_BIT & hwbFormatProps.formatFeatures));
    if (forWrite) {
        REPORTER_ASSERT(reporter,
                SkToBool(VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT & hwbFormatProps.formatFeatures));

    }

    bool useExternalFormat = VK_FORMAT_UNDEFINED == hwbFormatProps.format;
    const VkExternalFormatANDROID externalFormatInfo {
        VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID,             // sType
        nullptr,                                               // pNext
        useExternalFormat ? hwbFormatProps.externalFormat : 0, // externalFormat
    };

    const VkExternalMemoryImageCreateInfo externalMemoryImageInfo {
        VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO, // sType
        &externalFormatInfo, // pNext
        VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID, // handleTypes
    };

    VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT |
                                   VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                   VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (forWrite) {
        usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }

    const VkImageCreateInfo imageCreateInfo = {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,         // sType
        &externalMemoryImageInfo,                    // pNext
        0,                                           // VkImageCreateFlags
        VK_IMAGE_TYPE_2D,                            // VkImageType
        hwbFormatProps.format,                       // VkFormat
        { DEV_W, DEV_H, 1 },                         // VkExtent3D
        1,                                           // mipLevels
        1,                                           // arrayLayers
        VK_SAMPLE_COUNT_1_BIT,                       // samples
        VK_IMAGE_TILING_OPTIMAL,                     // VkImageTiling
        usageFlags,                                  // VkImageUsageFlags
        VK_SHARING_MODE_EXCLUSIVE,                   // VkSharingMode
        0,                                           // queueFamilyCount
        0,                                           // pQueueFamilyIndices
        VK_IMAGE_LAYOUT_UNDEFINED,                   // initialLayout
    };

    err = fVkCreateImage(fDevice, &imageCreateInfo, nullptr, &fImage);
    if (VK_SUCCESS != err) {
        ERRORF(reporter, "Create Image failed, err: %d", err);
        return false;
    }

    VkPhysicalDeviceMemoryProperties2 phyDevMemProps;
    phyDevMemProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
    phyDevMemProps.pNext = nullptr;

    uint32_t typeIndex = 0;
    uint32_t heapIndex = 0;
    bool foundHeap = false;
    fVkGetPhysicalDeviceMemoryProperties2(fBackendContext.fPhysicalDevice, &phyDevMemProps);
    uint32_t memTypeCnt = phyDevMemProps.memoryProperties.memoryTypeCount;
    for (uint32_t i = 0; i < memTypeCnt && !foundHeap; ++i) {
        if (hwbProps.memoryTypeBits & (1 << i)) {
            const VkPhysicalDeviceMemoryProperties& pdmp = phyDevMemProps.memoryProperties;
            uint32_t supportedFlags = pdmp.memoryTypes[i].propertyFlags &
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            if (supportedFlags == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
                typeIndex = i;
                heapIndex = pdmp.memoryTypes[i].heapIndex;
                REPORTER_ASSERT(reporter, heapIndex < pdmp.memoryHeapCount);
                foundHeap = true;
            }
        }
    }

    // Fallback to align with GrAHardwareBufferUtils
    if (!foundHeap && hwbProps.memoryTypeBits) {
        typeIndex = ffs(hwbProps.memoryTypeBits) - 1;
        foundHeap = true;
    }

    if (!foundHeap) {
        ERRORF(reporter, "Failed to find valid heap for imported memory");
        return false;
    }

    VkImportAndroidHardwareBufferInfoANDROID hwbImportInfo;
    hwbImportInfo.sType = VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID;
    hwbImportInfo.pNext = nullptr;
    hwbImportInfo.buffer = buffer;

    VkMemoryDedicatedAllocateInfo dedicatedAllocInfo;
    dedicatedAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO;
    dedicatedAllocInfo.pNext = &hwbImportInfo;
    dedicatedAllocInfo.image = fImage;
    dedicatedAllocInfo.buffer = VK_NULL_HANDLE;

    VkMemoryAllocateInfo allocInfo = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,      // sType
        &dedicatedAllocInfo,                         // pNext
        hwbProps.allocationSize,                     // allocationSize
        typeIndex,                                   // memoryTypeIndex
    };

    err = fVkAllocateMemory(fDevice, &allocInfo, nullptr, &fMemory);
    if (VK_SUCCESS != err) {
        ERRORF(reporter, "AllocateMemory failed for imported buffer, err: %d", err);
        return false;
    }

    VkBindImageMemoryInfo bindImageInfo;
    bindImageInfo.sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO;
    bindImageInfo.pNext = nullptr;
    bindImageInfo.image = fImage;
    bindImageInfo.memory = fMemory;
    bindImageInfo.memoryOffset = 0;

    err = fVkBindImageMemory2(fDevice, 1, &bindImageInfo);
    if (VK_SUCCESS != err) {
        ERRORF(reporter, "BindImageMemory failed for imported buffer, err: %d", err);
        return false;
    }

    skgpu::VulkanAlloc alloc;
    alloc.fMemory = fMemory;
    alloc.fOffset = 0;
    alloc.fSize = hwbProps.allocationSize;
    alloc.fFlags = 0;

    outImageInfo->fImage = fImage;
    outImageInfo->fAlloc = alloc;
    outImageInfo->fImageTiling = VK_IMAGE_TILING_OPTIMAL;
    outImageInfo->fImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    outImageInfo->fFormat = VK_FORMAT_R8G8B8A8_UNORM;
    outImageInfo->fImageUsageFlags = usageFlags;
    outImageInfo->fLevelCount = 1;
    outImageInfo->fCurrentQueueFamily = VK_QUEUE_FAMILY_EXTERNAL;
    return true;
}

sk_sp<SkImage> VulkanTestHelper::importHardwareBufferForRead(skiatest::Reporter* reporter,
                                                             AHardwareBuffer* buffer) {
    GrVkImageInfo imageInfo;
    if (!this->importHardwareBuffer(reporter, buffer, false, &imageInfo)) {
        return nullptr;
    }

    auto backendTex = GrBackendTextures::MakeVk(DEV_W, DEV_H, imageInfo);

    sk_sp<SkImage> wrappedImage = SkImages::BorrowTextureFrom(fDirectContext.get(),
                                                              backendTex,
                                                              kTopLeft_GrSurfaceOrigin,
                                                              kRGBA_8888_SkColorType,
                                                              kPremul_SkAlphaType,
                                                              nullptr);

    if (!wrappedImage.get()) {
        ERRORF(reporter, "Failed to create wrapped Vulkan SkImage");
        return nullptr;
    }

    return wrappedImage;
}

bool VulkanTestHelper::flushSurfaceAndSignalSemaphore(skiatest::Reporter* reporter,
                                                      sk_sp<SkSurface> surface) {
    this->releaseSurfaceToExternal(surface.get());
    surface.reset();
    GrBackendSemaphore semaphore;
    if (!this->setupSemaphoreForSignaling(reporter, &semaphore)) {
        return false;
    }
    GrFlushInfo info;
    info.fNumSemaphores = 1;
    info.fSignalSemaphores = &semaphore;
    GrSemaphoresSubmitted submitted = fDirectContext->flush(info);
    fDirectContext->submit();
    if (GrSemaphoresSubmitted::kNo == submitted) {
        ERRORF(reporter, "Failing call to flush on GrDirectContext");
        return false;
    }
    SkASSERT(semaphore.isInitialized());
    if (!this->exportSemaphore(reporter, semaphore)) {
        return false;
    }
    return true;
}

bool VulkanTestHelper::setupSemaphoreForSignaling(skiatest::Reporter* reporter,
                                                  GrBackendSemaphore* beSemaphore) {
    // Query supported info
    VkPhysicalDeviceExternalSemaphoreInfo exSemInfo;
    exSemInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO;
    exSemInfo.pNext = nullptr;
    exSemInfo.handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT;

    VkExternalSemaphoreProperties exSemProps;
    exSemProps.sType = VK_STRUCTURE_TYPE_EXTERNAL_SEMAPHORE_PROPERTIES;
    exSemProps.pNext = nullptr;

    fVkGetPhysicalDeviceExternalSemaphoreProperties(fBackendContext.fPhysicalDevice, &exSemInfo,
                                                    &exSemProps);

    if (!SkToBool(exSemProps.exportFromImportedHandleTypes &
                 VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT)) {
        ERRORF(reporter, "HANDLE_TYPE_SYNC_FD not listed as exportFromImportedHandleTypes");
        return false;
    }
    if (!SkToBool(exSemProps.compatibleHandleTypes &
                  VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT)) {
        ERRORF(reporter, "HANDLE_TYPE_SYNC_FD not listed as compatibleHandleTypes");
        return false;
    }
    if (!SkToBool(exSemProps.externalSemaphoreFeatures &
                  VK_EXTERNAL_SEMAPHORE_FEATURE_EXPORTABLE_BIT) ||
        !SkToBool(exSemProps.externalSemaphoreFeatures &
                  VK_EXTERNAL_SEMAPHORE_FEATURE_IMPORTABLE_BIT)) {
        ERRORF(reporter, "HANDLE_TYPE_SYNC_FD doesn't support export and import feature");
        return false;
    }

    VkExportSemaphoreCreateInfo exportInfo;
    exportInfo.sType = VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO;
    exportInfo.pNext = nullptr;
    exportInfo.handleTypes = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT;

    VkSemaphoreCreateInfo semaphoreInfo;
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreInfo.pNext = &exportInfo;
    semaphoreInfo.flags = 0;

    VkSemaphore semaphore;
    VkResult err = fVkCreateSemaphore(fDevice, &semaphoreInfo, nullptr, &semaphore);
    if (VK_SUCCESS != err) {
        ERRORF(reporter, "Failed to create signal semaphore, err: %d", err);
        return false;
    }
    *beSemaphore = GrBackendSemaphores::MakeVk(semaphore);
    return true;
}

bool VulkanTestHelper::exportSemaphore(skiatest::Reporter* reporter,
                                       const GrBackendSemaphore& beSemaphore) {
    VkSemaphore semaphore = GrBackendSemaphores::GetVkSemaphore(beSemaphore);
    if (VK_NULL_HANDLE == semaphore) {
        ERRORF(reporter, "Invalid vulkan handle in export call");
        return false;
    }

    VkSemaphoreGetFdInfoKHR getFdInfo;
    getFdInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_GET_FD_INFO_KHR;
    getFdInfo.pNext = nullptr;
    getFdInfo.semaphore = semaphore;
    getFdInfo.handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT;

    VkResult err = fVkGetSemaphoreFdKHR(fDevice, &getFdInfo, &fFdHandle);
    if (VK_SUCCESS != err) {
        ERRORF(reporter, "Failed to export signal semaphore, err: %d", err);
        return false;
    }
    fSignalSemaphore = semaphore;
    return true;
}

bool VulkanTestHelper::importAndWaitOnSemaphore(skiatest::Reporter* reporter, int fdHandle,
                                                sk_sp<SkSurface> surface) {
    VkSemaphoreCreateInfo semaphoreInfo;
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreInfo.pNext = nullptr;
    semaphoreInfo.flags = 0;

    VkSemaphore semaphore;
    VkResult err = fVkCreateSemaphore(fDevice, &semaphoreInfo, nullptr, &semaphore);
    if (VK_SUCCESS != err) {
        ERRORF(reporter, "Failed to create import semaphore, err: %d", err);
        return false;
    }

    VkImportSemaphoreFdInfoKHR importInfo;
    importInfo.sType = VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_FD_INFO_KHR;
    importInfo.pNext = nullptr;
    importInfo.semaphore = semaphore;
    importInfo.flags = VK_SEMAPHORE_IMPORT_TEMPORARY_BIT;
    importInfo.handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT;
    importInfo.fd = fdHandle;

    err = fVkImportSemaphoreFdKHR(fDevice, &importInfo);
    if (VK_SUCCESS != err) {
        ERRORF(reporter, "Failed to import semaphore, err: %d", err);
        return false;
    }

    GrBackendSemaphore beSemaphore = GrBackendSemaphores::MakeVk(semaphore);
    if (!surface->wait(1, &beSemaphore)) {
        ERRORF(reporter, "Failed to add wait semaphore to surface");
        fVkDestroySemaphore(fDevice, semaphore, nullptr);
        return false;
    }
    return true;
}

sk_sp<SkSurface> VulkanTestHelper::importHardwareBufferForWrite(skiatest::Reporter* reporter,
                                                                AHardwareBuffer* buffer) {
    GrVkImageInfo imageInfo;
    if (!this->importHardwareBuffer(reporter, buffer, true, &imageInfo)) {
        return nullptr;
    }

    auto backendTex = GrBackendTextures::MakeVk(DEV_W, DEV_H, imageInfo);

    sk_sp<SkSurface> surface = SkSurfaces::WrapBackendTexture(fDirectContext.get(),
                                                              backendTex,
                                                              kTopLeft_GrSurfaceOrigin,
                                                              0,
                                                              kRGBA_8888_SkColorType,
                                                              nullptr,
                                                              nullptr);

    if (!surface.get()) {
        ERRORF(reporter, "Failed to create wrapped Vulkan SkSurface");
        return nullptr;
    }

    return surface;
}

static SkPMColor get_src_color(int x, int y) {
    SkASSERT(x >= 0 && x < DEV_W);
    SkASSERT(y >= 0 && y < DEV_H);

    U8CPU r = x;
    U8CPU g = y;
    U8CPU b = 0xc;

    U8CPU a = 0xff;
    switch ((x+y) % 5) {
        case 0:
            a = 0xff;
            break;
        case 1:
            a = 0x80;
            break;
        case 2:
            a = 0xCC;
            break;
        case 4:
            a = 0x01;
            break;
        case 3:
            a = 0x00;
            break;
    }
    a = 0xff;
    return SkPremultiplyARGBInline(a, r, g, b);
}

static SkBitmap make_src_bitmap() {
    static SkBitmap bmp;
    if (bmp.isNull()) {
        bmp.allocN32Pixels(DEV_W, DEV_H);
        intptr_t pixels = reinterpret_cast<intptr_t>(bmp.getPixels());
        for (int y = 0; y < DEV_H; ++y) {
            for (int x = 0; x < DEV_W; ++x) {
                SkPMColor* pixel = reinterpret_cast<SkPMColor*>(
                        pixels + y * bmp.rowBytes() + x * bmp.bytesPerPixel());
                *pixel = get_src_color(x, y);
            }
        }
    }
    return bmp;
}

static bool check_read(skiatest::Reporter* reporter, const SkBitmap& srcBitmap,
                       const SkBitmap& dstBitmap) {
    bool result = true;
    for (int y = 0; y < DEV_H && result; ++y) {
        for (int x = 0; x < DEV_W && result; ++x) {
            const uint32_t srcPixel = *srcBitmap.getAddr32(x, y);
            const uint32_t dstPixel = *dstBitmap.getAddr32(x, y);
            if (srcPixel != dstPixel) {
                ERRORF(reporter, "Expected readback pixel (%d, %d) value 0x%08x, got 0x%08x.",
                       x, y,  srcPixel, dstPixel);
                result = false;
            } /*else {
                ERRORF(reporter, "Got good readback pixel (%d, %d) value 0x%08x, got 0x%08x.",
                       x, y,  srcPixel, dstPixel);

            }*/
        }
    }
    return result;
}

static void cleanup_resources(BaseTestHelper* srcHelper, BaseTestHelper* dstHelper,
                              AHardwareBuffer* buffer) {
    if (srcHelper) {
        srcHelper->cleanup();
    }
    if (dstHelper) {
        dstHelper->cleanup();
    }
    if (buffer) {
        AHardwareBuffer_release(buffer);
    }
}

enum class SrcType {
    kCPU,
    kEGL,
    kVulkan,
};

enum class DstType {
    kEGL,
    kVulkan,
};

void run_test(skiatest::Reporter* reporter, const GrContextOptions& options,
              SrcType srcType, DstType dstType, bool shareSyncs) {
    if (SrcType::kCPU == srcType && shareSyncs) {
        // We don't currently test this since we don't do any syncs in this case.
        return;
    }
    std::unique_ptr<BaseTestHelper> srcHelper;
    std::unique_ptr<BaseTestHelper> dstHelper;
    AHardwareBuffer* buffer = nullptr;
    if (SrcType::kVulkan == srcType) {
        srcHelper.reset(new VulkanTestHelper());
    } else if (SrcType::kEGL == srcType) {
#ifdef SK_GL
        srcHelper.reset(new EGLTestHelper(options));
#else
        SkASSERTF(false, "SrcType::kEGL used without OpenGL support.");
#endif
    }
    if (srcHelper) {
        if (!srcHelper->init(reporter)) {
            cleanup_resources(srcHelper.get(), dstHelper.get(), buffer);
            return;
        }
    }

    if (DstType::kVulkan == dstType) {
        dstHelper.reset(new VulkanTestHelper());
    } else {
#ifdef SK_GL
        SkASSERT(DstType::kEGL == dstType);
        dstHelper.reset(new EGLTestHelper(options));
#else
        SkASSERTF(false, "DstType::kEGL used without OpenGL support.");
#endif
    }
    if (dstHelper) {
        if (!dstHelper->init(reporter)) {
            cleanup_resources(srcHelper.get(), dstHelper.get(), buffer);
            return;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // Setup SkBitmaps
    ///////////////////////////////////////////////////////////////////////////

    SkBitmap srcBitmap = make_src_bitmap();
    SkBitmap dstBitmapSurface;
    dstBitmapSurface.allocN32Pixels(DEV_W, DEV_H);
    SkBitmap dstBitmapFinal;
    dstBitmapFinal.allocN32Pixels(DEV_W, DEV_H);

    ///////////////////////////////////////////////////////////////////////////
    // Setup AHardwareBuffer
    ///////////////////////////////////////////////////////////////////////////

    AHardwareBuffer_Desc hwbDesc;
    hwbDesc.width = DEV_W;
    hwbDesc.height = DEV_H;
    hwbDesc.layers = 1;
    if (SrcType::kCPU == srcType) {
        hwbDesc.usage = AHARDWAREBUFFER_USAGE_CPU_READ_NEVER |
                        AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN |
                        AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    } else {
        hwbDesc.usage = AHARDWAREBUFFER_USAGE_CPU_READ_NEVER |
                        AHARDWAREBUFFER_USAGE_CPU_WRITE_NEVER |
                        AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE |
                        AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT;
    }
    hwbDesc.format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
    // The following three are not used in the allocate
    hwbDesc.stride = 0;
    hwbDesc.rfu0= 0;
    hwbDesc.rfu1= 0;

    if (int error = AHardwareBuffer_allocate(&hwbDesc, &buffer)) {
        ERRORF(reporter, "Failed to allocated hardware buffer, error: %d", error);
        cleanup_resources(srcHelper.get(), dstHelper.get(), buffer);
        return;
    }

    if (SrcType::kCPU == srcType) {
        // Get actual desc for allocated buffer so we know the stride for uploading cpu data.
        AHardwareBuffer_describe(buffer, &hwbDesc);

        uint32_t* bufferAddr;
        if (AHardwareBuffer_lock(buffer, AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN, -1, nullptr,
                                 reinterpret_cast<void**>(&bufferAddr))) {
            ERRORF(reporter, "Failed to lock hardware buffer");
            cleanup_resources(srcHelper.get(), dstHelper.get(), buffer);
            return;
        }

        int bbp = srcBitmap.bytesPerPixel();
        uint32_t* src = (uint32_t*)srcBitmap.getPixels();
        uint32_t* dst = bufferAddr;
        for (int y = 0; y < DEV_H; ++y) {
            memcpy(dst, src, DEV_W * bbp);
            src += DEV_W;
            dst += hwbDesc.stride;
        }

        for (int y = 0; y < DEV_H; ++y) {
            for (int x = 0; x < DEV_W; ++x) {
                const uint32_t srcPixel = *srcBitmap.getAddr32(x, y);
                uint32_t dstPixel = bufferAddr[y * hwbDesc.stride + x];
                if (srcPixel != dstPixel) {
                    ERRORF(reporter, "CPU HWB Expected readpix (%d, %d) value 0x%08x, got 0x%08x.",
                           x, y, srcPixel, dstPixel);
                }
            }
        }

        AHardwareBuffer_unlock(buffer, nullptr);

    } else {
        srcHelper->makeCurrent();
        sk_sp<SkSurface> surface = srcHelper->importHardwareBufferForWrite(reporter, buffer);

        if (!surface) {
            cleanup_resources(srcHelper.get(), dstHelper.get(), buffer);
            return;
        }

        sk_sp<SkImage> srcBmpImage = SkImages::RasterFromBitmap(srcBitmap);
        surface->getCanvas()->drawImage(srcBmpImage, 0, 0);

        // If we are testing sharing of syncs, don't do a read here since it forces sychronization
        // to occur.
        if (!shareSyncs) {
            bool readResult = surface->readPixels(dstBitmapSurface, 0, 0);
            if (!readResult) {
                ERRORF(reporter, "Read Pixels on surface failed");
                surface.reset();
                cleanup_resources(srcHelper.get(), dstHelper.get(), buffer);
                return;
            }
            REPORTER_ASSERT(reporter, check_read(reporter, srcBitmap, dstBitmapSurface));
        }

        ///////////////////////////////////////////////////////////////////////////
        // Cleanup GL/EGL and add syncs
        ///////////////////////////////////////////////////////////////////////////

        if (shareSyncs) {
            if (!srcHelper->flushSurfaceAndSignalSemaphore(reporter, std::move(surface))) {
                cleanup_resources(srcHelper.get(), dstHelper.get(), buffer);
                return;
            }
        } else {
            srcHelper->releaseSurfaceToExternal(surface.get());
            srcHelper->doClientSync();
            surface.reset();
            srcHelper->releaseImage();
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // Import the HWB into backend and draw it to a surface
    ///////////////////////////////////////////////////////////////////////////

    dstHelper->makeCurrent();
    sk_sp<SkImage> wrappedImage = dstHelper->importHardwareBufferForRead(reporter, buffer);

    if (!wrappedImage) {
        cleanup_resources(srcHelper.get(), dstHelper.get(), buffer);
        return;
    }

    auto direct = dstHelper->directContext();

    // Make SkSurface to render wrapped HWB into.
    SkImageInfo imageInfo = SkImageInfo::Make(DEV_W, DEV_H, kRGBA_8888_SkColorType,
                                              kPremul_SkAlphaType, nullptr);

    sk_sp<SkSurface> dstSurf = SkSurfaces::RenderTarget(
            direct, skgpu::Budgeted::kNo, imageInfo, 0, kTopLeft_GrSurfaceOrigin, nullptr, false);
    if (!dstSurf.get()) {
        ERRORF(reporter, "Failed to create destination SkSurface");
        wrappedImage.reset();
        cleanup_resources(srcHelper.get(), dstHelper.get(), buffer);
        return;
    }

    if (shareSyncs) {
        if (!dstHelper->importAndWaitOnSemaphore(reporter, srcHelper->getFdHandle(), dstSurf)) {
            wrappedImage.reset();
            cleanup_resources(srcHelper.get(), dstHelper.get(), buffer);
            return;
        }
    }
    dstSurf->getCanvas()->drawImage(wrappedImage, 0, 0);

    bool readResult = dstSurf->readPixels(dstBitmapFinal, 0, 0);
    if (!readResult) {
        ERRORF(reporter, "Read Pixels failed");
        wrappedImage.reset();
        dstSurf.reset();
        dstHelper->doClientSync();
        cleanup_resources(srcHelper.get(), dstHelper.get(), buffer);
        return;
    }

    REPORTER_ASSERT(reporter, check_read(reporter, srcBitmap, dstBitmapFinal));

    dstSurf.reset();
    wrappedImage.reset();
    dstHelper->doClientSync();
    cleanup_resources(srcHelper.get(), dstHelper.get(), buffer);
}

DEF_GANESH_TEST(VulkanHardwareBuffer_CPU_Vulkan, reporter, options, CtsEnforcement::kApiLevel_T) {
    run_test(reporter, options, SrcType::kCPU, DstType::kVulkan, false);
}

DEF_GANESH_TEST(VulkanHardwareBuffer_Vulkan_Vulkan,
                reporter,
                options,
                CtsEnforcement::kApiLevel_T) {
    run_test(reporter, options, SrcType::kVulkan, DstType::kVulkan, false);
}

DEF_GANESH_TEST(VulkanHardwareBuffer_Vulkan_Vulkan_Syncs,
                reporter,
                options,
                CtsEnforcement::kApiLevel_T) {
    run_test(reporter, options, SrcType::kVulkan, DstType::kVulkan, true);
}

#if defined(SK_GL)
DEF_GANESH_TEST(VulkanHardwareBuffer_EGL_Vulkan, reporter, options, CtsEnforcement::kApiLevel_T) {
    run_test(reporter, options, SrcType::kEGL, DstType::kVulkan, false);
}

DEF_GANESH_TEST(VulkanHardwareBuffer_CPU_EGL, reporter, options, CtsEnforcement::kApiLevel_T) {
    run_test(reporter, options, SrcType::kCPU, DstType::kEGL, false);
}

DEF_GANESH_TEST(VulkanHardwareBuffer_EGL_EGL, reporter, options, CtsEnforcement::kApiLevel_T) {
    run_test(reporter, options, SrcType::kEGL, DstType::kEGL, false);
}

DEF_GANESH_TEST(VulkanHardwareBuffer_Vulkan_EGL, reporter, options, CtsEnforcement::kApiLevel_T) {
    run_test(reporter, options, SrcType::kVulkan, DstType::kEGL, false);
}

DEF_GANESH_TEST(VulkanHardwareBuffer_EGL_EGL_Syncs,
                reporter,
                options,
                CtsEnforcement::kApiLevel_T) {
    run_test(reporter, options, SrcType::kEGL, DstType::kEGL, true);
}

DEF_GANESH_TEST(VulkanHardwareBuffer_Vulkan_EGL_Syncs,
                reporter,
                options,
                CtsEnforcement::kApiLevel_T) {
    run_test(reporter, options, SrcType::kVulkan, DstType::kEGL, true);
}

DEF_GANESH_TEST(VulkanHardwareBuffer_EGL_Vulkan_Syncs,
                reporter,
                options,
                CtsEnforcement::kApiLevel_T) {
    run_test(reporter, options, SrcType::kEGL, DstType::kVulkan, true);
}
#endif

#endif  // defined(SK_GANESH) && defined(SK_BUILD_FOR_ANDROID) &&
        // __ANDROID_API__ >= 26 && defined(SK_VULKAN)

