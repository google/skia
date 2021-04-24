/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SK_VULKAN
#define SK_VULKAN
#endif

#include <tuple>

#include <android/native_window_jni.h>
#include <android/log.h>
#include <jni.h>

#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrBackendSemaphore.h"
#include "include/gpu/vk/GrVkBackendContext.h"
#include "include/gpu/vk/GrVkExtensions.h"
#include "include/gpu/vk/GrVkVulkan.h"
#include "modules/androidkit/src/Surface.h"
#include "src/core/SkAutoMalloc.h"
#include "src/gpu/vk/GrVkImage.h"
#include "src/gpu/vk/GrVkInterface.h"
#include "src/gpu/vk/GrVkUtil.h"
#include "src/ports/SkOSLibrary.h"
#include "tools/gpu/vk/GrVulkanDefines.h"

namespace {

#define ACQUIRE_VK_PROC_NOCHECK(name, instance, device) \
    PFN_vk##name grVk##name = reinterpret_cast<PFN_vk##name>(getProc("vk" #name, instance, device))

#define ACQUIRE_VK_PROC(name, instance, device)                                    \
    PFN_vk##name grVk##name =                                                      \
            reinterpret_cast<PFN_vk##name>(getProc("vk" #name, instance, device)); \
    do {                                                                           \
        if (grVk##name == nullptr) {                                               \
            SkDebugf("Function ptr for vk%s could not be acquired\n", #name);      \
            if (device != VK_NULL_HANDLE) {                                        \
                destroy_instance(getProc, inst, debugCallback, hasDebugExtension); \
            }                                                                      \
            return false;                                                          \
        }                                                                          \
    } while (0)

#define ACQUIRE_VK_PROC_LOCAL(name, instance, device)                              \
    PFN_vk##name grVk##name =                                                      \
            reinterpret_cast<PFN_vk##name>(getProc("vk" #name, instance, device)); \
    do {                                                                           \
        if (grVk##name == nullptr) {                                               \
            SkDebugf("Function ptr for vk%s could not be acquired\n", #name);      \
            return false;                                                                \
        }                                                                          \
    } while (0)

#define GET_PROC_LOCAL(F, inst, device) PFN_vk ## F F = (PFN_vk ## F) getProc("vk" #F, inst, device)

static std::tuple<PFN_vkGetInstanceProcAddr, PFN_vkGetDeviceProcAddr>
LoadVkLibraryAndGetProcAddrFuncs() {
    static void*                     vkLib           = nullptr;
    static PFN_vkGetInstanceProcAddr vkLocalInstProc = nullptr;
    static PFN_vkGetDeviceProcAddr   vkLocalDevProc  = nullptr;

    if (!vkLib) {
        vkLib = SkLoadDynamicLibrary("libvulkan.so");
        if (vkLib) {
            vkLocalInstProc = reinterpret_cast<PFN_vkGetInstanceProcAddr>(
                                SkGetProcedureAddress(vkLib, "vkGetInstanceProcAddr"));
            vkLocalDevProc = reinterpret_cast<PFN_vkGetDeviceProcAddr>(
                                SkGetProcedureAddress(vkLib, "vkGetDeviceProcAddr"));
        }
    }

    return std::make_tuple(vkLocalInstProc, vkLocalDevProc);
}

#ifdef SK_ENABLE_VK_LAYERS
const char* kDebugLayerNames[] = {
    // single merged layer
    "VK_LAYER_KHRONOS_validation",
    // not included in standard_validation
    //"VK_LAYER_LUNARG_api_dump",
    //"VK_LAYER_LUNARG_vktrace",
    //"VK_LAYER_LUNARG_screenshot",
};

static uint32_t remove_patch_version(uint32_t specVersion) {
    return (specVersion >> 12) << 12;
}

// Returns the index into layers array for the layer we want. Returns -1 if not supported.
static int should_include_debug_layer(const char* layerName,
                                       uint32_t layerCount, VkLayerProperties* layers,
                                       uint32_t version) {
    for (uint32_t i = 0; i < layerCount; ++i) {
        if (!strcmp(layerName, layers[i].layerName)) {
            // Since the layers intercept the vulkan calls and forward them on, we need to make sure
            // layer was written against a version that isn't older than the version of Vulkan we're
            // using so that it has all the api entry points.
            if (version <= remove_patch_version(layers[i].specVersion)) {
                return i;
            }
            return -1;
        }

    }
    return -1;
}

static void print_backtrace() {
#if defined(SK_BUILD_FOR_UNIX)
    void* stack[64];
    int count = backtrace(stack, SK_ARRAY_COUNT(stack));
    backtrace_symbols_fd(stack, count, 2);
#else
    // Please add implementations for other platforms.
#endif
}

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
        // See https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/1887
        if (strstr(pMessage, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-01521") ||
            strstr(pMessage, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-01522")) {
            return VK_FALSE;
        }
        // See https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/2171
        if (strstr(pMessage, "VUID-vkCmdDraw-None-02686") ||
            strstr(pMessage, "VUID-vkCmdDrawIndexed-None-02686")) {
            return VK_FALSE;
        }
        SkDebugf("Vulkan error [%s]: code: %d: %s\n", pLayerPrefix, messageCode, pMessage);
        print_backtrace();
        SkDEBUGFAIL("Vulkan debug layer error");
        return VK_TRUE; // skip further layers
    } else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
        SkDebugf("Vulkan warning [%s]: code: %d: %s\n", pLayerPrefix, messageCode, pMessage);
        print_backtrace();
    } else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
        SkDebugf("Vulkan perf warning [%s]: code: %d: %s\n", pLayerPrefix, messageCode, pMessage);
        print_backtrace();
    } else {
        SkDebugf("Vulkan info/debug [%s]: code: %d: %s\n", pLayerPrefix, messageCode, pMessage);
    }
    return VK_FALSE;
}
#endif

static bool init_instance_extensions_and_layers(GrVkGetProc getProc,
                                                uint32_t specVersion,
                                                SkTArray<VkExtensionProperties>* instanceExtensions,
                                                SkTArray<VkLayerProperties>* instanceLayers) {
    if (getProc == nullptr) {
        return false;
    }

    GET_PROC_LOCAL(EnumerateInstanceExtensionProperties, VK_NULL_HANDLE, VK_NULL_HANDLE);
    GET_PROC_LOCAL(EnumerateInstanceLayerProperties, VK_NULL_HANDLE, VK_NULL_HANDLE);

    if (!EnumerateInstanceExtensionProperties ||
        !EnumerateInstanceLayerProperties) {
        return false;
    }

    VkResult res;
    uint32_t layerCount = 0;
#ifdef SK_ENABLE_VK_LAYERS
    // instance layers
    res = EnumerateInstanceLayerProperties(&layerCount, nullptr);
    if (VK_SUCCESS != res) {
        return false;
    }
    VkLayerProperties* layers = new VkLayerProperties[layerCount];
    res = EnumerateInstanceLayerProperties(&layerCount, layers);
    if (VK_SUCCESS != res) {
        delete[] layers;
        return false;
    }

    uint32_t nonPatchVersion = remove_patch_version(specVersion);
    for (size_t i = 0; i < SK_ARRAY_COUNT(kDebugLayerNames); ++i) {
        int idx = should_include_debug_layer(kDebugLayerNames[i], layerCount, layers,
                                             nonPatchVersion);
        if (idx != -1) {
            instanceLayers->push_back() = layers[idx];
        }
    }
    delete[] layers;
#endif

    // instance extensions
    // via Vulkan implementation and implicitly enabled layers
    uint32_t extensionCount = 0;
    res = EnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    if (VK_SUCCESS != res) {
        return false;
    }
    VkExtensionProperties* extensions = new VkExtensionProperties[extensionCount];
    res = EnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions);
    if (VK_SUCCESS != res) {
        delete[] extensions;
        return false;
    }
    for (uint32_t i = 0; i < extensionCount; ++i) {
        instanceExtensions->push_back() = extensions[i];
    }
    delete [] extensions;

