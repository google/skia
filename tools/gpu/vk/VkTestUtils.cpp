/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "VkTestUtils.h"

#ifdef SK_VULKAN

#include "SkAutoMalloc.h"
#include "vk/GrVkBackendContext.h"
#include "vk/GrVkExtensions.h"
#include "../ports/SkOSLibrary.h"

namespace sk_gpu_test {

bool LoadVkLibraryAndGetProcAddrFuncs(PFN_vkGetInstanceProcAddr* instProc,
                                                   PFN_vkGetDeviceProcAddr* devProc) {
#ifdef SK_MOLTENVK
    // MoltenVK is a statically linked framework, so there is no Vulkan library to load.
    *instProc = &vkGetInstanceProcAddr;
    *devProc = &vkGetDeviceProcAddr;
    return true;
#else
    static void* vkLib = nullptr;
    static PFN_vkGetInstanceProcAddr localInstProc = nullptr;
    static PFN_vkGetDeviceProcAddr localDevProc = nullptr;
    if (!vkLib) {
#if defined _WIN32
        vkLib = DynamicLoadLibrary("vulkan-1.dll");
#else
        vkLib = DynamicLoadLibrary("libvulkan.so");
#endif
        if (!vkLib) {
            return false;
        }
        localInstProc = (PFN_vkGetInstanceProcAddr) GetProcedureAddress(vkLib,
                                                                        "vkGetInstanceProcAddr");
        localDevProc = (PFN_vkGetDeviceProcAddr) GetProcedureAddress(vkLib,
                                                                     "vkGetDeviceProcAddr");
    }
    if (!localInstProc || !localDevProc) {
        return false;
    }
    *instProc = localInstProc;
    *devProc = localDevProc;
    return true;
#endif
}

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
        if (device != VK_NULL_HANDLE) {                                        \
            destroy_instance(getProc, inst, debugCallback, hasDebugExtension); \
        }                                                                      \
        return false;                                                          \
    }

#ifdef SK_ENABLE_VK_LAYERS
VKAPI_ATTR VkBool32 VKAPI_CALL DebugReportCallback(
    VkDebugReportFlagsEXT       flags,
    VkDebugReportObjectTypeEXT  objectType,
    uint64_t                    object,
    size_t                      location,
    int32_t                     messageCode,
    const char*                 pLayerPrefix,
    const char*                 pMessage,
    void*                       pUserData) {
    if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
        SkDebugf("Vulkan error [%s]: code: %d: %s\n", pLayerPrefix, messageCode, pMessage);
        return VK_TRUE; // skip further layers
    } else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
        SkDebugf("Vulkan warning [%s]: code: %d: %s\n", pLayerPrefix, messageCode, pMessage);
    } else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
        SkDebugf("Vulkan perf warning [%s]: code: %d: %s\n", pLayerPrefix, messageCode, pMessage);
    } else {
        SkDebugf("Vulkan info/debug [%s]: code: %d: %s\n", pLayerPrefix, messageCode, pMessage);
    }
    return VK_FALSE;
}
#endif

