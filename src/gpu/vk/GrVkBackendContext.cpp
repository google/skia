/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAutoMalloc.h"
#include "vk/GrVkBackendContext.h"
#include "vk/GrVkExtensions.h"
#include "vk/GrVkInterface.h"
#include "vk/GrVkUtil.h"

////////////////////////////////////////////////////////////////////////////////
// Helper code to set up Vulkan context objects

#ifdef SK_ENABLE_VK_LAYERS
const char* kDebugLayerNames[] = {
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

// the minimum version of Vulkan supported
#ifdef SK_BUILD_FOR_ANDROID
const uint32_t kGrVkMinimumVersion = VK_MAKE_VERSION(1, 0, 3);
#else
const uint32_t kGrVkMinimumVersion = VK_MAKE_VERSION(1, 0, 8);
#endif

#define ACQUIRE_VK_PROC(name, instance, device)                                \
    PFN_vk##name grVk##name =                                                  \
        reinterpret_cast<PFN_vk##name>(getProc("vk" #name, instance, device)); \
    if (grVk##name == nullptr) {                                               \
        SkDebugf("Function ptr for vk%s could not be acquired\n", #name);      \
        return nullptr;                                                        \
    }

// Create the base Vulkan objects needed by the GrVkGpu object
const GrVkBackendContext* GrVkBackendContext::Create(uint32_t* presentQueueIndexPtr,
                                                     CanPresentFn canPresent,
                                                     GrVkInterface::GetProc getProc) {
    if (!getProc) {
        return nullptr;
    }
    SkASSERT(getProc);

    VkPhysicalDevice physDev;
    VkDevice device;
    VkInstance inst;
    VkResult err;

    const VkApplicationInfo app_info = {
        VK_STRUCTURE_TYPE_APPLICATION_INFO, // sType
        nullptr,                            // pNext
        "vktest",                           // pApplicationName
        0,                                  // applicationVersion
        "vktest",                           // pEngineName
        0,                                  // engineVerison
        kGrVkMinimumVersion,                // apiVersion
    };

    GrVkExtensions extensions(getProc);
    extensions.initInstance(kGrVkMinimumVersion);

    SkTArray<const char*> instanceLayerNames;
    SkTArray<const char*> instanceExtensionNames;
    uint32_t extensionFlags = 0;
#ifdef SK_ENABLE_VK_LAYERS
    for (size_t i = 0; i < SK_ARRAY_COUNT(kDebugLayerNames); ++i) {
        if (extensions.hasInstanceLayer(kDebugLayerNames[i])) {
            instanceLayerNames.push_back(kDebugLayerNames[i]);
        }
    }
    if (extensions.hasInstanceExtension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME)) {
        instanceExtensionNames.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        extensionFlags |= kEXT_debug_report_GrVkExtensionFlag;
    }
#endif

    if (extensions.hasInstanceExtension(VK_KHR_SURFACE_EXTENSION_NAME)) {
        instanceExtensionNames.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
        extensionFlags |= kKHR_surface_GrVkExtensionFlag;
    }
    if (extensions.hasInstanceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME)) {
        instanceExtensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        extensionFlags |= kKHR_swapchain_GrVkExtensionFlag;
    }
#ifdef SK_BUILD_FOR_WIN
    if (extensions.hasInstanceExtension(VK_KHR_WIN32_SURFACE_EXTENSION_NAME)) {
        instanceExtensionNames.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
        extensionFlags |= kKHR_win32_surface_GrVkExtensionFlag;
    }
#elif defined(SK_BUILD_FOR_ANDROID)
    if (extensions.hasInstanceExtension(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME)) {
        instanceExtensionNames.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
        extensionFlags |= kKHR_android_surface_GrVkExtensionFlag;
    }
#elif defined(SK_BUILD_FOR_UNIX) && !defined(__Fuchsia__)
    if (extensions.hasInstanceExtension(VK_KHR_XCB_SURFACE_EXTENSION_NAME)) {
        instanceExtensionNames.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
        extensionFlags |= kKHR_xcb_surface_GrVkExtensionFlag;
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
        SkDebugf("vkCreateInstance failed: %d\n", err);
        return nullptr;
    }

    ACQUIRE_VK_PROC(DestroyInstance, inst, VK_NULL_HANDLE);
    ACQUIRE_VK_PROC(EnumeratePhysicalDevices, inst, VK_NULL_HANDLE);
    ACQUIRE_VK_PROC(GetPhysicalDeviceQueueFamilyProperties, inst, VK_NULL_HANDLE);
    ACQUIRE_VK_PROC(GetPhysicalDeviceFeatures, inst, VK_NULL_HANDLE);
    ACQUIRE_VK_PROC(CreateDevice, inst, VK_NULL_HANDLE);
    ACQUIRE_VK_PROC(GetDeviceQueue, inst, VK_NULL_HANDLE);
    ACQUIRE_VK_PROC(DeviceWaitIdle, inst, VK_NULL_HANDLE);
    ACQUIRE_VK_PROC(DestroyDevice, inst, VK_NULL_HANDLE);

    uint32_t gpuCount;
    err = grVkEnumeratePhysicalDevices(inst, &gpuCount, nullptr);
    if (err) {
        SkDebugf("vkEnumeratePhysicalDevices failed: %d\n", err);
        grVkDestroyInstance(inst, nullptr);
        return nullptr;
    }
    if (!gpuCount) {
        SkDebugf("vkEnumeratePhysicalDevices returned no supported devices.\n");
        grVkDestroyInstance(inst, nullptr);
        return nullptr;
    }
    // Just returning the first physical device instead of getting the whole array.
    // TODO: find best match for our needs
    gpuCount = 1;
    err = grVkEnumeratePhysicalDevices(inst, &gpuCount, &physDev);
    if (err) {
        SkDebugf("vkEnumeratePhysicalDevices failed: %d\n", err);
        grVkDestroyInstance(inst, nullptr);
        return nullptr;
    }

    // query to get the initial queue props size
    uint32_t queueCount;
    grVkGetPhysicalDeviceQueueFamilyProperties(physDev, &queueCount, nullptr);
    if (!queueCount) {
        SkDebugf("vkGetPhysicalDeviceQueueFamilyProperties returned no queues.\n");
        grVkDestroyInstance(inst, nullptr);
        return nullptr;
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
        SkDebugf("Could not find any supported graphics queues.\n");
        grVkDestroyInstance(inst, nullptr);
        return nullptr;
    }

    // iterate to find the present queue, if needed
    uint32_t presentQueueIndex = queueCount;
    if (presentQueueIndexPtr && canPresent) {
        for (uint32_t i = 0; i < queueCount; i++) {
            if (canPresent(inst, physDev, i)) {
                presentQueueIndex = i;
                break;
            }
        }
        if (presentQueueIndex == queueCount) {
            SkDebugf("Could not find any supported present queues.\n");
            grVkDestroyInstance(inst, nullptr);
            return nullptr;
        }
        *presentQueueIndexPtr = presentQueueIndex;
    } else {
        // Just setting this so we end up make a single queue for graphics since there was no
        // request for a present queue.
        presentQueueIndex = graphicsQueueIndex;
    }

    extensions.initDevice(kGrVkMinimumVersion, inst, physDev);

    SkTArray<const char*> deviceLayerNames;
    SkTArray<const char*> deviceExtensionNames;
#ifdef SK_ENABLE_VK_LAYERS
    for (size_t i = 0; i < SK_ARRAY_COUNT(kDebugLayerNames); ++i) {
        if (extensions.hasDeviceLayer(kDebugLayerNames[i])) {
            deviceLayerNames.push_back(kDebugLayerNames[i]);
        }
    }
#endif
    if (extensions.hasDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME)) {
        deviceExtensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        extensionFlags |= kKHR_swapchain_GrVkExtensionFlag;
    }
    if (extensions.hasDeviceExtension("VK_NV_glsl_shader")) {
        deviceExtensionNames.push_back("VK_NV_glsl_shader");
        extensionFlags |= kNV_glsl_shader_GrVkExtensionFlag;
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
    const VkDeviceQueueCreateInfo queueInfo[2] = {
        {
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, // sType
            nullptr,                                    // pNext
            0,                                          // VkDeviceQueueCreateFlags
            graphicsQueueIndex,                         // queueFamilyIndex
            1,                                          // queueCount
            queuePriorities,                            // pQueuePriorities
        },
        {
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, // sType
            nullptr,                                    // pNext
            0,                                          // VkDeviceQueueCreateFlags
            presentQueueIndex,                          // queueFamilyIndex
            1,                                          // queueCount
            queuePriorities,                            // pQueuePriorities
        }
    };
    uint32_t queueInfoCount = (presentQueueIndex != graphicsQueueIndex) ? 2 : 1;

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
        SkDebugf("CreateDevice failed: %d\n", err);
        grVkDestroyInstance(inst, nullptr);
        return nullptr;
    }

    auto interface =
        sk_make_sp<GrVkInterface>(getProc, inst, device, extensionFlags);
    if (!interface->validate(extensionFlags)) {
        SkDebugf("Vulkan interface validation failed\n");
        grVkDeviceWaitIdle(device);
        grVkDestroyDevice(device, nullptr);
        grVkDestroyInstance(inst, nullptr);
        return nullptr;
    }

    VkQueue queue;
    grVkGetDeviceQueue(device, graphicsQueueIndex, 0, &queue);

    GrVkBackendContext* ctx = new GrVkBackendContext();
    ctx->fInstance = inst;
    ctx->fPhysicalDevice = physDev;
    ctx->fDevice = device;
    ctx->fQueue = queue;
    ctx->fGraphicsQueueIndex = graphicsQueueIndex;
    ctx->fMinAPIVersion = kGrVkMinimumVersion;
    ctx->fExtensions = extensionFlags;
    ctx->fFeatures = featureFlags;
    ctx->fInterface.reset(interface.release());
    ctx->fOwnsInstanceAndDevice = true;

    return ctx;
}

GrVkBackendContext::~GrVkBackendContext() {
    if (fInterface == nullptr || !fOwnsInstanceAndDevice) {
        return;
    }

    fInterface->fFunctions.fDeviceWaitIdle(fDevice);
    fInterface->fFunctions.fDestroyDevice(fDevice, nullptr);
    fDevice = VK_NULL_HANDLE;
    fInterface->fFunctions.fDestroyInstance(fInstance, nullptr);
    fInstance = VK_NULL_HANDLE;
}