    // via explicitly enabled layers
    layerCount = instanceLayers->count();
    for (uint32_t layerIndex = 0; layerIndex < layerCount; ++layerIndex) {
        uint32_t extensionCount = 0;
        res = EnumerateInstanceExtensionProperties((*instanceLayers)[layerIndex].layerName,
                                                   &extensionCount, nullptr);
        if (VK_SUCCESS != res) {
            return false;
        }
        VkExtensionProperties* extensions = new VkExtensionProperties[extensionCount];
        res = EnumerateInstanceExtensionProperties((*instanceLayers)[layerIndex].layerName,
                                                   &extensionCount, extensions);
        if (VK_SUCCESS != res) {
            delete[] extensions;
            return false;
        }
        for (uint32_t i = 0; i < extensionCount; ++i) {
            instanceExtensions->push_back() = extensions[i];
        }
        delete[] extensions;
    }

    return true;
}

static bool init_device_extensions_and_layers(GrVkGetProc getProc, uint32_t specVersion,
                                              VkInstance inst, VkPhysicalDevice physDev,
                                              SkTArray<VkExtensionProperties>* deviceExtensions,
                                              SkTArray<VkLayerProperties>* deviceLayers) {
    if (getProc == nullptr) {
        return false;
    }

    GET_PROC_LOCAL(EnumerateDeviceExtensionProperties, inst, VK_NULL_HANDLE);
    GET_PROC_LOCAL(EnumerateDeviceLayerProperties, inst, VK_NULL_HANDLE);

    if (!EnumerateDeviceExtensionProperties ||
        !EnumerateDeviceLayerProperties) {
        return false;
    }

    VkResult res;
    // device layers
    uint32_t layerCount = 0;
#ifdef SK_ENABLE_VK_LAYERS
    res = EnumerateDeviceLayerProperties(physDev, &layerCount, nullptr);
    if (VK_SUCCESS != res) {
        return false;
    }
    VkLayerProperties* layers = new VkLayerProperties[layerCount];
    res = EnumerateDeviceLayerProperties(physDev, &layerCount, layers);
    if (VK_SUCCESS != res) {
        delete[] layers;
        return false;
    }

    uint32_t nonPatchVersion = remove_patch_version(specVersion);
    for (size_t i = 0; i < SK_ARRAY_COUNT(kDebugLayerNames); ++i) {
        int idx = should_include_debug_layer(kDebugLayerNames[i], layerCount, layers,
                                             nonPatchVersion);
        if (idx != -1) {
            deviceLayers->push_back() = layers[idx];
        }
    }
    delete[] layers;
#endif

    // device extensions
    // via Vulkan implementation and implicitly enabled layers
    uint32_t extensionCount = 0;
    res = EnumerateDeviceExtensionProperties(physDev, nullptr, &extensionCount, nullptr);
    if (VK_SUCCESS != res) {
        return false;
    }
    VkExtensionProperties* extensions = new VkExtensionProperties[extensionCount];
    res = EnumerateDeviceExtensionProperties(physDev, nullptr, &extensionCount, extensions);
    if (VK_SUCCESS != res) {
        delete[] extensions;
        return false;
    }
    for (uint32_t i = 0; i < extensionCount; ++i) {
        deviceExtensions->push_back() = extensions[i];
    }
    delete[] extensions;

    // via explicitly enabled layers
    layerCount = deviceLayers->count();
    for (uint32_t layerIndex = 0; layerIndex < layerCount; ++layerIndex) {
        uint32_t extensionCount = 0;
        res = EnumerateDeviceExtensionProperties(physDev,
            (*deviceLayers)[layerIndex].layerName,
            &extensionCount, nullptr);
        if (VK_SUCCESS != res) {
            return false;
        }
        VkExtensionProperties* extensions = new VkExtensionProperties[extensionCount];
        res = EnumerateDeviceExtensionProperties(physDev,
            (*deviceLayers)[layerIndex].layerName,
            &extensionCount, extensions);
        if (VK_SUCCESS != res) {
            delete[] extensions;
            return false;
        }
        for (uint32_t i = 0; i < extensionCount; ++i) {
            deviceExtensions->push_back() = extensions[i];
        }
        delete[] extensions;
    }

    return true;
}