#define ACQUIRE_VK_PROC_LOCAL(name, instance, device)                          \
    PFN_vk##name grVk##name =                                                  \
        reinterpret_cast<PFN_vk##name>(getProc("vk" #name, instance, device)); \
    if (grVk##name == nullptr) {                                               \
        SkDebugf("Function ptr for vk%s could not be acquired\n", #name);      \
        return;                                                                \
    }

static void destroy_instance(GrVkInterface::GetProc getProc, VkInstance inst,
                             VkDebugReportCallbackEXT* debugCallback,
                             bool hasDebugExtension) {
    if (hasDebugExtension && *debugCallback != VK_NULL_HANDLE) {
        ACQUIRE_VK_PROC_LOCAL(DestroyDebugReportCallbackEXT, inst, VK_NULL_HANDLE);
        grVkDestroyDebugReportCallbackEXT(inst, *debugCallback, nullptr);
        *debugCallback = VK_NULL_HANDLE;
    }
    ACQUIRE_VK_PROC_LOCAL(DestroyInstance, inst, VK_NULL_HANDLE);
    grVkDestroyInstance(inst, nullptr);
}

bool CreateVkBackendContext(const GrVkInterface::GetInstanceProc& getInstanceProc,
                            const GrVkInterface::GetDeviceProc& getDeviceProc,
                            GrVkBackendContext* ctx,
                            VkDebugReportCallbackEXT* debugCallback,
                            uint32_t* presentQueueIndexPtr,
                            CanPresentFn canPresent) {
    auto getProc = [getInstanceProc, getDeviceProc](const char* proc_name,
                                                    VkInstance instance, VkDevice device) {
        if (device != VK_NULL_HANDLE) {
            return getDeviceProc(device, proc_name);
        }
        return getInstanceProc(instance, proc_name);
    };

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
    bool hasDebugExtension = false;
#ifdef SK_ENABLE_VK_LAYERS
    for (size_t i = 0; i < SK_ARRAY_COUNT(kDebugLayerNames); ++i) {
        if (extensions.hasInstanceLayer(kDebugLayerNames[i])) {
            instanceLayerNames.push_back(kDebugLayerNames[i]);
        }
    }
    if (extensions.hasInstanceExtension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME)) {
        instanceExtensionNames.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        extensionFlags |= kEXT_debug_report_GrVkExtensionFlag;
        hasDebugExtension = true;
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
        return false;
    }

#ifdef SK_ENABLE_VK_LAYERS
    *debugCallback = VK_NULL_HANDLE;
    for (int i = 0; i < instanceExtensionNames.count() && !hasDebugExtension; ++i) {
        if (!strcmp(instanceExtensionNames[i], VK_EXT_DEBUG_REPORT_EXTENSION_NAME)) {
            hasDebugExtension = true;
        }
    }
    if (hasDebugExtension) {
        // Setup callback creation information
        VkDebugReportCallbackCreateInfoEXT callbackCreateInfo;
        callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
        callbackCreateInfo.pNext = nullptr;
        callbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
                                   VK_DEBUG_REPORT_WARNING_BIT_EXT |
                                   // VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
                                   // VK_DEBUG_REPORT_DEBUG_BIT_EXT |
                                   VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
        callbackCreateInfo.pfnCallback = &DebugReportCallback;
        callbackCreateInfo.pUserData = nullptr;

        ACQUIRE_VK_PROC(CreateDebugReportCallbackEXT, inst, VK_NULL_HANDLE);
        // Register the callback
        grVkCreateDebugReportCallbackEXT(inst, &callbackCreateInfo, nullptr, debugCallback);
    }
#endif

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
        destroy_instance(getProc, inst, debugCallback, hasDebugExtension);
        return false;
    }
    if (!gpuCount) {
        SkDebugf("vkEnumeratePhysicalDevices returned no supported devices.\n");
        destroy_instance(getProc, inst, debugCallback, hasDebugExtension);
        return false;
    }
    // Just returning the first physical device instead of getting the whole array.
    // TODO: find best match for our needs
    gpuCount = 1;
    err = grVkEnumeratePhysicalDevices(inst, &gpuCount, &physDev);
    // VK_INCOMPLETE is returned when the count we provide is less than the total device count.
    if (err && VK_INCOMPLETE != err) {
        SkDebugf("vkEnumeratePhysicalDevices failed: %d\n", err);
        destroy_instance(getProc, inst, debugCallback, hasDebugExtension);
        return false;
    }

    // query to get the initial queue props size
    uint32_t queueCount;
    grVkGetPhysicalDeviceQueueFamilyProperties(physDev, &queueCount, nullptr);
    if (!queueCount) {
        SkDebugf("vkGetPhysicalDeviceQueueFamilyProperties returned no queues.\n");
        destroy_instance(getProc, inst, debugCallback, hasDebugExtension);
        return false;
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
        destroy_instance(getProc, inst, debugCallback, hasDebugExtension);
        return false;
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
            destroy_instance(getProc, inst, debugCallback, hasDebugExtension);
            return false;
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
        destroy_instance(getProc, inst, debugCallback, hasDebugExtension);
        return false;
    }

    VkQueue queue;
    grVkGetDeviceQueue(device, graphicsQueueIndex, 0, &queue);

    ctx->fInstance = inst;
    ctx->fPhysicalDevice = physDev;
    ctx->fDevice = device;
    ctx->fQueue = queue;
    ctx->fGraphicsQueueIndex = graphicsQueueIndex;
    ctx->fMinAPIVersion = kGrVkMinimumVersion;
    ctx->fExtensions = extensionFlags;
    ctx->fFeatures = featureFlags;
    ctx->fInterface = nullptr;
    ctx->fGetProc = getProc;
    ctx->fOwnsInstanceAndDevice = false;

    return true;
}

}

#endif
