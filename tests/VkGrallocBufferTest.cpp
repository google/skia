/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test. It relies on static intializers to work

#include "SkTypes.h"

#if SK_SUPPORT_GPU && defined(SK_VULKAN) && defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26

#include "GrContext.h"
#include "GrContextFactory.h"
#include "GrContextPriv.h"
#include "GrProxyProvider.h"
#include "GrTest.h"
#include "SkGr.h"
#include "Test.h"
#include "VkTestUtils.h"
#include "vk/GrVkExtensions.h"

#include <android/hardware_buffer.h>

static const int DEV_W = 100, DEV_H = 100;

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

bool check_read(skiatest::Reporter* reporter, const SkBitmap& srcBitmap,
                const SkBitmap& dstBitmap) {

    for (int y = 0; y < DEV_H; ++y) {
        for (int x = 0; x < DEV_W; ++x) {
            const uint32_t srcPixel = *srcBitmap.getAddr32(x, y);
            const uint32_t dstPixel = *srcBitmap.getAddr32(x, y);
            if (srcPixel != dstPixel) {
                ERRORF(reporter, "Expected readback pixel (%d, %d) value 0x%08x, got 0x%08x.",
                       x, y,  srcPixel, dstPixel);
                return false;
            }
        }
    }
    return true;
}

DEF_GPUTEST_FOR_VULKAN_CONTEXT(VkGrallocBufferTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    SkBitmap srcBitmap = make_src_bitmap();
    SkBitmap dstBitmap;
    dstBitmap.allocN32Pixels(DEV_W, DEV_H);

    SkDebugf("top\n");
    {
        sk_sp<GrTextureProxy> proxy = GrUploadBitmapToTextureProxy(
                context->contextPriv().proxyProvider(), srcBitmap, nullptr);
        sk_sp<GrSurfaceContext> sContext = context->contextPriv().makeWrappedSurfaceContext(
                std::move(proxy));

        bool success = sContext->readPixels(dstBitmap.info(), dstBitmap.getPixels(),
                                            dstBitmap.rowBytes(), 0, 0, 0);
        REPORTER_ASSERT(reporter, success);
        REPORTER_ASSERT(reporter, check_read(reporter, srcBitmap, dstBitmap));
    }

    SkDebugf("I am here\n");
    AHardwareBuffer_Desc hwbDesc;
    hwbDesc.width = DEV_W;
    hwbDesc.height = DEV_H;
    hwbDesc.layers = 1;
    hwbDesc.usage = AHARDWAREBUFFER_USAGE_CPU_READ_NEVER | AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN;
    hwbDesc.format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
    // The following three are not used in the allocate
    hwbDesc.stride = 0;
    hwbDesc.rfu0= 0;
    hwbDesc.rfu1= 0;

    SkDebugf("Here 2\n");
    AHardwareBuffer* buffer;
    if (int error = AHardwareBuffer_allocate(&hwbDesc, &buffer)) {
    SkDebugf("Here 3\n");
        ERRORF(reporter, "Failed to allocated hardware buffer, error: %d", error);
        return;
    }

    // Get actual desc for allocated buffer so we know the stride for uploading cpu data.
    AHardwareBuffer_describe(buffer, &hwbDesc);
    SkDebugf("W: %d, H: %d, Stride: %d\n", DEV_W, DEV_H, hwbDesc.stride);

    uint32_t* bufferAddr;
    if (AHardwareBuffer_lock(buffer, AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN, -1, nullptr,
                             reinterpret_cast<void**>(&bufferAddr))) {
        ERRORF(reporter, "Failed to lock hardware buffer");
        AHardwareBuffer_release(buffer);
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
                ERRORF(reporter, "Expected readback pixel (%d, %d) value 0x%08x, got 0x%08x.",
                       x, y,  srcPixel, dstPixel);
            }
        }
    }

    AHardwareBuffer_unlock(buffer, nullptr);

    SkDebugf("Here 4\n");
    AHardwareBuffer_release(buffer);
    SkDebugf("Here 5\n");
}

#define ACQUIRE_VK_PROC(name, instance, device)                                \
    PFN_vk##name grVk##name =                                                  \
        reinterpret_cast<PFN_vk##name>(getProc("vk" #name, instance, device)); \
    if (grVk##name == nullptr) {                                               \
        SkDebugf("Function ptr for vk%s could not be acquired\n", #name);      \
        return;                                                                \
    }

DEF_GPUTEST(VulkanHardwareBuffer, reporter, /* options */) {
    PFN_vkGetInstanceProcAddr instProc;
    PFN_vkGetDeviceProcAddr devProc;
    if (!sk_gpu_test::LoadVkLibraryAndGetProcAddrFuncs(&instProc, &devProc)) {
        return;
    }

    auto getProc = [&instProc, &devProc](const char* proc_name,
                                         VkInstance instance, VkDevice device) {
        if (device != VK_NULL_HANDLE) {
            return devProc(device, proc_name);
        }
        return instProc(instance, proc_name);
    };

    VkInstance inst;
    VkPhysicalDevice physDev;
    VkDevice device;
    VkResult err;

    ACQUIRE_VK_PROC(EnumerateInstanceVersion, VK_NULL_HANDLE, VK_NULL_HANDLE);
    uint32_t instanceVersion = 0;
    if (grVkEnumerateInstanceVersion) {
        err = grVkEnumerateInstanceVersion(&instanceVersion);
        if (err) {
            SkDebugf("failed ot enumerate instance version. Err: %d\n", err);
            return;
        }
    }
    if (instanceVersion < VK_MAKE_VERSION(1, 1, 0)) {
        return;
    }

    SkDebugf("I am here\n");

    GrVkExtensions extensions(getProc);
    extensions.initInstance(instanceVersion);

    const VkApplicationInfo app_info = {
        VK_STRUCTURE_TYPE_APPLICATION_INFO, // sType
        nullptr,                            // pNext
        "vkHWBTest",                        // pApplicationName
        0,                                  // applicationVersion
        "vkHWBTest",                        // pEngineName
        0,                                  // engineVerison
        instanceVersion,                    // apiVersion
    };



}

#endif