static bool setup_features(GrVkGetProc getProc, VkInstance inst, VkPhysicalDevice physDev,
                           uint32_t physDeviceVersion, GrVkExtensions* extensions,
                           VkPhysicalDeviceFeatures2* features, bool isProtected) {
    SkASSERT(physDeviceVersion >= VK_MAKE_VERSION(1, 1, 0) ||
             extensions->hasExtension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, 1));

    // Setup all extension feature structs we may want to use.
    void** tailPNext = &features->pNext;

    // If |isProtected| is given, attach that first
    VkPhysicalDeviceProtectedMemoryFeatures* protectedMemoryFeatures = nullptr;
    if (isProtected) {
        SkASSERT(physDeviceVersion >= VK_MAKE_VERSION(1, 1, 0));
        protectedMemoryFeatures =
          (VkPhysicalDeviceProtectedMemoryFeatures*)sk_malloc_throw(
              sizeof(VkPhysicalDeviceProtectedMemoryFeatures));
        protectedMemoryFeatures->sType =
          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES;
        protectedMemoryFeatures->pNext = nullptr;
        *tailPNext = protectedMemoryFeatures;
        tailPNext = &protectedMemoryFeatures->pNext;
    }

    VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT* blend = nullptr;
    if (extensions->hasExtension(VK_EXT_BLEND_OPERATION_ADVANCED_EXTENSION_NAME, 2)) {
        blend = (VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT*) sk_malloc_throw(
                sizeof(VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT));
        blend->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT;
        blend->pNext = nullptr;
        *tailPNext = blend;
        tailPNext = &blend->pNext;
    }

    VkPhysicalDeviceSamplerYcbcrConversionFeatures* ycbcrFeature = nullptr;
    if (physDeviceVersion >= VK_MAKE_VERSION(1, 1, 0) ||
        extensions->hasExtension(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME, 1)) {
        ycbcrFeature = (VkPhysicalDeviceSamplerYcbcrConversionFeatures*) sk_malloc_throw(
                sizeof(VkPhysicalDeviceSamplerYcbcrConversionFeatures));
        ycbcrFeature->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES;
        ycbcrFeature->pNext = nullptr;
        ycbcrFeature->samplerYcbcrConversion = VK_TRUE;
        *tailPNext = ycbcrFeature;
        tailPNext = &ycbcrFeature->pNext;
    }

    if (physDeviceVersion >= VK_MAKE_VERSION(1, 1, 0)) {
        ACQUIRE_VK_PROC_LOCAL(GetPhysicalDeviceFeatures2, inst, VK_NULL_HANDLE);
        grVkGetPhysicalDeviceFeatures2(physDev, features);
    } else {
        SkASSERT(extensions->hasExtension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
                                          1));
        ACQUIRE_VK_PROC_LOCAL(GetPhysicalDeviceFeatures2KHR, inst, VK_NULL_HANDLE);
        grVkGetPhysicalDeviceFeatures2KHR(physDev, features);
    }

    if (isProtected) {
        if (!protectedMemoryFeatures->protectedMemory) {
            return false;
        }
    }
    return true;
    // If we want to disable any extension features do so here.
}

static bool destroy_instance(GrVkGetProc getProc, VkInstance inst,
                             VkDebugReportCallbackEXT* debugCallback,
                             bool hasDebugExtension) {
    if (hasDebugExtension && *debugCallback != VK_NULL_HANDLE) {
        ACQUIRE_VK_PROC_LOCAL(DestroyDebugReportCallbackEXT, inst, VK_NULL_HANDLE);
        grVkDestroyDebugReportCallbackEXT(inst, *debugCallback, nullptr);
        *debugCallback = VK_NULL_HANDLE;
    }
    ACQUIRE_VK_PROC_LOCAL(DestroyInstance, inst, VK_NULL_HANDLE);
    grVkDestroyInstance(inst, nullptr);
    return true;
}

using CanPresentFn = std::function<bool(VkInstance, VkPhysicalDevice,
                                        uint32_t queueFamilyIndex)>;
