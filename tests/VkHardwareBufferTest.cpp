/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test. It relies on static intializers to work

#include "SkTypes.h"

#ifdef SKQP_BUILD_HARDWAREBUFFER_TEST
#if SK_SUPPORT_GPU && defined(SK_VULKAN)

#include "GrBackendSemaphore.h"
#include "GrContext.h"
#include "GrContextFactory.h"
#include "GrContextPriv.h"
#include "GrGpu.h"
#include "GrProxyProvider.h"
#include "GrTest.h"
#include "SkAutoMalloc.h"
#include "SkCanvas.h"
#include "SkGr.h"
#include "SkImage.h"
#include "SkSurface.h"
#include "Test.h"
#include "../tools/gpu/vk/VkTestUtils.h"
#include "gl/GrGLDefines.h"
#include "gl/GrGLUtil.h"
#include "vk/GrVkBackendContext.h"
#include "vk/GrVkExtensions.h"

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

    virtual GrContext* grContext() = 0;

    int getFdHandle() { return fFdHandle; }

protected:
    BaseTestHelper() {}

    int fFdHandle = 0;
};

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

    GrContext* grContext() override { return fGrContext; }

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
    GrContext* fGrContext = nullptr;
};

bool EGLTestHelper::init(skiatest::Reporter* reporter) {
    fGLESContextInfo = fFactory.getContextInfo(sk_gpu_test::GrContextFactory::kGLES_ContextType);
    fGrContext = fGLESContextInfo.grContext();
    fGLCtx = fGLESContextInfo.glContext();
    if (!fGrContext || !fGLCtx) {
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
        !fGLCtx->gl()->hasExtension("EGL_KHR_fence_sync")) {
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
    GrGLClearErr(fGLCtx->gl());

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
    if (GR_GL_GET_ERROR(fGLCtx->gl()) != GR_GL_NO_ERROR) {
        ERRORF(reporter, "Failed to bind GL Texture");
        return false;
    }

    fEGLImageTargetTexture2DOES(GL_TEXTURE_2D, fImage);
    GLenum status = GL_NO_ERROR;
    if ((status = glGetError()) != GL_NO_ERROR) {
        ERRORF(reporter, "EGLImageTargetTexture2DOES failed (%#x)", (int) status);
        return false;
    }

    fGrContext->resetContext(kTextureBinding_GrGLBackendState);
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

    GrBackendTexture backendTex(DEV_W, DEV_H, GrMipMapped::kNo, textureInfo);
    REPORTER_ASSERT(reporter, backendTex.isValid());

    sk_sp<SkImage> image = SkImage::MakeFromTexture(fGrContext,
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

    GrBackendTexture backendTex(DEV_W, DEV_H, GrMipMapped::kNo, textureInfo);
    REPORTER_ASSERT(reporter, backendTex.isValid());

    sk_sp<SkSurface> surface = SkSurface::MakeFromBackendTexture(fGrContext,
                                                                 backendTex,
                                                                 kTopLeft_GrSurfaceOrigin,
                                                                 0,
                                                                 kRGBA_8888_SkColorType,
                                                                 nullptr, nullptr);

    if (!surface) {
        ERRORF(reporter, "Failed to make wrapped GL SkSurface");
        return nullptr;
    }

    return surface;
}

bool EGLTestHelper::flushSurfaceAndSignalSemaphore(skiatest::Reporter* reporter,
                                                      sk_sp<SkSurface> surface) {
    EGLDisplay eglDisplay = eglGetCurrentDisplay();
    EGLSyncKHR eglsync = fEGLCreateSyncKHR(eglDisplay, EGL_SYNC_NATIVE_FENCE_ANDROID, nullptr);
    if (EGL_NO_SYNC_KHR == eglsync) {
        ERRORF(reporter, "Failed to create EGLSync for EGL_SYNC_NATIVE_FENCE_ANDROID\n");
        return false;
    }

    surface->flush();
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
    sk_gpu_test::FenceSync* fenceSync = fGLCtx->fenceSync();
    sk_gpu_test::PlatformFence fence = fenceSync->insertFence();
    fenceSync->waitFence(fence);
    fenceSync->deleteFence(fence);
}

#define DECLARE_VK_PROC(name) PFN_vk##name fVk##name

#define ACQUIRE_VK_PROC(name, instance, device)                                        \
    fVk##name = reinterpret_cast<PFN_vk##name>(getProc("vk" #name, instance, device)); \
    if (fVk##name == nullptr) {                                                        \
        ERRORF(reporter, "Function ptr for vk%s could not be acquired\n", #name);      \
        return false;                                                                  \
    }

#define ACQUIRE_INST_VK_PROC(name)                                                          \
    fVk##name = reinterpret_cast<PFN_vk##name>(getProc("vk" #name, fInst, VK_NULL_HANDLE)); \
    if (fVk##name == nullptr) {                                                             \
        ERRORF(reporter, "Function ptr for vk%s could not be acquired\n", #name);           \
        fVkDestroyInstance(fInst, nullptr);                                                 \
        return false;                                                                       \
    }

#define ACQUIRE_DEVICE_VK_PROC(name)                                                          \
    fVk##name = reinterpret_cast<PFN_vk##name>(getProc("vk" #name, VK_NULL_HANDLE, fDevice)); \
    if (fVk##name == nullptr) {                                                               \
        ERRORF(reporter, "Function ptr for vk%s could not be acquired\n", #name);             \
        fVkDestroyDevice(fDevice, nullptr);                                                   \
        fVkDestroyInstance(fInst, nullptr);                                                   \
        return false;                                                                         \
    }

#ifdef SK_ENABLE_VK_LAYERS
const char* kMyDebugLayerNames[] = {
    // elements of VK_LAYER_LUNARG_standard_validation
    "VK_LAYER_GOOGLE_threading",
    "VK_LAYER_LUNARG_parameter_validation",
    "VK_LAYER_LUNARG_object_tracker",
    "VK_LAYER_LUNARG_image",
    "VK_LAYER_LUNARG_core_validation",
    "VK_LAYER_LUNARG_swapchain",
    "VK_LAYER_GOOGLE_unique_objects",
    // not included in standard_validation
    //"VK_LAYER_LUNARG_api_dump",
    //"VK_LAYER_LUNARG_vktrace",
    //"VK_LAYER_LUNARG_screenshot",
};
#endif

class VulkanTestHelper : public BaseTestHelper {
public:
    VulkanTestHelper() {}

    ~VulkanTestHelper() override {}

    void releaseImage() override {
        if (VK_NULL_HANDLE == fDevice) {
            return;
        }
        if (fImage != VK_NULL_HANDLE) {
            SkASSERT(fVkDestroyImage);
            fVkDestroyImage(fDevice, fImage, nullptr);
            fImage = VK_NULL_HANDLE;
        }

        if (fMemory != VK_NULL_HANDLE) {
            SkASSERT(fVkFreeMemory);
            fVkFreeMemory(fDevice, fMemory, nullptr);
            fMemory = VK_NULL_HANDLE;
        }
    }
    void cleanup() override {
        this->releaseImage();

        fGrContext.reset();
        fBackendContext.reset();

        fInst = VK_NULL_HANDLE;
        fPhysDev = VK_NULL_HANDLE;
        fDevice = VK_NULL_HANDLE;
    }

    bool init(skiatest::Reporter* reporter) override;

    void doClientSync() override {
        if (!fGrContext) {
            return;
        }

        fGrContext->contextPriv().getGpu()->testingOnly_flushGpuAndSync();
    }

    bool flushSurfaceAndSignalSemaphore(skiatest::Reporter* reporter, sk_sp<SkSurface>) override;
    bool importAndWaitOnSemaphore(skiatest::Reporter* reporter, int fdHandle,
                                  sk_sp<SkSurface>) override;

    sk_sp<SkImage> importHardwareBufferForRead(skiatest::Reporter* reporter,
                                               AHardwareBuffer* buffer) override;

    sk_sp<SkSurface> importHardwareBufferForWrite(skiatest::Reporter* reporter,
                                                  AHardwareBuffer* buffer) override;

    void makeCurrent() override {}

    GrContext* grContext() override { return fGrContext.get(); }

private:
    bool checkOptimalHardwareBuffer(skiatest::Reporter* reporter);

    bool importHardwareBuffer(skiatest::Reporter* reporter, AHardwareBuffer* buffer, bool forWrite,
                              GrVkImageInfo* outImageInfo);

    bool setupSemaphoreForSignaling(skiatest::Reporter* reporter, GrBackendSemaphore*);
    bool exportSemaphore(skiatest::Reporter* reporter, const GrBackendSemaphore&);

    DECLARE_VK_PROC(EnumerateInstanceVersion);
    DECLARE_VK_PROC(CreateInstance);
    DECLARE_VK_PROC(DestroyInstance);
    DECLARE_VK_PROC(EnumeratePhysicalDevices);
    DECLARE_VK_PROC(GetPhysicalDeviceProperties);
    DECLARE_VK_PROC(GetPhysicalDeviceMemoryProperties2);
    DECLARE_VK_PROC(GetPhysicalDeviceQueueFamilyProperties);
    DECLARE_VK_PROC(GetPhysicalDeviceFeatures);
    DECLARE_VK_PROC(GetPhysicalDeviceExternalSemaphoreProperties);
    DECLARE_VK_PROC(CreateDevice);
    DECLARE_VK_PROC(GetDeviceQueue);
    DECLARE_VK_PROC(DeviceWaitIdle);
    DECLARE_VK_PROC(DestroyDevice);
    DECLARE_VK_PROC(GetPhysicalDeviceImageFormatProperties2);
    DECLARE_VK_PROC(CreateImage);
    DECLARE_VK_PROC(GetImageMemoryRequirements2);
    DECLARE_VK_PROC(GetAndroidHardwareBufferPropertiesANDROID);
    DECLARE_VK_PROC(AllocateMemory);
    DECLARE_VK_PROC(BindImageMemory2);
    DECLARE_VK_PROC(DestroyImage);
    DECLARE_VK_PROC(FreeMemory);
    DECLARE_VK_PROC(CreateSemaphore);
    DECLARE_VK_PROC(GetSemaphoreFdKHR);
    DECLARE_VK_PROC(ImportSemaphoreFdKHR);
    DECLARE_VK_PROC(DestroySemaphore);

    VkInstance fInst = VK_NULL_HANDLE;
    VkPhysicalDevice fPhysDev = VK_NULL_HANDLE;
    VkDevice fDevice = VK_NULL_HANDLE;

    VkImage fImage = VK_NULL_HANDLE;
    VkDeviceMemory fMemory = VK_NULL_HANDLE;

    sk_sp<GrVkBackendContext> fBackendContext;
    sk_sp<GrContext> fGrContext;
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

    VkResult err;

    ACQUIRE_VK_PROC(EnumerateInstanceVersion, VK_NULL_HANDLE, VK_NULL_HANDLE);
    uint32_t instanceVersion = 0;
    if (fVkEnumerateInstanceVersion) {
        err = fVkEnumerateInstanceVersion(&instanceVersion);
        if (err) {
            ERRORF(reporter, "failed to enumerate instance version. Err: %d\n", err);
            return false;
        }
    }
    if (instanceVersion < VK_MAKE_VERSION(1, 1, 0)) {
        return false;
    }

    const VkApplicationInfo app_info = {
        VK_STRUCTURE_TYPE_APPLICATION_INFO, // sType
        nullptr,                            // pNext
        "vkHWBTest",                        // pApplicationName
        0,                                  // applicationVersion
        "vkHWBTest",                        // pEngineName
        0,                                  // engineVerison
        instanceVersion,                    // apiVersion
    };

    GrVkExtensions extensions(getProc);
    extensions.initInstance(instanceVersion);

    SkTArray<const char*> instanceLayerNames;
    SkTArray<const char*> instanceExtensionNames;
    uint32_t extensionFlags = 0;
#ifdef SK_ENABLE_VK_LAYERS
    for (size_t i = 0; i < SK_ARRAY_COUNT(kMyDebugLayerNames); ++i) {
        if (extensions.hasInstanceLayer(kMyDebugLayerNames[i])) {
            instanceLayerNames.push_back(kMyDebugLayerNames[i]);
        }
    }
    if (extensions.hasInstanceExtension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME)) {
        instanceExtensionNames.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        extensionFlags |= kEXT_debug_report_GrVkExtensionFlag;
    }
#endif

    const VkInstanceCreateInfo instance_create = {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,    // sType
        nullptr,                                   // pNext
        0,                                         // flags
        &app_info,                                 // pApplicationInfo
        (uint32_t) instanceLayerNames.count(),     // enabledLayerNameCount
        instanceLayerNames.begin(),                // ppEnabledLayerNames
        (uint32_t) instanceExtensionNames.count(), // enabledExtensionNameCount
        instanceExtensionNames.begin(),            // ppEnabledExtensionNames
    };

    ACQUIRE_VK_PROC(CreateInstance, VK_NULL_HANDLE, VK_NULL_HANDLE);
    err = fVkCreateInstance(&instance_create, nullptr, &fInst);
    if (err < 0) {
        ERRORF(reporter, "vkCreateInstance failed: %d\n", err);
        return false;
    }

    ACQUIRE_VK_PROC(DestroyInstance, fInst, VK_NULL_HANDLE);
    ACQUIRE_INST_VK_PROC(EnumeratePhysicalDevices);
    ACQUIRE_INST_VK_PROC(GetPhysicalDeviceProperties);
    ACQUIRE_INST_VK_PROC(GetPhysicalDeviceQueueFamilyProperties);
    ACQUIRE_INST_VK_PROC(GetPhysicalDeviceFeatures);
    ACQUIRE_INST_VK_PROC(CreateDevice);
    ACQUIRE_INST_VK_PROC(GetDeviceQueue);
    ACQUIRE_INST_VK_PROC(DeviceWaitIdle);
    ACQUIRE_INST_VK_PROC(DestroyDevice);

    uint32_t gpuCount;
    err = fVkEnumeratePhysicalDevices(fInst, &gpuCount, nullptr);
    if (err) {
        ERRORF(reporter, "vkEnumeratePhysicalDevices failed: %d\n", err);
        fVkDestroyInstance(fInst, nullptr);
        return false;
    }
    if (!gpuCount) {
        // We can no physical devices so this isn't an error and failure in the test.
        fVkDestroyInstance(fInst, nullptr);
        return false;
    }
    // Just returning the first physical device instead of getting the whole array.
    // TODO: find best match for our needs
    gpuCount = 1;
    err = fVkEnumeratePhysicalDevices(fInst, &gpuCount, &fPhysDev);
    if (err) {
        ERRORF(reporter, "vkEnumeratePhysicalDevices failed: %d\n", err);
        fVkDestroyInstance(fInst, nullptr);
        return false;
    }

    // query to get the initial queue props size
    uint32_t queueCount;
    fVkGetPhysicalDeviceQueueFamilyProperties(fPhysDev, &queueCount, nullptr);
    if (!queueCount) {
        ERRORF(reporter, "vkGetPhysicalDeviceQueueFamilyProperties returned no queues.\n");
        fVkDestroyInstance(fInst, nullptr);
        return false;
    }

    SkAutoMalloc queuePropsAlloc(queueCount * sizeof(VkQueueFamilyProperties));
    // now get the actual queue props
    VkQueueFamilyProperties* queueProps = (VkQueueFamilyProperties*)queuePropsAlloc.get();

    fVkGetPhysicalDeviceQueueFamilyProperties(fPhysDev, &queueCount, queueProps);

    // iterate to find the graphics queue
    uint32_t graphicsQueueIndex = queueCount;
    for (uint32_t i = 0; i < queueCount; i++) {
        if (queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsQueueIndex = i;
            break;
        }
    }
    if (graphicsQueueIndex == queueCount) {
        ERRORF(reporter, "Could not find any supported graphics queues.\n");
        fVkDestroyInstance(fInst, nullptr);
        return false;
    }

    VkPhysicalDeviceProperties physDevProperties;
    fVkGetPhysicalDeviceProperties(fPhysDev, &physDevProperties);
    int physDevVersion = physDevProperties.apiVersion;

    if (physDevVersion < VK_MAKE_VERSION(1, 1, 0)) {
        return false;
    }

    // Physical-Device-level functions added in 1.1
    ACQUIRE_INST_VK_PROC(GetPhysicalDeviceMemoryProperties2);
    ACQUIRE_INST_VK_PROC(GetPhysicalDeviceImageFormatProperties2);
    ACQUIRE_INST_VK_PROC(GetPhysicalDeviceExternalSemaphoreProperties);

    extensions.initDevice(physDevVersion, fInst, fPhysDev);

    SkTArray<const char*> deviceLayerNames;
    SkTArray<const char*> deviceExtensionNames;
#ifdef SK_ENABLE_VK_LAYERS
    for (size_t i = 0; i < SK_ARRAY_COUNT(kMyDebugLayerNames); ++i) {
        if (extensions.hasDeviceLayer(kMyDebugLayerNames[i])) {
            deviceLayerNames.push_back(kMyDebugLayerNames[i]);
        }
    }
#endif

    if (extensions.hasDeviceExtension(
            VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME)) {
        deviceExtensionNames.push_back(
                VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    } else {
        fVkDestroyInstance(fInst, nullptr);
        return false;
    }

    if (extensions.hasDeviceExtension(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME)) {
        deviceExtensionNames.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    } else {
        ERRORF(reporter, "Has HWB extension, but doesn't not have YCBCR coversion extension");
        fVkDestroyInstance(fInst, nullptr);
        return false;
    }

    if (extensions.hasDeviceExtension(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME)) {
        deviceExtensionNames.push_back(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME);
    } else {
        fVkDestroyInstance(fInst, nullptr);
        return false;
    }

    if (extensions.hasDeviceExtension(VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME)) {
        deviceExtensionNames.push_back(VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME);
    } else {
        SkDebugf("We don't have the extension for VK_EXT_QUEUE_FAMILY_FOREIGN\n");
        //fVkDestroyInstance(fInst, nullptr);
        //return false;
    }

    // query to get the physical device properties
    VkPhysicalDeviceFeatures deviceFeatures;
    fVkGetPhysicalDeviceFeatures(fPhysDev, &deviceFeatures);
    // this looks like it would slow things down,
    // and we can't depend on it on all platforms
    deviceFeatures.robustBufferAccess = VK_FALSE;

    uint32_t featureFlags = 0;
    if (deviceFeatures.geometryShader) {
        featureFlags |= kGeometryShader_GrVkFeatureFlag;
    }
    if (deviceFeatures.dualSrcBlend) {
        featureFlags |= kDualSrcBlend_GrVkFeatureFlag;
    }
    if (deviceFeatures.sampleRateShading) {
        featureFlags |= kSampleRateShading_GrVkFeatureFlag;
    }

    float queuePriorities[1] = { 0.0 };
    // Here we assume no need for swapchain queue
    // If one is needed, the client will need its own setup code
    const VkDeviceQueueCreateInfo queueInfo[1] = {
        {
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, // sType
            nullptr,                                    // pNext
            0,                                          // VkDeviceQueueCreateFlags
            graphicsQueueIndex,                         // queueFamilyIndex
            1,                                          // queueCount
            queuePriorities,                            // pQueuePriorities
        }
    };
    uint32_t queueInfoCount = 1;

    const VkDeviceCreateInfo deviceInfo = {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,    // sType
        nullptr,                                 // pNext
        0,                                       // VkDeviceCreateFlags
        queueInfoCount,                          // queueCreateInfoCount
        queueInfo,                               // pQueueCreateInfos
        (uint32_t) deviceLayerNames.count(),     // layerCount
        deviceLayerNames.begin(),                // ppEnabledLayerNames
        (uint32_t) deviceExtensionNames.count(), // extensionCount
        deviceExtensionNames.begin(),            // ppEnabledExtensionNames
        &deviceFeatures                          // ppEnabledFeatures
    };

    err = fVkCreateDevice(fPhysDev, &deviceInfo, nullptr, &fDevice);
    if (err) {
        ERRORF(reporter, "CreateDevice failed: %d\n", err);
        fVkDestroyInstance(fInst, nullptr);
        return false;
    }

    ACQUIRE_DEVICE_VK_PROC(CreateImage);
    ACQUIRE_DEVICE_VK_PROC(GetImageMemoryRequirements2);
    ACQUIRE_DEVICE_VK_PROC(GetAndroidHardwareBufferPropertiesANDROID);
    ACQUIRE_DEVICE_VK_PROC(AllocateMemory);
    ACQUIRE_DEVICE_VK_PROC(BindImageMemory2);
    ACQUIRE_DEVICE_VK_PROC(DestroyImage);
    ACQUIRE_DEVICE_VK_PROC(FreeMemory);
    ACQUIRE_DEVICE_VK_PROC(CreateSemaphore);
    ACQUIRE_DEVICE_VK_PROC(GetSemaphoreFdKHR);
    ACQUIRE_DEVICE_VK_PROC(ImportSemaphoreFdKHR);
    ACQUIRE_DEVICE_VK_PROC(DestroySemaphore);

    VkQueue queue;
    fVkGetDeviceQueue(fDevice, graphicsQueueIndex, 0, &queue);

    // Setting up actual skia things now
    auto interface =
            sk_make_sp<GrVkInterface>(getProc, fInst, fDevice, extensionFlags);
    if (!interface->validate(extensionFlags)) {
        ERRORF(reporter, "Vulkan interface validation failed\n");
        fVkDeviceWaitIdle(fDevice);
        fVkDestroyDevice(fDevice, nullptr);
        fVkDestroyInstance(fInst, nullptr);
        return false;
    }

    fBackendContext.reset(new GrVkBackendContext());
    fBackendContext->fInstance = fInst;
    fBackendContext->fPhysicalDevice = fPhysDev;
    fBackendContext->fDevice = fDevice;
    fBackendContext->fQueue = queue;
    fBackendContext->fGraphicsQueueIndex = graphicsQueueIndex;
    fBackendContext->fMinAPIVersion = instanceVersion;
    fBackendContext->fExtensions = extensionFlags;
    fBackendContext->fFeatures = featureFlags;
    fBackendContext->fInterface.reset(interface.release());
    fBackendContext->fOwnsInstanceAndDevice = true;

    fGrContext = GrContext::MakeVulkan(fBackendContext);
    REPORTER_ASSERT(reporter, fGrContext.get());

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

    err = fVkGetPhysicalDeviceImageFormatProperties2(fPhysDev, &imageFormatInfo,
                                                        &imgFormProps);
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
        ERRORF(reporter, "GetAndroidHardwareBufferPropertiesAndoird failed, err: %d", err);
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
        //nullptr, // pNext
        VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID, // handleTypes
        //0x80, // handleTypes
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

    VkImageMemoryRequirementsInfo2 memReqsInfo;
    memReqsInfo.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2;
    memReqsInfo.pNext = nullptr;
    memReqsInfo.image = fImage;

    VkMemoryDedicatedRequirements dedicatedMemReqs;
    dedicatedMemReqs.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS;
    dedicatedMemReqs.pNext = nullptr;

    VkMemoryRequirements2 memReqs;
    memReqs.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
    memReqs.pNext = &dedicatedMemReqs;

    fVkGetImageMemoryRequirements2(fDevice, &memReqsInfo, &memReqs);
    REPORTER_ASSERT(reporter, VK_TRUE == dedicatedMemReqs.requiresDedicatedAllocation);

    VkPhysicalDeviceMemoryProperties2 phyDevMemProps;
    phyDevMemProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
    phyDevMemProps.pNext = nullptr;

    uint32_t typeIndex = 0;
    uint32_t heapIndex = 0;
    bool foundHeap = false;
    fVkGetPhysicalDeviceMemoryProperties2(fPhysDev, &phyDevMemProps);
    uint32_t memTypeCnt = phyDevMemProps.memoryProperties.memoryTypeCount;
    for (uint32_t i = 0; i < memTypeCnt && !foundHeap; ++i) {
        if (hwbProps.memoryTypeBits & (1 << i)) {
            const VkPhysicalDeviceMemoryProperties& pdmp = phyDevMemProps.memoryProperties;
            uint32_t supportedFlags = pdmp.memoryTypes[i].propertyFlags &
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            if (supportedFlags == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
                typeIndex = i;
                heapIndex = pdmp.memoryTypes[i].heapIndex;
                foundHeap = true;
            }
        }
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

    outImageInfo->fImage = fImage;
    outImageInfo->fAlloc = GrVkAlloc(fMemory, 0, hwbProps.allocationSize, 0);
    outImageInfo->fImageTiling = VK_IMAGE_TILING_OPTIMAL;
    outImageInfo->fImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    outImageInfo->fFormat = VK_FORMAT_R8G8B8A8_UNORM;
    outImageInfo->fLevelCount = 1;
    outImageInfo->fInitialQueueFamily = VK_QUEUE_FAMILY_EXTERNAL;
    outImageInfo->fCurrentQueueFamily = VK_QUEUE_FAMILY_EXTERNAL;
    return true;
}

sk_sp<SkImage> VulkanTestHelper::importHardwareBufferForRead(skiatest::Reporter* reporter,
                                                             AHardwareBuffer* buffer) {
    GrVkImageInfo imageInfo;
    if (!this->importHardwareBuffer(reporter, buffer, false, &imageInfo)) {
        return nullptr;
    }

    GrBackendTexture backendTex(DEV_W, DEV_H, imageInfo);

    sk_sp<SkImage> wrappedImage = SkImage::MakeFromTexture(fGrContext.get(),
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
    GrBackendSemaphore semaphore;
    if (!this->setupSemaphoreForSignaling(reporter, &semaphore)) {
        return false;
    }
    GrSemaphoresSubmitted submitted = surface->flushAndSignalSemaphores(1, &semaphore);
    if (GrSemaphoresSubmitted::kNo == submitted) {
        ERRORF(reporter, "Failing call to flushAndSignalSemaphores on SkSurface");
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

    fVkGetPhysicalDeviceExternalSemaphoreProperties(fPhysDev, &exSemInfo, &exSemProps);

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
    beSemaphore->initVulkan(semaphore);
    return true;
}

bool VulkanTestHelper::exportSemaphore(skiatest::Reporter* reporter,
                                       const GrBackendSemaphore& beSemaphore) {
    VkSemaphore semaphore = beSemaphore.vkSemaphore();
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
    fVkDestroySemaphore(fDevice, semaphore, nullptr);
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

    GrBackendSemaphore beSemaphore;
    beSemaphore.initVulkan(semaphore);
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

    GrBackendTexture backendTex(DEV_W, DEV_H, imageInfo);

    sk_sp<SkSurface> surface = SkSurface::MakeFromBackendTexture(fGrContext.get(),
                                                                 backendTex,
                                                                 kTopLeft_GrSurfaceOrigin,
                                                                 0,
                                                                 kRGBA_8888_SkColorType,
                                                                 nullptr, nullptr);

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
        srcHelper.reset(new EGLTestHelper(options));
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
        SkASSERT(DstType::kEGL == dstType);
        dstHelper.reset(new EGLTestHelper(options));
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

        sk_sp<SkImage> srcBmpImage = SkImage::MakeFromBitmap(srcBitmap);
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
            if (!srcHelper->flushSurfaceAndSignalSemaphore(reporter, surface)) {
                cleanup_resources(srcHelper.get(), dstHelper.get(), buffer);
                return;
            }

            surface.reset();
        } else {
            surface.reset();
            srcHelper->doClientSync();
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

    GrContext* grContext = dstHelper->grContext();

    // Make SkSurface to render wrapped HWB into.
    SkImageInfo imageInfo = SkImageInfo::Make(DEV_W, DEV_H, kRGBA_8888_SkColorType,
                                              kPremul_SkAlphaType, nullptr);

    sk_sp<SkSurface> dstSurf = SkSurface::MakeRenderTarget(grContext,
                                                           SkBudgeted::kNo, imageInfo, 0,
                                                           kTopLeft_GrSurfaceOrigin,
                                                           nullptr, false);
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
        dstHelper->doClientSync();
        cleanup_resources(srcHelper.get(), dstHelper.get(), buffer);
        return;
    }

    REPORTER_ASSERT(reporter, check_read(reporter, srcBitmap, dstBitmapFinal));

    wrappedImage.reset();
    dstHelper->doClientSync();
    cleanup_resources(srcHelper.get(), dstHelper.get(), buffer);
}

DEF_GPUTEST(VulkanHardwareBuffer_CPU_Vulkan, reporter, options) {
    run_test(reporter, options, SrcType::kCPU, DstType::kVulkan, false);
}

DEF_GPUTEST(VulkanHardwareBuffer_EGL_Vulkan, reporter, options) {
    run_test(reporter, options, SrcType::kEGL, DstType::kVulkan, false);
}

DEF_GPUTEST(VulkanHardwareBuffer_Vulkan_Vulkan, reporter, options) {
    run_test(reporter, options, SrcType::kVulkan, DstType::kVulkan, false);
}

DEF_GPUTEST(VulkanHardwareBuffer_CPU_EGL, reporter, options) {
    run_test(reporter, options, SrcType::kCPU, DstType::kEGL, false);
}

DEF_GPUTEST(VulkanHardwareBuffer_EGL_EGL, reporter, options) {
    run_test(reporter, options, SrcType::kEGL, DstType::kEGL, false);
}

DEF_GPUTEST(VulkanHardwareBuffer_Vulkan_EGL, reporter, options) {
    run_test(reporter, options, SrcType::kVulkan, DstType::kEGL, false);
}

DEF_GPUTEST(VulkanHardwareBuffer_EGL_EGL_Syncs, reporter, options) {
    run_test(reporter, options, SrcType::kEGL, DstType::kEGL, true);
}

DEF_GPUTEST(VulkanHardwareBuffer_Vulkan_EGL_Syncs, reporter, options) {
    run_test(reporter, options, SrcType::kVulkan, DstType::kEGL, true);
}

DEF_GPUTEST(VulkanHardwareBuffer_EGL_Vulkan_Syncs, reporter, options) {
    run_test(reporter, options, SrcType::kEGL, DstType::kVulkan, true);
}

DEF_GPUTEST(VulkanHardwareBuffer_Vulkan_Vulkan_Syncs, reporter, options) {
    run_test(reporter, options, SrcType::kVulkan, DstType::kVulkan, true);
}

#endif
#endif

