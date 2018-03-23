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
#include "SkAutoMalloc.h"
#include "SkCanvas.h"
#include "SkGr.h"
#include "SkImage.h"
#include "SkSurface.h"
#include "Test.h"
#include "../tools/gpu/vk/VkTestUtils.h"
#include "vk/GrVkBackendContext.h"
#include "vk/GrVkExtensions.h"

#include <android/hardware_buffer.h>

static const int DEV_W = 16, DEV_H = 16;

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

#if 0
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
#endif

#define ACQUIRE_VK_PROC(name, instance, device)                                   \
    PFN_vk##name grVk##name =                                                     \
        reinterpret_cast<PFN_vk##name>(getProc("vk" #name, instance, device));    \
    if (grVk##name == nullptr) {                                                  \
        ERRORF(reporter, "Function ptr for vk%s could not be acquired\n", #name); \
        return;                                                                   \
    }

#define ACQUIRE_INST_VK_PROC(name)                                                 \
    PFN_vk##name grVk##name =                                                      \
        reinterpret_cast<PFN_vk##name>(getProc("vk" #name, inst, VK_NULL_HANDLE)); \
    if (grVk##name == nullptr) {                                                   \
        ERRORF(reporter, "Function ptr for vk%s could not be acquired\n", #name);  \
        grVkDestroyInstance(inst, nullptr);                                        \
        return;                                                                    \
    }

#define ACQUIRE_DEVICE_VK_PROC(name)                                                 \
    PFN_vk##name grVk##name =                                                        \
        reinterpret_cast<PFN_vk##name>(getProc("vk" #name, VK_NULL_HANDLE, device)); \
    if (grVk##name == nullptr) {                                                     \
        ERRORF(reporter, "Function ptr for vk%s could not be acquired\n", #name);    \
        grVkDestroyDevice(device, nullptr);                                          \
        grVkDestroyInstance(inst, nullptr);                                          \
        return;                                                                      \
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
            ERRORF(reporter, "failed ot enumerate instance version. Err: %d\n", err);
            return;
        }
    }
    if (instanceVersion < VK_MAKE_VERSION(1, 1, 0)) {
        return;
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
    err = grVkCreateInstance(&instance_create, nullptr, &inst);
    if (err < 0) {
        ERRORF(reporter, "vkCreateInstance failed: %d\n", err);
        return;
    }

    ACQUIRE_VK_PROC(DestroyInstance, inst, VK_NULL_HANDLE);
    ACQUIRE_INST_VK_PROC(EnumeratePhysicalDevices);
    ACQUIRE_INST_VK_PROC(GetPhysicalDeviceProperties);
    ACQUIRE_INST_VK_PROC(GetPhysicalDeviceMemoryProperties2);
    ACQUIRE_INST_VK_PROC(GetPhysicalDeviceQueueFamilyProperties);
    ACQUIRE_INST_VK_PROC(GetPhysicalDeviceFeatures);
    ACQUIRE_INST_VK_PROC(CreateDevice);
    ACQUIRE_INST_VK_PROC(GetDeviceQueue);
    ACQUIRE_INST_VK_PROC(DeviceWaitIdle);
    ACQUIRE_INST_VK_PROC(DestroyDevice);
    ACQUIRE_INST_VK_PROC(GetPhysicalDeviceImageFormatProperties2);

    uint32_t gpuCount;
    err = grVkEnumeratePhysicalDevices(inst, &gpuCount, nullptr);
    if (err) {
        ERRORF(reporter, "vkEnumeratePhysicalDevices failed: %d\n", err);
        grVkDestroyInstance(inst, nullptr);
        return;
    }
    if (!gpuCount) {
        ERRORF(reporter, "vkEnumeratePhysicalDevices returned no supported devices.\n");
        grVkDestroyInstance(inst, nullptr);
        return;
    }
    // Just returning the first physical device instead of getting the whole array.
    // TODO: find best match for our needs
    gpuCount = 1;
    err = grVkEnumeratePhysicalDevices(inst, &gpuCount, &physDev);
    if (err) {
        ERRORF(reporter, "vkEnumeratePhysicalDevices failed: %d\n", err);
        grVkDestroyInstance(inst, nullptr);
        return;
    }

    // query to get the initial queue props size
    uint32_t queueCount;
    grVkGetPhysicalDeviceQueueFamilyProperties(physDev, &queueCount, nullptr);
    if (!queueCount) {
        ERRORF(reporter, "vkGetPhysicalDeviceQueueFamilyProperties returned no queues.\n");
        grVkDestroyInstance(inst, nullptr);
        return;
    }

    SkAutoMalloc queuePropsAlloc(queueCount * sizeof(VkQueueFamilyProperties));
    // now get the actual queue props
    VkQueueFamilyProperties* queueProps = (VkQueueFamilyProperties*)queuePropsAlloc.get();

    grVkGetPhysicalDeviceQueueFamilyProperties(physDev, &queueCount, queueProps);

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
        grVkDestroyInstance(inst, nullptr);
        return;
    }

    VkPhysicalDeviceProperties physDevProperties;
    grVkGetPhysicalDeviceProperties(physDev, &physDevProperties);
    int physDevVersion = physDevProperties.apiVersion;

    extensions.initDevice(physDevVersion, inst, physDev);

    SkTArray<const char*> deviceLayerNames;
    SkTArray<const char*> deviceExtensionNames;
#ifdef SK_ENABLE_VK_LAYERS
    for (size_t i = 0; i < SK_ARRAY_COUNT(kMyDebugLayerNames); ++i) {
        if (extensions.hasDeviceLayer(kMyDebugLayerNames[i])) {
            deviceLayerNames.push_back(kMyDebugLayerNames[i]);
        }
    }
#endif

    if (extensions.hasDeviceExtension(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME)) {
        deviceExtensionNames.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    } else {
        grVkDestroyInstance(inst, nullptr);
        return;
    }

    if (extensions.hasDeviceExtension(
            VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME)) {
        deviceExtensionNames.push_back(
                VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME);
    } else {
        grVkDestroyInstance(inst, nullptr);
        return;
    }

    // query to get the physical device properties
    VkPhysicalDeviceFeatures deviceFeatures;
    grVkGetPhysicalDeviceFeatures(physDev, &deviceFeatures);
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

    err = grVkCreateDevice(physDev, &deviceInfo, nullptr, &device);
    if (err) {
        ERRORF(reporter, "CreateDevice failed: %d\n", err);
        grVkDestroyInstance(inst, nullptr);
        return;
    }

    ACQUIRE_DEVICE_VK_PROC(CreateImage);
    ACQUIRE_DEVICE_VK_PROC(GetImageMemoryRequirements2);
    ACQUIRE_DEVICE_VK_PROC(GetAndroidHardwareBufferPropertiesANDROID);
    ACQUIRE_DEVICE_VK_PROC(AllocateMemory);
    ACQUIRE_DEVICE_VK_PROC(BindImageMemory2);
    ACQUIRE_DEVICE_VK_PROC(DestroyImage);
    ACQUIRE_DEVICE_VK_PROC(FreeMemory);

    VkQueue queue;
    grVkGetDeviceQueue(device, graphicsQueueIndex, 0, &queue);

    // Setting up actual skia things now
    auto interface =
        sk_make_sp<GrVkInterface>(getProc, inst, device, extensionFlags);
    if (!interface->validate(extensionFlags)) {
        ERRORF(reporter, "Vulkan interface validation failed\n");
        grVkDeviceWaitIdle(device);
        grVkDestroyDevice(device, nullptr);
        grVkDestroyInstance(inst, nullptr);
        return;
    }

    sk_sp<GrVkBackendContext> backendCtx(new GrVkBackendContext());
    backendCtx->fInstance = inst;
    backendCtx->fPhysicalDevice = physDev;
    backendCtx->fDevice = device;
    backendCtx->fQueue = queue;
    backendCtx->fGraphicsQueueIndex = graphicsQueueIndex;
    backendCtx->fMinAPIVersion = instanceVersion;
    backendCtx->fExtensions = extensionFlags;
    backendCtx->fFeatures = featureFlags;
    backendCtx->fInterface.reset(interface.release());
    backendCtx->fOwnsInstanceAndDevice = true;

    sk_sp<GrContext> grVkContext = GrContext::MakeVulkan(backendCtx);
    REPORTER_ASSERT(reporter, grVkContext.get());

    ///////////////////////////////////////////////////////////////////////////
    // Finished setting up Vulkan. Setup hardware buffer stuff now
    ///////////////////////////////////////////////////////////////////////////

    SkBitmap srcBitmap = make_src_bitmap();
    SkBitmap dstBitmap;
    dstBitmap.allocN32Pixels(DEV_W, DEV_H);

    AHardwareBuffer_Desc hwbDesc;
    hwbDesc.width = DEV_W;
    hwbDesc.height = DEV_H;
    hwbDesc.layers = 1;
    hwbDesc.usage = AHARDWAREBUFFER_USAGE_CPU_READ_NEVER |
                    AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN |
                    AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    hwbDesc.format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
    // The following three are not used in the allocate
    hwbDesc.stride = 0;
    hwbDesc.rfu0= 0;
    hwbDesc.rfu1= 0;

    AHardwareBuffer* buffer;
    if (int error = AHardwareBuffer_allocate(&hwbDesc, &buffer)) {
        ERRORF(reporter, "Failed to allocated hardware buffer, error: %d", error);
        return;
    }

    // Get actual desc for allocated buffer so we know the stride for uploading cpu data.
    AHardwareBuffer_describe(buffer, &hwbDesc);

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

#if 0
    for (int y = 0; y < DEV_H; ++y) {
        for (int x = 0; x < DEV_W; ++x) {
            const uint32_t srcPixel = *srcBitmap.getAddr32(x, y);
            uint32_t dstPixel = bufferAddr[y * hwbDesc.stride + x];
            if (srcPixel != dstPixel) {
                ERRORF(reporter, "Intitial test Expected readback pixel (%d, %d) value 0x%08x, got 0x%08x.",
                       x, y,  srcPixel, dstPixel);
            }
        }
    }
#endif

    AHardwareBuffer_unlock(buffer, nullptr);

    ///////////////////////////////////////////////////////////////////////////
    // Finished setting up HWB. Get Vulkan properties of HWB
    ///////////////////////////////////////////////////////////////////////////

#if 0
    // Image structs
    VkPhysicalDeviceExternalImage

    VkPhysicalDeviceImageFormatInfo2 imageFormatInfo2;
    imageFormatInfo2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
    imageFormatInfo2.pNext = thing;
    imageFormatInfo2.fomrat = VK_FORMAT_R8G8B8A8_UNORM;

    // Output structs
#endif


    ///////////////////////////////////////////////////////////////////////////
    // Setup Vulkan image to import HWB into to
    ///////////////////////////////////////////////////////////////////////////
    VkImage image = 0;

    // Not currently used
    const VkExternalFormatANDROID externalFormatInfo {
        VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID,   // sType
        nullptr,                                     // pNext
        0,                                           // externalFormat
    };

    const VkExternalMemoryImageCreateInfo externalMemoryImageInfo {
        VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO, // sType
        //&externalFormatInfo, // pNext
        nullptr, // pNext
        VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID, // handleTypes
    };

    // We created the hardware buffer with gpu sampled so these usages should all be valid
    VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT |
                                   VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                   VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    // This is equivalent to the AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM we created the buffer with
    VkFormat vkFormat = VK_FORMAT_R8G8B8A8_UNORM;

    const VkImageCreateInfo imageCreateInfo = {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,         // sType
        &externalMemoryImageInfo,                    // pNext
        0,                                           // VkImageCreateFlags
        VK_IMAGE_TYPE_2D,                            // VkImageType
        vkFormat,                                    // VkFormat
        { DEV_W, DEV_H, 1 },                // VkExtent3D
        //{ hwbDesc.stride, DEV_H, 1 },                // VkExtent3D
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

    err = grVkCreateImage(device, &imageCreateInfo, nullptr, &image);
    if (VK_SUCCESS != err) {
        ERRORF(reporter, "Create Image failed, err: %d", err);
        AHardwareBuffer_release(buffer);
        return;
    }

#if 1
    VkImageMemoryRequirementsInfo2 memReqsInfo;
    memReqsInfo.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2;
    memReqsInfo.pNext = nullptr;
    memReqsInfo.image = image;

    VkMemoryDedicatedRequirements dedicatedMemReqs;
    dedicatedMemReqs.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS;
    dedicatedMemReqs.pNext = nullptr;

    VkMemoryRequirements2 memReqs;
    memReqs.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
    memReqs.pNext = &dedicatedMemReqs;

    grVkGetImageMemoryRequirements2(device, &memReqsInfo, &memReqs);
    REPORTER_ASSERT(reporter, VK_TRUE == dedicatedMemReqs.prefersDedicatedAllocation);
    REPORTER_ASSERT(reporter, VK_TRUE == dedicatedMemReqs.requiresDedicatedAllocation);
#endif

    VkAndroidHardwareBufferPropertiesANDROID hwbProps;
    hwbProps.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID;
    hwbProps.pNext = nullptr; // If using external formats add VkAndroidHardwareBufferFormatPropertiesANDROID

    err = grVkGetAndroidHardwareBufferPropertiesANDROID(device, buffer, &hwbProps);
    if (VK_SUCCESS != err) {
        ERRORF(reporter, "GetAndroidHardwareBufferPropertiesAndoird failed, err: %d", err);
        grVkDestroyImage(device, image, nullptr);
        AHardwareBuffer_release(buffer);
        return;
    }

    VkPhysicalDeviceMemoryProperties2 phyDevMemProps;
    phyDevMemProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
    phyDevMemProps.pNext = nullptr;

    uint32_t typeIndex = 0;
    uint32_t heapIndex = 0;
    bool foundHeap = false;
    grVkGetPhysicalDeviceMemoryProperties2(physDev, &phyDevMemProps);
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
        grVkDestroyImage(device, image, nullptr);
        AHardwareBuffer_release(buffer);
        return;
    }

    VkImportAndroidHardwareBufferInfoANDROID hwbImportInfo;
    hwbImportInfo.sType = VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID;
    hwbImportInfo.pNext = nullptr;
    hwbImportInfo.buffer = buffer;

    VkMemoryDedicatedAllocateInfo dedicatedAllocInfo;
    dedicatedAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO;
    dedicatedAllocInfo.pNext = &hwbImportInfo;
    dedicatedAllocInfo.image = image;
    dedicatedAllocInfo.buffer = VK_NULL_HANDLE;

    VkMemoryAllocateInfo allocInfo = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,      // sType
        &dedicatedAllocInfo,                         // pNext
        hwbProps.allocationSize,                     // allocationSize
        typeIndex,                                   // memoryTypeIndex
    };

    VkDeviceMemory alloc;
    err = grVkAllocateMemory(device, &allocInfo, nullptr, &alloc);
    if (VK_SUCCESS != err) {
        ERRORF(reporter, "AllocateMemory failed for imported buffer, err: %d", err);
        grVkDestroyImage(device, image, nullptr);
        AHardwareBuffer_release(buffer);
        return;
    }

    VkBindImageMemoryInfo bindImageInfo;
    bindImageInfo.sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO;
    bindImageInfo.pNext = nullptr;
    bindImageInfo.image = image;
    bindImageInfo.memory = alloc;
    bindImageInfo.memoryOffset = 0;

    err = grVkBindImageMemory2(device, 1, &bindImageInfo);
    if (VK_SUCCESS != err) {
        ERRORF(reporter, "BindImageMemory failed for imported buffer, err: %d", err);
        grVkDestroyImage(device, image, nullptr);
        grVkFreeMemory(device, alloc, nullptr);
        AHardwareBuffer_release(buffer);
        return;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Wrap in GrBackendTexture and draw it
    ///////////////////////////////////////////////////////////////////////////

    GrVkImageInfo grImageInfo;
    grImageInfo.fImage = image;
    grImageInfo.fAlloc = GrVkAlloc(alloc, 0, hwbProps.allocationSize, 0);
    grImageInfo.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
    grImageInfo.fImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    grImageInfo.fFormat = VK_FORMAT_R8G8B8A8_UNORM;
    grImageInfo.fLevelCount = 1;

    GrBackendTexture backendTex(DEV_W, DEV_H, grImageInfo);
    //GrBackendTexture backendTex(hwbDesc.stride, DEV_H, grImageInfo);

    sk_sp<SkImage> wrappedImage = SkImage::MakeFromTexture(grVkContext.get(),
                                                           backendTex,
                                                           kTopLeft_GrSurfaceOrigin,
                                                           kRGBA_8888_SkColorType,
                                                           kPremul_SkAlphaType,
                                                           nullptr);
    if (!wrappedImage.get()) {
        ERRORF(reporter, "Failed to create wrapped SkImage");
        grVkDestroyImage(device, image, nullptr);
        grVkFreeMemory(device, alloc, nullptr);
        AHardwareBuffer_release(buffer);
        return;
    }

    // Make SkSurface to render into
    SkImageInfo imageInfo = SkImageInfo::Make(DEV_W, DEV_H, kRGBA_8888_SkColorType,
                                              kPremul_SkAlphaType, nullptr);

    sk_sp<SkSurface> dstSurf = SkSurface::MakeRenderTarget(grVkContext.get(), SkBudgeted::kNo,
                                                           imageInfo, 0, kTopLeft_GrSurfaceOrigin,
                                                           nullptr, false);
    if (!dstSurf.get()) {
        ERRORF(reporter, "Failed to create destination SkSurface");
        grVkDestroyImage(device, image, nullptr);
        grVkFreeMemory(device, alloc, nullptr);
        AHardwareBuffer_release(buffer);
        return;
    }

    dstSurf->getCanvas()->drawImage(wrappedImage, 0, 0);

    bool readResult = dstSurf->readPixels(dstBitmap, 0, 0);
    if (!readResult) {
        ERRORF(reporter, "Read Pixels failed");
        grVkDestroyImage(device, image, nullptr);
        grVkFreeMemory(device, alloc, nullptr);
        AHardwareBuffer_release(buffer);
        return;
    }

    for (int y = 0; y < DEV_H; ++y) {
        for (int x = 0; x < DEV_W; ++x) {
            const uint32_t srcPixel = *srcBitmap.getAddr32(x, y);
            const uint32_t dstPixel = *dstBitmap.getAddr32(x, y);
            if (srcPixel != dstPixel) {
                ERRORF(reporter, "Expected readback pixel (%d, %d) value 0x%08x, got 0x%08x.",
                       x, y,  srcPixel, dstPixel);
            } /*else {
                ERRORF(reporter, "Got good readback pixel (%d, %d) value 0x%08x, got 0x%08x.",
                       x, y,  srcPixel, dstPixel);

            }*/
        }
    }

    grVkDestroyImage(device, image, nullptr);
    grVkFreeMemory(device, alloc, nullptr);

    // Destory image
    AHardwareBuffer_release(buffer);
}

#endif