bool CreateVkBackendContext(GrVkGetProc getProc,
                            GrVkBackendContext* ctx,
                            GrVkExtensions* extensions,
                            VkPhysicalDeviceFeatures2* features,
                            VkDebugReportCallbackEXT* debugCallback,
                            uint32_t* presentQueueIndexPtr,
                            CanPresentFn canPresent,
                            bool isProtected) {
    VkResult err;

    ACQUIRE_VK_PROC_NOCHECK(EnumerateInstanceVersion, VK_NULL_HANDLE, VK_NULL_HANDLE);
    uint32_t instanceVersion = 0;
    if (!grVkEnumerateInstanceVersion) {
        instanceVersion = VK_MAKE_VERSION(1, 0, 0);
    } else {
        err = grVkEnumerateInstanceVersion(&instanceVersion);
        if (err) {
            SkDebugf("failed to enumerate instance version. Err: %d\n", err);
            return false;
        }
    }
    SkASSERT(instanceVersion >= VK_MAKE_VERSION(1, 0, 0));
    if (isProtected && instanceVersion < VK_MAKE_VERSION(1, 1, 0)) {
        SkDebugf("protected requires vk instance version 1.1\n");
        return false;
    }

    uint32_t apiVersion = VK_MAKE_VERSION(1, 0, 0);
    if (instanceVersion >= VK_MAKE_VERSION(1, 1, 0)) {
        // If the instance version is 1.0 we must have the apiVersion also be 1.0. However, if the
        // instance version is 1.1 or higher, we can set the apiVersion to be whatever the highest
        // api we may use in skia (technically it can be arbitrary). So for now we set it to 1.1
        // since that is the highest vulkan version.
        apiVersion = VK_MAKE_VERSION(1, 1, 0);
    }

    instanceVersion = std::min(instanceVersion, apiVersion);

    VkPhysicalDevice physDev;
    VkDevice device;
    VkInstance inst;

    const VkApplicationInfo app_info = {
        VK_STRUCTURE_TYPE_APPLICATION_INFO, // sType
        nullptr,                            // pNext
        "vktest",                           // pApplicationName
        0,                                  // applicationVersion
        "vktest",                           // pEngineName
        0,                                  // engineVerison
        apiVersion,                         // apiVersion
    };

    SkTArray<VkLayerProperties> instanceLayers;
    SkTArray<VkExtensionProperties> instanceExtensions;

    if (!init_instance_extensions_and_layers(getProc, instanceVersion,
                                             &instanceExtensions,
                                             &instanceLayers)) {
        return false;
    }

    SkTArray<const char*> instanceLayerNames;
    SkTArray<const char*> instanceExtensionNames;
    for (int i = 0; i < instanceLayers.count(); ++i) {
        instanceLayerNames.push_back(instanceLayers[i].layerName);
    }
    for (int i = 0; i < instanceExtensions.count(); ++i) {
        if (strncmp(instanceExtensions[i].extensionName, "VK_KHX", 6) != 0) {
            instanceExtensionNames.push_back(instanceExtensions[i].extensionName);
        }
    }

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

    bool hasDebugExtension = false;

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

    ACQUIRE_VK_PROC(EnumeratePhysicalDevices, inst, VK_NULL_HANDLE);
    ACQUIRE_VK_PROC(GetPhysicalDeviceProperties, inst, VK_NULL_HANDLE);
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

    VkPhysicalDeviceProperties physDeviceProperties;
    grVkGetPhysicalDeviceProperties(physDev, &physDeviceProperties);
    int physDeviceVersion = std::min(physDeviceProperties.apiVersion, apiVersion);

    if (isProtected && physDeviceVersion < VK_MAKE_VERSION(1, 1, 0)) {
        SkDebugf("protected requires vk physical device version 1.1\n");
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

    SkTArray<VkLayerProperties> deviceLayers;
    SkTArray<VkExtensionProperties> deviceExtensions;
    if (!init_device_extensions_and_layers(getProc, physDeviceVersion,
                                           inst, physDev,
                                           &deviceExtensions,
                                           &deviceLayers)) {
        destroy_instance(getProc, inst, debugCallback, hasDebugExtension);
        return false;
    }

    SkTArray<const char*> deviceLayerNames;
    SkTArray<const char*> deviceExtensionNames;
    for (int i = 0; i < deviceLayers.count(); ++i) {
        deviceLayerNames.push_back(deviceLayers[i].layerName);
    }

    // We can't have both VK_KHR_buffer_device_address and VK_EXT_buffer_device_address as
    // extensions. So see if we have the KHR version and if so don't push back the EXT version in
    // the next loop.
    bool hasKHRBufferDeviceAddress = false;
    for (int i = 0; i < deviceExtensions.count(); ++i) {
        if (!strcmp(deviceExtensions[i].extensionName, "VK_KHR_buffer_device_address")) {
            hasKHRBufferDeviceAddress = true;
            break;
        }
    }

    for (int i = 0; i < deviceExtensions.count(); ++i) {
        // Don't use experimental extensions since they typically don't work with debug layers and
        // often are missing dependecy requirements for other extensions. Additionally, these are
        // often left behind in the driver even after they've been promoted to real extensions.
        if (0 != strncmp(deviceExtensions[i].extensionName, "VK_KHX", 6) &&
            0 != strncmp(deviceExtensions[i].extensionName, "VK_NVX", 6)) {

            // This is an nvidia extension that isn't supported by the debug layers so we get lots
            // of warnings. We don't actually use it, so it is easiest to just not enable it.
            if (0 == strcmp(deviceExtensions[i].extensionName, "VK_NV_low_latency")) {
                continue;
            }

            if (!hasKHRBufferDeviceAddress ||
                0 != strcmp(deviceExtensions[i].extensionName, "VK_EXT_buffer_device_address")) {
                deviceExtensionNames.push_back(deviceExtensions[i].extensionName);
            }
        }
    }

    extensions->init(getProc, inst, physDev,
                     (uint32_t) instanceExtensionNames.count(),
                     instanceExtensionNames.begin(),
                     (uint32_t) deviceExtensionNames.count(),
                     deviceExtensionNames.begin());

    memset(features, 0, sizeof(VkPhysicalDeviceFeatures2));
    features->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    features->pNext = nullptr;

    VkPhysicalDeviceFeatures* deviceFeatures = &features->features;
    void* pointerToFeatures = nullptr;
    if (physDeviceVersion >= VK_MAKE_VERSION(1, 1, 0) ||
        extensions->hasExtension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, 1)) {
        if (!setup_features(getProc, inst, physDev, physDeviceVersion, extensions, features,
                          isProtected)) {
            destroy_instance(getProc, inst, debugCallback, hasDebugExtension);
            return false;
        }

        // If we set the pNext of the VkDeviceCreateInfo to our VkPhysicalDeviceFeatures2 struct,
        // the device creation will use that instead of the ppEnabledFeatures.
        pointerToFeatures = features;
    } else {
        grVkGetPhysicalDeviceFeatures(physDev, deviceFeatures);
    }

    // this looks like it would slow things down,
    // and we can't depend on it on all platforms
    deviceFeatures->robustBufferAccess = VK_FALSE;

    VkDeviceQueueCreateFlags flags = isProtected ? VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT : 0;
    float queuePriorities[1] = { 0.0 };
    // Here we assume no need for swapchain queue
    // If one is needed, the client will need its own setup code
    const VkDeviceQueueCreateInfo queueInfo[2] = {
        {
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, // sType
            nullptr,                                    // pNext
            flags,                                      // VkDeviceQueueCreateFlags
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
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,        // sType
        pointerToFeatures,                           // pNext
        0,                                           // VkDeviceCreateFlags
        queueInfoCount,                              // queueCreateInfoCount
        queueInfo,                                   // pQueueCreateInfos
        (uint32_t) deviceLayerNames.count(),         // layerCount
        deviceLayerNames.begin(),                    // ppEnabledLayerNames
        (uint32_t) deviceExtensionNames.count(),     // extensionCount
        deviceExtensionNames.begin(),                // ppEnabledExtensionNames
        pointerToFeatures ? nullptr : deviceFeatures // ppEnabledFeatures
    };

    {
#if defined(SK_ENABLE_SCOPED_LSAN_SUPPRESSIONS)
        // skia:8712
        __lsan::ScopedDisabler lsanDisabler;
#endif
        err = grVkCreateDevice(physDev, &deviceInfo, nullptr, &device);
    }
    if (err) {
        SkDebugf("CreateDevice failed: %d\n", err);
        destroy_instance(getProc, inst, debugCallback, hasDebugExtension);
        return false;
    }

    VkQueue queue;
    if (isProtected) {
        ACQUIRE_VK_PROC(GetDeviceQueue2, inst, device);
        SkASSERT(grVkGetDeviceQueue2 != nullptr);
        VkDeviceQueueInfo2 queue_info2 = {
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2,          // sType
            nullptr,                                        // pNext
            VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT,           // flags
            graphicsQueueIndex,                             // queueFamilyIndex
            0                                               // queueIndex
        };
        grVkGetDeviceQueue2(device, &queue_info2, &queue);
    } else {
        grVkGetDeviceQueue(device, graphicsQueueIndex, 0, &queue);
    }

    ctx->fInstance = inst;
    ctx->fPhysicalDevice = physDev;
    ctx->fDevice = device;
    ctx->fQueue = queue;
    ctx->fGraphicsQueueIndex = graphicsQueueIndex;
    ctx->fMaxAPIVersion = apiVersion;
    ctx->fVkExtensions = extensions;
    ctx->fDeviceFeatures2 = features;
    ctx->fGetProc = getProc;
    ctx->fOwnsInstanceAndDevice = false;
    ctx->fProtectedContext = isProtected ? GrProtected::kYes : GrProtected::kNo;

    return true;
}

void FreeVulkanFeaturesStructs(const VkPhysicalDeviceFeatures2* features) {
    // All Vulkan structs that could be part of the features chain will start with the
    // structure type followed by the pNext pointer. We cast to the CommonVulkanHeader
    // so we can get access to the pNext for the next struct.
    struct CommonVulkanHeader {
        VkStructureType sType;
        void*           pNext;
    };

    void* pNext = features->pNext;
    while (pNext) {
        void* current = pNext;
        pNext = static_cast<CommonVulkanHeader*>(current)->pNext;
        sk_free(current);
    }
}

class VKSurface final : public Surface {
public:
    explicit VKSurface(ANativeWindow* win)
        : fWindow(win)
    {
        SkASSERT(fWindow);

        const auto [ vk_inst_proc, vk_dev_proc ] = LoadVkLibraryAndGetProcAddrFuncs();

        if (!vk_inst_proc || !vk_dev_proc) {
            __android_log_print(ANDROID_LOG_ERROR, "AndroidKit", "Failed to load libvulkan.so");
            return;
        }

        auto getProc = [inst_proc = vk_inst_proc, dev_proc = vk_dev_proc]
                           (const char* proc_name, VkInstance instance, VkDevice device) {
            return device != VK_NULL_HANDLE
                ? dev_proc(device, proc_name)
                : inst_proc(instance, proc_name);
        };

        auto canPresent = [](VkInstance, VkPhysicalDevice, uint32_t) { return true; };

        GrVkBackendContext backendContext;
        GrVkExtensions extensions;
        VkPhysicalDeviceFeatures2 features;
        if (!CreateVkBackendContext(getProc, &backendContext, &extensions, &features,
                                    &fVKDebugCallback, &fVKPresentQueueIndex, canPresent, false)) {
            FreeVulkanFeaturesStructs(&features);
            return;
        }

        if (!extensions.hasExtension(VK_KHR_SURFACE_EXTENSION_NAME, 25) ||
            !extensions.hasExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME, 68)) {
            FreeVulkanFeaturesStructs(&features);
            return;
        }

        fVKInstance = backendContext.fInstance;
        fVKPhysicalDevice = backendContext.fPhysicalDevice;
        fVKDevice = backendContext.fDevice;
        fVKGraphicsQueueIndex = backendContext.fGraphicsQueueIndex;
        fVKGraphicsQueue = backendContext.fQueue;

        PFN_vkGetPhysicalDeviceProperties localGetPhysicalDeviceProperties =
                reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(
                        backendContext.fGetProc("vkGetPhysicalDeviceProperties",
                                                backendContext.fInstance,
                                                VK_NULL_HANDLE));
        if (!localGetPhysicalDeviceProperties) {
            FreeVulkanFeaturesStructs(&features);
            return;
        }

        VkPhysicalDeviceProperties physDeviceProperties;
        localGetPhysicalDeviceProperties(backendContext.fPhysicalDevice, &physDeviceProperties);
        uint32_t physDevVersion = physDeviceProperties.apiVersion;

        fInterface.reset(new GrVkInterface(backendContext.fGetProc, fVKInstance, fVKDevice,
                                           backendContext.fInstanceVersion, physDevVersion,
                                           &extensions));

        #define GET_PROC(F) f ## F = (PFN_vk ## F) vk_inst_proc(fVKInstance, "vk" #F)
        #define GET_DEV_PROC(F) f ## F = (PFN_vk ## F) vk_dev_proc(fVKDevice, "vk" #F)

        GET_PROC(DestroyInstance);
        if (fVKDebugCallback != VK_NULL_HANDLE) {
            GET_PROC(DestroyDebugReportCallbackEXT);
        }
        GET_PROC(DestroySurfaceKHR);
        GET_PROC(GetPhysicalDeviceSurfaceSupportKHR);
        GET_PROC(GetPhysicalDeviceSurfaceCapabilitiesKHR);
        GET_PROC(GetPhysicalDeviceSurfaceFormatsKHR);
        GET_PROC(GetPhysicalDeviceSurfacePresentModesKHR);
        GET_DEV_PROC(DeviceWaitIdle);
        GET_DEV_PROC(QueueWaitIdle);
        GET_DEV_PROC(DestroyDevice);
        GET_DEV_PROC(CreateSwapchainKHR);
        GET_DEV_PROC(DestroySwapchainKHR);
        GET_DEV_PROC(GetSwapchainImagesKHR);
        GET_DEV_PROC(AcquireNextImageKHR);
        GET_DEV_PROC(QueuePresentKHR);
        GET_DEV_PROC(GetDeviceQueue);

        auto createVkSurface = [window = fWindow, instProc = vk_inst_proc]
                (VkInstance instance) -> VkSurfaceKHR {
            PFN_vkCreateAndroidSurfaceKHR createAndroidSurfaceKHR =
                    (PFN_vkCreateAndroidSurfaceKHR) instProc(instance, "vkCreateAndroidSurfaceKHR");

            if (!window) {
                return VK_NULL_HANDLE;
            }
            VkSurfaceKHR surface;

            VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo;
            memset(&surfaceCreateInfo, 0, sizeof(VkAndroidSurfaceCreateInfoKHR));
            surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
            surfaceCreateInfo.pNext = nullptr;
            surfaceCreateInfo.flags = 0;
            surfaceCreateInfo.window = window;

            VkResult res = createAndroidSurfaceKHR(instance, &surfaceCreateInfo,
                                                   nullptr, &surface);
            return (VK_SUCCESS == res) ? surface : VK_NULL_HANDLE;
        };

        // TODO
        GrContextOptions opts;
        fContext = GrDirectContext::MakeVulkan(backendContext, opts);

        fVKSurface = createVkSurface(fVKInstance);
        if (fVKSurface == VK_NULL_HANDLE) {
            this->destroyContext();
            FreeVulkanFeaturesStructs(&features);
            return;
        }

        VkBool32 supported;
        VkResult res = fGetPhysicalDeviceSurfaceSupportKHR(fVKPhysicalDevice,
                                                           fVKPresentQueueIndex,
                                                           fVKSurface, &supported);
        if (VK_SUCCESS != res) {
            this->destroyContext();
            FreeVulkanFeaturesStructs(&features);
            return;
        }

        if (!this->createSwapchain(-1, -1, fDisplayParams)) {
            this->destroyContext();
            FreeVulkanFeaturesStructs(&features);
            return;
        }

        // create presentQueue
        fGetDeviceQueue(fVKDevice, fVKPresentQueueIndex, 0, &fVKPresentQueue);
        FreeVulkanFeaturesStructs(&features);


        fSurface = this->getBackbufferSurface();
        __android_log_print(ANDROID_LOG_ERROR, "*** AK", "YAY surface: %p", fSurface.get());
    }

private:
    struct DisplayParams {
        DisplayParams()
            : fColorType(kN32_SkColorType)
            , fColorSpace(nullptr)
            , fMSAASampleCount(1)
            , fSurfaceProps(0, kUnknown_SkPixelGeometry)
            , fDisableVsync(false)
        {}

        SkColorType         fColorType;
        sk_sp<SkColorSpace> fColorSpace;
        int                 fMSAASampleCount;
        GrContextOptions    fGrContextOptions;
        SkSurfaceProps      fSurfaceProps;
        bool                fDisableVsync;
    };

    struct BackbufferInfo {
        uint32_t        fImageIndex;          // image this is associated with
        VkSemaphore     fRenderSemaphore;     // we wait on this for rendering to be done
    };

    bool createSwapchain(int width, int height, const DisplayParams& params) {
        // check for capabilities
        VkSurfaceCapabilitiesKHR caps;
        VkResult res = fGetPhysicalDeviceSurfaceCapabilitiesKHR(fVKPhysicalDevice,
                                                                fVKSurface, &caps);
        if (VK_SUCCESS != res) {
            return false;
        }

        uint32_t surfaceFormatCount;
        res = fGetPhysicalDeviceSurfaceFormatsKHR(fVKPhysicalDevice, fVKSurface,
                                                  &surfaceFormatCount, nullptr);
        if (VK_SUCCESS != res) {
            return false;
        }

        SkAutoMalloc surfaceFormatAlloc(surfaceFormatCount * sizeof(VkSurfaceFormatKHR));
        VkSurfaceFormatKHR* surfaceFormats = (VkSurfaceFormatKHR*)surfaceFormatAlloc.get();
        res = fGetPhysicalDeviceSurfaceFormatsKHR(fVKPhysicalDevice, fVKSurface,
                                                  &surfaceFormatCount, surfaceFormats);
        if (VK_SUCCESS != res) {
            return false;
        }

        uint32_t presentModeCount;
        res = fGetPhysicalDeviceSurfacePresentModesKHR(fVKPhysicalDevice, fVKSurface,
                                                       &presentModeCount, nullptr);
        if (VK_SUCCESS != res) {
            return false;
        }

        SkAutoMalloc presentModeAlloc(presentModeCount * sizeof(VkPresentModeKHR));
        VkPresentModeKHR* presentModes = (VkPresentModeKHR*)presentModeAlloc.get();
        res = fGetPhysicalDeviceSurfacePresentModesKHR(fVKPhysicalDevice, fVKSurface,
                                                       &presentModeCount, presentModes);
        if (VK_SUCCESS != res) {
            return false;
        }

        VkExtent2D extent = caps.currentExtent;
        // use the hints
        if (extent.width == (uint32_t)-1) {
            extent.width = width;
            extent.height = height;
        }

        // clamp width; to protect us from broken hints
        if (extent.width < caps.minImageExtent.width) {
            extent.width = caps.minImageExtent.width;
        } else if (extent.width > caps.maxImageExtent.width) {
            extent.width = caps.maxImageExtent.width;
        }
        // clamp height
        if (extent.height < caps.minImageExtent.height) {
            extent.height = caps.minImageExtent.height;
        } else if (extent.height > caps.maxImageExtent.height) {
            extent.height = caps.maxImageExtent.height;
        }

        fSize = {(int)extent.width, (int)extent.height};

        uint32_t imageCount = caps.minImageCount + 2;
        if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount) {
            // Application must settle for fewer images than desired:
            imageCount = caps.maxImageCount;
        }

        VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                       VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                       VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        SkASSERT((caps.supportedUsageFlags & usageFlags) == usageFlags);
        if (caps.supportedUsageFlags & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) {
            usageFlags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        }
        if (caps.supportedUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT) {
            usageFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;
        }
        SkASSERT(caps.supportedTransforms & caps.currentTransform);
        SkASSERT(caps.supportedCompositeAlpha & (VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR |
                                                 VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR));
        VkCompositeAlphaFlagBitsKHR composite_alpha =
            (caps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) ?
                                            VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR :
                                            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        // Pick our surface format.
        VkFormat surfaceFormat = VK_FORMAT_UNDEFINED;
        VkColorSpaceKHR colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        for (uint32_t i = 0; i < surfaceFormatCount; ++i) {
            VkFormat localFormat = surfaceFormats[i].format;
            if (GrVkFormatIsSupported(localFormat)) {
                surfaceFormat = localFormat;
                colorSpace = surfaceFormats[i].colorSpace;
                break;
            }
        }

        fDisplayParams = params;
        fSampleCount = std::max(1, params.fMSAASampleCount);
//        fStencilBits = 8;

        if (VK_FORMAT_UNDEFINED == surfaceFormat) {
            return false;
        }

        SkColorType colorType;
        switch (surfaceFormat) {
            case VK_FORMAT_R8G8B8A8_UNORM: // fall through
            case VK_FORMAT_R8G8B8A8_SRGB:
                colorType = kRGBA_8888_SkColorType;
                break;
            case VK_FORMAT_B8G8R8A8_UNORM: // fall through
                colorType = kBGRA_8888_SkColorType;
                break;
            default:
                return false;
        }

        // If mailbox mode is available, use it, as it is the lowest-latency non-
        // tearing mode. If not, fall back to FIFO which is always available.
        VkPresentModeKHR mode = VK_PRESENT_MODE_FIFO_KHR;
        bool hasImmediate = false;
        for (uint32_t i = 0; i < presentModeCount; ++i) {
            // use mailbox
            if (VK_PRESENT_MODE_MAILBOX_KHR == presentModes[i]) {
                mode = VK_PRESENT_MODE_MAILBOX_KHR;
            }
            if (VK_PRESENT_MODE_IMMEDIATE_KHR == presentModes[i]) {
                hasImmediate = true;
            }
        }
        if (params.fDisableVsync && hasImmediate) {
            mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
        }

        VkSwapchainCreateInfoKHR swapchainCreateInfo;
        memset(&swapchainCreateInfo, 0, sizeof(VkSwapchainCreateInfoKHR));
        swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainCreateInfo.surface = fVKSurface;
        swapchainCreateInfo.minImageCount = imageCount;
        swapchainCreateInfo.imageFormat = surfaceFormat;
        swapchainCreateInfo.imageColorSpace = colorSpace;
        swapchainCreateInfo.imageExtent = extent;
        swapchainCreateInfo.imageArrayLayers = 1;
        swapchainCreateInfo.imageUsage = usageFlags;

        uint32_t queueFamilies[] = { fVKGraphicsQueueIndex, fVKPresentQueueIndex };
        if (fVKGraphicsQueueIndex != fVKPresentQueueIndex) {
            swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapchainCreateInfo.queueFamilyIndexCount = 2;
            swapchainCreateInfo.pQueueFamilyIndices = queueFamilies;
        } else {
            swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            swapchainCreateInfo.queueFamilyIndexCount = 0;
            swapchainCreateInfo.pQueueFamilyIndices = nullptr;
        }

        swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        swapchainCreateInfo.compositeAlpha = composite_alpha;
        swapchainCreateInfo.presentMode = mode;
        swapchainCreateInfo.clipped = true;
        swapchainCreateInfo.oldSwapchain = fVKSwapchain;

        res = fCreateSwapchainKHR(fVKDevice, &swapchainCreateInfo, nullptr, &fVKSwapchain);
        if (VK_SUCCESS != res) {
            return false;
        }

        // destroy the old swapchain
        if (swapchainCreateInfo.oldSwapchain != VK_NULL_HANDLE) {
            fDeviceWaitIdle(fVKDevice);

            this->destroyBuffers();

            fDestroySwapchainKHR(fVKDevice, swapchainCreateInfo.oldSwapchain, nullptr);
        }

        if (!this->createBuffers(swapchainCreateInfo.imageFormat, usageFlags, colorType,
                                 swapchainCreateInfo.imageSharingMode)) {
            fDeviceWaitIdle(fVKDevice);

            this->destroyBuffers();

            fDestroySwapchainKHR(fVKDevice, swapchainCreateInfo.oldSwapchain, nullptr);
        }

        return true;
    }

    bool createBuffers(VkFormat format, VkImageUsageFlags usageFlags, SkColorType colorType,
                       VkSharingMode sharingMode) {
        fGetSwapchainImagesKHR(fVKDevice, fVKSwapchain, &fImageCount, nullptr);
        SkASSERT(fImageCount);
        fImages = new VkImage[fImageCount];
        fGetSwapchainImagesKHR(fVKDevice, fVKSwapchain, &fImageCount, fImages);

        // set up initial image layouts and create surfaces
        fImageLayouts = new VkImageLayout[fImageCount];
        fSurfaces = new sk_sp<SkSurface>[fImageCount];
        for (uint32_t i = 0; i < fImageCount; ++i) {
            fImageLayouts[i] = VK_IMAGE_LAYOUT_UNDEFINED;

            GrVkImageInfo info;
            info.fImage = fImages[i];
            info.fAlloc = GrVkAlloc();
            info.fImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            info.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
            info.fFormat = format;
            info.fImageUsageFlags = usageFlags;
            info.fLevelCount = 1;
            info.fCurrentQueueFamily = fVKPresentQueueIndex;
            info.fSharingMode = sharingMode;

            if (usageFlags & VK_IMAGE_USAGE_SAMPLED_BIT) {
                GrBackendTexture backendTexture(fSize.width(), fSize.height(), info);
                fSurfaces[i] = SkSurface::MakeFromBackendTexture(
                        fContext.get(), backendTexture, kTopLeft_GrSurfaceOrigin,
                        fDisplayParams.fMSAASampleCount,
                        colorType, fDisplayParams.fColorSpace, &fDisplayParams.fSurfaceProps);
            } else {
                if (fDisplayParams.fMSAASampleCount > 1) {
                    return false;
                }
                GrBackendRenderTarget backendRT(fSize.width(), fSize.height(), fSampleCount, info);
                fSurfaces[i] = SkSurface::MakeFromBackendRenderTarget(
                        fContext.get(), backendRT, kTopLeft_GrSurfaceOrigin, colorType,
                        fDisplayParams.fColorSpace, &fDisplayParams.fSurfaceProps);

            }
            if (!fSurfaces[i]) {
                return false;
            }
        }

        // set up the backbuffers
        VkSemaphoreCreateInfo semaphoreInfo;
        memset(&semaphoreInfo, 0, sizeof(VkSemaphoreCreateInfo));
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreInfo.pNext = nullptr;
        semaphoreInfo.flags = 0;

        // we create one additional backbuffer structure here, because we want to
        // give the command buffers they contain a chance to finish before we cycle back
        fBackbuffers = new BackbufferInfo[fImageCount + 1];
        for (uint32_t i = 0; i < fImageCount + 1; ++i) {
            fBackbuffers[i].fImageIndex = -1;
            SkDEBUGCODE(VkResult result = )GR_VK_CALL(fInterface,
                    CreateSemaphore(fVKDevice, &semaphoreInfo, nullptr,
                                    &fBackbuffers[i].fRenderSemaphore));
            SkASSERT(result == VK_SUCCESS);
        }
        fCurrentBackbufferIndex = fImageCount;
        return true;
    }

    void destroyBuffers() {
        if (fBackbuffers) {
            for (uint32_t i = 0; i < fImageCount + 1; ++i) {
                fBackbuffers[i].fImageIndex = -1;
                GR_VK_CALL(fInterface,
                           DestroySemaphore(fVKDevice,
                                            fBackbuffers[i].fRenderSemaphore,
                                            nullptr));
            }
        }

        delete[] fBackbuffers;
        fBackbuffers = nullptr;

        // Does this actually free the surfaces?
        delete[] fSurfaces;
        fSurfaces = nullptr;
        delete[] fImageLayouts;
        fImageLayouts = nullptr;
        delete[] fImages;
        fImages = nullptr;
    }

    void destroyContext() {
        if (fVKDevice != VK_NULL_HANDLE) {
            fQueueWaitIdle(fVKPresentQueue);
            fDeviceWaitIdle(fVKDevice);

            this->destroyBuffers();

            if (VK_NULL_HANDLE != fVKSwapchain) {
                fDestroySwapchainKHR(fVKDevice, fVKSwapchain, nullptr);
                fVKSwapchain = VK_NULL_HANDLE;
            }

            if (VK_NULL_HANDLE != fSurface) {
                fDestroySurfaceKHR(fVKInstance, fVKSurface, nullptr);
                fSurface = VK_NULL_HANDLE;
            }
        }

        SkASSERT(fContext->unique());
        fContext.reset();
        fInterface.reset();

        if (VK_NULL_HANDLE != fVKDevice) {
            fDestroyDevice(fVKDevice, nullptr);
            fVKDevice = VK_NULL_HANDLE;
        }

#ifdef SK_ENABLE_VK_LAYERS
        if (fVKDebugCallback != VK_NULL_HANDLE) {
            fDestroyDebugReportCallbackEXT(fVKInstance, fVKDebugCallback, nullptr);
        }
#endif

        fVKPhysicalDevice = VK_NULL_HANDLE;

        if (VK_NULL_HANDLE != fVKInstance) {
            fDestroyInstance(fVKInstance, nullptr);
            fVKInstance = VK_NULL_HANDLE;
        }
    }

    BackbufferInfo* getAvailableBackbuffer() {
        SkASSERT(fBackbuffers);

        ++fCurrentBackbufferIndex;
        if (fCurrentBackbufferIndex > fImageCount) {
            fCurrentBackbufferIndex = 0;
        }

        BackbufferInfo* backbuffer = fBackbuffers + fCurrentBackbufferIndex;
        return backbuffer;
    }

    sk_sp<SkSurface> getBackbufferSurface() {
        BackbufferInfo* backbuffer = this->getAvailableBackbuffer();
        SkASSERT(backbuffer);

        // semaphores should be in unsignaled state
        VkSemaphoreCreateInfo semaphoreInfo;
        memset(&semaphoreInfo, 0, sizeof(VkSemaphoreCreateInfo));
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreInfo.pNext = nullptr;
        semaphoreInfo.flags = 0;
        VkSemaphore semaphore;
        SkDEBUGCODE(VkResult result = )
            GR_VK_CALL(fInterface, CreateSemaphore(fVKDevice, &semaphoreInfo, nullptr, &semaphore));
        SkASSERT(result == VK_SUCCESS);

        // acquire the image
        VkResult res = fAcquireNextImageKHR(fVKDevice, fVKSwapchain, UINT64_MAX,
                                            semaphore, VK_NULL_HANDLE,
                                            &backbuffer->fImageIndex);
        if (VK_ERROR_SURFACE_LOST_KHR == res) {
            // need to figure out how to create a new vkSurface without the platformData*
            // maybe use attach somehow? but need a Window
            GR_VK_CALL(fInterface, DestroySemaphore(fVKDevice, semaphore, nullptr));
            return nullptr;
        }
        if (VK_ERROR_OUT_OF_DATE_KHR == res) {
            // tear swapchain down and try again
            if (!this->createSwapchain(-1, -1, fDisplayParams)) {
                GR_VK_CALL(fInterface, DestroySemaphore(fVKDevice, semaphore, nullptr));
                return nullptr;
            }
            backbuffer = this->getAvailableBackbuffer();

            // acquire the image
            res = fAcquireNextImageKHR(fVKDevice, fVKSwapchain, UINT64_MAX,
                                       semaphore, VK_NULL_HANDLE,
                                       &backbuffer->fImageIndex);

            if (VK_SUCCESS != res) {
                GR_VK_CALL(fInterface, DestroySemaphore(fVKDevice, semaphore, nullptr));
                return nullptr;
            }
        }

        SkSurface* surface = fSurfaces[backbuffer->fImageIndex].get();

        GrBackendSemaphore beSemaphore;
        beSemaphore.initVulkan(semaphore);

        surface->wait(1, &beSemaphore);

        return sk_ref_sp(surface);
    }

    void flushAndSubmit() override {
        BackbufferInfo* backbuffer = fBackbuffers + fCurrentBackbufferIndex;
        SkSurface* surface = fSurfaces[backbuffer->fImageIndex].get();

        GrBackendSemaphore beSemaphore;
        beSemaphore.initVulkan(backbuffer->fRenderSemaphore);

        GrFlushInfo info;
        info.fNumSemaphores = 1;
        info.fSignalSemaphores = &beSemaphore;
        GrBackendSurfaceMutableState presentState(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                                  fVKPresentQueueIndex);
        surface->flush(info, &presentState);
        surface->recordingContext()->asDirectContext()->submit();

        // Submit present operation to present queue
        const VkPresentInfoKHR presentInfo =
        {
            VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, // sType
            nullptr, // pNext
            1, // waitSemaphoreCount
            &backbuffer->fRenderSemaphore, // pWaitSemaphores
            1, // swapchainCount
            &fVKSwapchain, // pSwapchains
            &backbuffer->fImageIndex, // pImageIndices
            nullptr // pResults
        };

        fQueuePresentKHR(fVKPresentQueue, &presentInfo);

        fSurface = this->getBackbufferSurface();
    }

    void release(JNIEnv* env) override {
        __android_log_print(ANDROID_LOG_ERROR, "*** AK", "VKSurface::release()");

        this->destroyContext();

        ANativeWindow_release(fWindow);
    }

    ANativeWindow* fWindow;

    VkInstance               fVKInstance       = VK_NULL_HANDLE;
    VkPhysicalDevice         fVKPhysicalDevice = VK_NULL_HANDLE;
    VkDevice                 fVKDevice         = VK_NULL_HANDLE;
    VkDebugReportCallbackEXT fVKDebugCallback  = VK_NULL_HANDLE;

    sk_sp<const GrVkInterface> fInterface;
    sk_sp<GrDirectContext>     fContext;

    VkSurfaceKHR   fVKSurface   = VK_NULL_HANDLE;
    VkSwapchainKHR fVKSwapchain = VK_NULL_HANDLE;

    uint32_t fVKGraphicsQueueIndex = 0;
    VkQueue  fVKGraphicsQueue;
    uint32_t fVKPresentQueueIndex  = 0;
    VkQueue  fVKPresentQueue;

    uint32_t               fImageCount             = 0;
    VkImage*               fImages                 = nullptr;
    VkImageLayout*         fImageLayouts           = nullptr;
    sk_sp<SkSurface>*      fSurfaces               = nullptr;
    BackbufferInfo*        fBackbuffers            = nullptr;
    uint32_t               fCurrentBackbufferIndex = 0;

    // WSI interface functions
    PFN_vkDestroySurfaceKHR fDestroySurfaceKHR = nullptr;
    PFN_vkGetPhysicalDeviceSurfaceSupportKHR fGetPhysicalDeviceSurfaceSupportKHR = nullptr;
    PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fGetPhysicalDeviceSurfaceCapabilitiesKHR =nullptr;
    PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fGetPhysicalDeviceSurfaceFormatsKHR = nullptr;
    PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fGetPhysicalDeviceSurfacePresentModesKHR =nullptr;

    PFN_vkCreateSwapchainKHR fCreateSwapchainKHR = nullptr;
    PFN_vkDestroySwapchainKHR fDestroySwapchainKHR = nullptr;
    PFN_vkGetSwapchainImagesKHR fGetSwapchainImagesKHR = nullptr;
    PFN_vkAcquireNextImageKHR fAcquireNextImageKHR = nullptr;
    PFN_vkQueuePresentKHR fQueuePresentKHR = nullptr;

    PFN_vkDestroyInstance fDestroyInstance = nullptr;
    PFN_vkDeviceWaitIdle fDeviceWaitIdle = nullptr;
    PFN_vkDestroyDebugReportCallbackEXT fDestroyDebugReportCallbackEXT = nullptr;
    PFN_vkQueueWaitIdle fQueueWaitIdle = nullptr;
    PFN_vkDestroyDevice fDestroyDevice = nullptr;
    PFN_vkGetDeviceQueue fGetDeviceQueue = nullptr;

    SkISize       fSize = {0,0};
    DisplayParams fDisplayParams;
    // parameters obtained from the native window
    int           fSampleCount;
};

}

jlong Surface_CreateVK(JNIEnv* env, jobject, jobject jsurface) {
    __android_log_print(ANDROID_LOG_ERROR, "*** AK", "Surface_CreateGL");

    auto* win = ANativeWindow_fromSurface(env, jsurface);
    if (!win) {
        return 0;
    }

    return reinterpret_cast<jlong>(new VKSurface(win));
}
