/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/vk/VkTestUtils.h"

#ifdef SK_VULKAN

#ifndef SK_GPU_TOOLS_VK_LIBRARY_NAME
    #if defined _WIN32
        #define SK_GPU_TOOLS_VK_LIBRARY_NAME vulkan-1.dll
    #elif defined SK_BUILD_FOR_MAC
        #define SK_GPU_TOOLS_VK_LIBRARY_NAME libvk_swiftshader.dylib
    #else
        #define SK_GPU_TOOLS_VK_LIBRARY_NAME        libvulkan.so
        #define SK_GPU_TOOLS_VK_LIBRARY_NAME_BACKUP libvulkan.so.1
    #endif
#endif

#define STRINGIFY2(S) #S
#define STRINGIFY(S) STRINGIFY2(S)

#include <algorithm>

#if defined(__GLIBC__)
#include <execinfo.h>
#endif
#include "include/gpu/vk/GrVkBackendContext.h"
#include "include/gpu/vk/VulkanBackendContext.h"
#include "include/gpu/vk/VulkanExtensions.h"
#include "src/base/SkAutoMalloc.h"
#include "src/ports/SkOSLibrary.h"

#if defined(SK_ENABLE_SCOPED_LSAN_SUPPRESSIONS)
#include <sanitizer/lsan_interface.h>
#endif

using namespace skia_private;

namespace sk_gpu_test {

bool LoadVkLibraryAndGetProcAddrFuncs(PFN_vkGetInstanceProcAddr* instProc) {
    static void* vkLib = nullptr;
    static PFN_vkGetInstanceProcAddr localInstProc = nullptr;
    if (!vkLib) {
        vkLib = SkLoadDynamicLibrary(STRINGIFY(SK_GPU_TOOLS_VK_LIBRARY_NAME));
        if (!vkLib) {
            // vulkaninfo tries to load the library from two places, so we do as well
            // https://github.com/KhronosGroup/Vulkan-Tools/blob/078d44e4664b7efa0b6c96ebced1995c4425d57a/vulkaninfo/vulkaninfo.h#L249
#ifdef SK_GPU_TOOLS_VK_LIBRARY_NAME_BACKUP
            vkLib = SkLoadDynamicLibrary(STRINGIFY(SK_GPU_TOOLS_VK_LIBRARY_NAME_BACKUP));
            if (!vkLib) {
                return false;
            }
#else
            return false;
#endif
        }
        localInstProc = (PFN_vkGetInstanceProcAddr) SkGetProcedureAddress(vkLib,
                                                                          "vkGetInstanceProcAddr");
    }
    if (!localInstProc) {
        return false;
    }
    *instProc = localInstProc;
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// Helper code to set up Vulkan context objects

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
#if defined(__GLIBC__)
    void* stack[64];
    int count = backtrace(stack, std::size(stack));
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

#define ACQUIRE_VK_INST_PROC_LOCAL(name, instance)                                 \
    PFN_vk##name grVk##name =                                                      \
        reinterpret_cast<PFN_vk##name>(getInstProc(instance, "vk" #name));         \
    do {                                                                           \
        if (grVk##name == nullptr) {                                               \
            SkDebugf("Function ptr for vk%s could not be acquired\n", #name);      \
            return false;                                                          \
        }                                                                          \
    } while (0)

static bool init_instance_extensions_and_layers(PFN_vkGetInstanceProcAddr getInstProc,
                                                uint32_t specVersion,
                                                TArray<VkExtensionProperties>* instanceExtensions,
                                                TArray<VkLayerProperties>* instanceLayers) {
    if (getInstProc == nullptr) {
        return false;
    }

    ACQUIRE_VK_INST_PROC_LOCAL(EnumerateInstanceExtensionProperties, VK_NULL_HANDLE);
    ACQUIRE_VK_INST_PROC_LOCAL(EnumerateInstanceLayerProperties, VK_NULL_HANDLE);

    VkResult res;
    uint32_t layerCount = 0;
#ifdef SK_ENABLE_VK_LAYERS
    // instance layers
    res = grVkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    if (VK_SUCCESS != res) {
        return false;
    }
    VkLayerProperties* layers = new VkLayerProperties[layerCount];
    res = grVkEnumerateInstanceLayerProperties(&layerCount, layers);
    if (VK_SUCCESS != res) {
        delete[] layers;
        return false;
    }

    uint32_t nonPatchVersion = remove_patch_version(specVersion);
    for (size_t i = 0; i < std::size(kDebugLayerNames); ++i) {
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
    {
        uint32_t extensionCount = 0;
        res = grVkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        if (VK_SUCCESS != res) {
            return false;
        }
        VkExtensionProperties* extensions = new VkExtensionProperties[extensionCount];
        res = grVkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions);
        if (VK_SUCCESS != res) {
            delete[] extensions;
            return false;
        }
        for (uint32_t i = 0; i < extensionCount; ++i) {
            instanceExtensions->push_back() = extensions[i];
        }
        delete [] extensions;
    }

    // via explicitly enabled layers
    layerCount = instanceLayers->size();
    for (uint32_t layerIndex = 0; layerIndex < layerCount; ++layerIndex) {
        uint32_t extensionCount = 0;
        res = grVkEnumerateInstanceExtensionProperties((*instanceLayers)[layerIndex].layerName,
                                                       &extensionCount, nullptr);
        if (VK_SUCCESS != res) {
            return false;
        }
        VkExtensionProperties* extensions = new VkExtensionProperties[extensionCount];
        res = grVkEnumerateInstanceExtensionProperties((*instanceLayers)[layerIndex].layerName,
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

#define GET_PROC_LOCAL(F, inst, device) PFN_vk ## F F = (PFN_vk ## F) getProc("vk" #F, inst, device)

static bool init_device_extensions_and_layers(const skgpu::VulkanGetProc& getProc,
                                              uint32_t specVersion, VkInstance inst,
                                              VkPhysicalDevice physDev,
                                              TArray<VkExtensionProperties>* deviceExtensions,
                                              TArray<VkLayerProperties>* deviceLayers) {
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
    for (size_t i = 0; i < std::size(kDebugLayerNames); ++i) {
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
    {
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
    }

    // via explicitly enabled layers
    layerCount = deviceLayers->size();
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

#define ACQUIRE_VK_INST_PROC_NOCHECK(name, instance) \
    PFN_vk##name grVk##name = reinterpret_cast<PFN_vk##name>(getInstProc(instance, "vk" #name))

#define ACQUIRE_VK_INST_PROC(name, instance) \
    PFN_vk##name grVk##name =                                                          \
        reinterpret_cast<PFN_vk##name>(getInstProc(instance, "vk" #name));             \
    do {                                                                               \
        if (grVk##name == nullptr) {                                                   \
            SkDebugf("Function ptr for vk%s could not be acquired\n", #name);          \
            if (inst != VK_NULL_HANDLE) {                                              \
                destroy_instance(getInstProc, inst, debugCallback, hasDebugExtension); \
            }                                                                          \
            return false;                                                              \
        }                                                                              \
    } while (0)

#define ACQUIRE_VK_PROC_NOCHECK(name, instance, device) \
    PFN_vk##name grVk##name = reinterpret_cast<PFN_vk##name>(getProc("vk" #name, instance, device))

#define ACQUIRE_VK_PROC(name, instance, device)                                        \
    PFN_vk##name grVk##name =                                                          \
            reinterpret_cast<PFN_vk##name>(getProc("vk" #name, instance, device));     \
    do {                                                                               \
        if (grVk##name == nullptr) {                                                   \
            SkDebugf("Function ptr for vk%s could not be acquired\n", #name);          \
            if (inst != VK_NULL_HANDLE) {                                              \
                destroy_instance(getInstProc, inst, debugCallback, hasDebugExtension); \
            }                                                                          \
            return false;                                                              \
        }                                                                              \
    } while (0)

#define ACQUIRE_VK_PROC_LOCAL(name, instance, device)                              \
    PFN_vk##name grVk##name =                                                      \
            reinterpret_cast<PFN_vk##name>(getProc("vk" #name, instance, device)); \
    do {                                                                           \
        if (grVk##name == nullptr) {                                               \
            SkDebugf("Function ptr for vk%s could not be acquired\n", #name);      \
            return false;                                                          \
        }                                                                          \
    } while (0)

static bool destroy_instance(PFN_vkGetInstanceProcAddr getInstProc, VkInstance inst,
                             VkDebugReportCallbackEXT* debugCallback,
                             bool hasDebugExtension) {
    if (hasDebugExtension && *debugCallback != VK_NULL_HANDLE) {
        ACQUIRE_VK_INST_PROC_LOCAL(DestroyDebugReportCallbackEXT, inst);
        grVkDestroyDebugReportCallbackEXT(inst, *debugCallback, nullptr);
        *debugCallback = VK_NULL_HANDLE;
    }
    ACQUIRE_VK_INST_PROC_LOCAL(DestroyInstance, inst);
    grVkDestroyInstance(inst, nullptr);
    return true;
}

static bool setup_features(const skgpu::VulkanGetProc& getProc, VkInstance inst,
                           VkPhysicalDevice physDev, uint32_t physDeviceVersion,
                           skgpu::VulkanExtensions* extensions, VkPhysicalDeviceFeatures2* features,
                           bool isProtected) {
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

// TODO: remove once GrVkBackendContext is deprecated (skbug.com/309785258)
void ConvertBackendContext(const skgpu::VulkanBackendContext& newStyle,
                           GrVkBackendContext* oldStyle) {
    oldStyle->fInstance = newStyle.fInstance;
    oldStyle->fPhysicalDevice = newStyle.fPhysicalDevice;
    oldStyle->fDevice = newStyle.fDevice;
    oldStyle->fQueue = newStyle.fQueue;
    oldStyle->fGraphicsQueueIndex = newStyle.fGraphicsQueueIndex;
    oldStyle->fMaxAPIVersion = newStyle.fMaxAPIVersion;
    oldStyle->fVkExtensions = newStyle.fVkExtensions;
    oldStyle->fDeviceFeatures = newStyle.fDeviceFeatures;
    oldStyle->fDeviceFeatures2 = newStyle.fDeviceFeatures2;
    oldStyle->fMemoryAllocator = newStyle.fMemoryAllocator;
    oldStyle->fGetProc = newStyle.fGetProc;
    oldStyle->fProtectedContext = newStyle.fProtectedContext;
}

// TODO: remove once GrVkBackendContext is deprecated (skbug.com/309785258)
bool CreateVkBackendContext(PFN_vkGetInstanceProcAddr getInstProc,
                            GrVkBackendContext* ctx,
                            skgpu::VulkanExtensions* extensions,
                            VkPhysicalDeviceFeatures2* features,
                            VkDebugReportCallbackEXT* debugCallback,
                            uint32_t* presentQueueIndexPtr,
                            const CanPresentFn& canPresent,
                            bool isProtected) {
    skgpu::VulkanBackendContext skgpuCtx;
    if (!CreateVkBackendContext(getInstProc,
                                &skgpuCtx,
                                extensions,
                                features,
                                debugCallback,
                                presentQueueIndexPtr,
                                canPresent,
                                isProtected)) {
        return false;
    }

    SkASSERT(skgpuCtx.fProtectedContext == skgpu::Protected(isProtected));

    ConvertBackendContext(skgpuCtx, ctx);
    return true;
}

bool CreateVkBackendContext(PFN_vkGetInstanceProcAddr getInstProc,
                            skgpu::VulkanBackendContext* ctx,
                            skgpu::VulkanExtensions* extensions,
                            VkPhysicalDeviceFeatures2* features,
                            VkDebugReportCallbackEXT* debugCallback,
                            uint32_t* presentQueueIndexPtr,
                            const CanPresentFn& canPresent,
                            bool isProtected) {
    VkResult err;

    ACQUIRE_VK_INST_PROC_NOCHECK(EnumerateInstanceVersion, VK_NULL_HANDLE);
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

    STArray<2, VkPhysicalDevice> physDevs;
    VkDevice device;
    VkInstance inst = VK_NULL_HANDLE;

    const VkApplicationInfo app_info = {
        VK_STRUCTURE_TYPE_APPLICATION_INFO, // sType
        nullptr,                            // pNext
        "vktest",                           // pApplicationName
        0,                                  // applicationVersion
        "vktest",                           // pEngineName
        0,                                  // engineVerison
        apiVersion,                         // apiVersion
    };

    TArray<VkLayerProperties> instanceLayers;
    TArray<VkExtensionProperties> instanceExtensions;

    if (!init_instance_extensions_and_layers(getInstProc, instanceVersion,
                                             &instanceExtensions,
                                             &instanceLayers)) {
        return false;
    }

    TArray<const char*> instanceLayerNames;
    TArray<const char*> instanceExtensionNames;
    for (int i = 0; i < instanceLayers.size(); ++i) {
        instanceLayerNames.push_back(instanceLayers[i].layerName);
    }
    for (int i = 0; i < instanceExtensions.size(); ++i) {
        if (strncmp(instanceExtensions[i].extensionName, "VK_KHX", 6) != 0) {
            instanceExtensionNames.push_back(instanceExtensions[i].extensionName);
        }
    }

    const VkInstanceCreateInfo instance_create = {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,    // sType
        nullptr,                                   // pNext
        0,                                         // flags
        &app_info,                                 // pApplicationInfo
        (uint32_t) instanceLayerNames.size(),      // enabledLayerNameCount
        instanceLayerNames.begin(),                // ppEnabledLayerNames
        (uint32_t) instanceExtensionNames.size(),  // enabledExtensionNameCount
        instanceExtensionNames.begin(),            // ppEnabledExtensionNames
    };

    bool hasDebugExtension = false;

    ACQUIRE_VK_INST_PROC(CreateInstance, VK_NULL_HANDLE);
    err = grVkCreateInstance(&instance_create, nullptr, &inst);
    if (err < 0) {
        SkDebugf("vkCreateInstance failed: %d\n", err);
        return false;
    }

    ACQUIRE_VK_INST_PROC(GetDeviceProcAddr, inst);
    auto getProc = [getInstProc, grVkGetDeviceProcAddr](const char* proc_name,
                                                        VkInstance instance, VkDevice device) {
        if (device != VK_NULL_HANDLE) {
            return grVkGetDeviceProcAddr(device, proc_name);
        }
        return getInstProc(instance, proc_name);
    };

#ifdef SK_ENABLE_VK_LAYERS
    *debugCallback = VK_NULL_HANDLE;
    for (int i = 0; i < instanceExtensionNames.size() && !hasDebugExtension; ++i) {
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
        destroy_instance(getInstProc, inst, debugCallback, hasDebugExtension);
        return false;
    }
    if (!gpuCount) {
        SkDebugf("vkEnumeratePhysicalDevices returned no supported devices.\n");
        destroy_instance(getInstProc, inst, debugCallback, hasDebugExtension);
        return false;
    }
    // Allocate enough storage for all available physical devices. We should be able to just ask for
    // the first one, but a bug in RenderDoc (https://github.com/baldurk/renderdoc/issues/2766)
    // will smash the stack if we do that.
    physDevs.resize(gpuCount);
    err = grVkEnumeratePhysicalDevices(inst, &gpuCount, physDevs.data());
    if (err) {
        SkDebugf("vkEnumeratePhysicalDevices failed: %d\n", err);
        destroy_instance(getInstProc, inst, debugCallback, hasDebugExtension);
        return false;
    }
    // We just use the first physical device.
    // TODO: find best match for our needs
    VkPhysicalDevice physDev = physDevs.front();

    VkPhysicalDeviceProperties physDeviceProperties;
    grVkGetPhysicalDeviceProperties(physDev, &physDeviceProperties);
    uint32_t physDeviceVersion = std::min(physDeviceProperties.apiVersion, apiVersion);

    if (isProtected && physDeviceVersion < VK_MAKE_VERSION(1, 1, 0)) {
        SkDebugf("protected requires vk physical device version 1.1\n");
        destroy_instance(getInstProc, inst, debugCallback, hasDebugExtension);
        return false;
    }

    // query to get the initial queue props size
    uint32_t queueCount;
    grVkGetPhysicalDeviceQueueFamilyProperties(physDev, &queueCount, nullptr);
    if (!queueCount) {
        SkDebugf("vkGetPhysicalDeviceQueueFamilyProperties returned no queues.\n");
        destroy_instance(getInstProc, inst, debugCallback, hasDebugExtension);
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
        destroy_instance(getInstProc, inst, debugCallback, hasDebugExtension);
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
            destroy_instance(getInstProc, inst, debugCallback, hasDebugExtension);
            return false;
        }
        *presentQueueIndexPtr = presentQueueIndex;
    } else {
        // Just setting this so we end up make a single queue for graphics since there was no
        // request for a present queue.
        presentQueueIndex = graphicsQueueIndex;
    }

    TArray<VkLayerProperties> deviceLayers;
    TArray<VkExtensionProperties> deviceExtensions;
    if (!init_device_extensions_and_layers(getProc, physDeviceVersion,
                                           inst, physDev,
                                           &deviceExtensions,
                                           &deviceLayers)) {
        destroy_instance(getInstProc, inst, debugCallback, hasDebugExtension);
        return false;
    }

    TArray<const char*> deviceLayerNames;
    TArray<const char*> deviceExtensionNames;
    for (int i = 0; i < deviceLayers.size(); ++i) {
        deviceLayerNames.push_back(deviceLayers[i].layerName);
    }

    // We can't have both VK_KHR_buffer_device_address and VK_EXT_buffer_device_address as
    // extensions. So see if we have the KHR version and if so don't push back the EXT version in
    // the next loop.
    bool hasKHRBufferDeviceAddress = false;
    for (int i = 0; i < deviceExtensions.size(); ++i) {
        if (!strcmp(deviceExtensions[i].extensionName, "VK_KHR_buffer_device_address")) {
            hasKHRBufferDeviceAddress = true;
            break;
        }
    }

    for (int i = 0; i < deviceExtensions.size(); ++i) {
        // Don't use experimental extensions since they typically don't work with debug layers and
        // often are missing dependecy requirements for other extensions. Additionally, these are
        // often left behind in the driver even after they've been promoted to real extensions.
        if (0 != strncmp(deviceExtensions[i].extensionName, "VK_KHX", 6) &&
            0 != strncmp(deviceExtensions[i].extensionName, "VK_NVX", 6)) {

            // There are some extensions that are not supported by the debug layers which result in
            // many warnings even though we don't actually use them. It's easiest to just
            // avoid enabling those.
            if (0 == strcmp(deviceExtensions[i].extensionName, "VK_EXT_provoking_vertex")     ||
                0 == strcmp(deviceExtensions[i].extensionName, "VK_EXT_shader_object")        ||
                0 == strcmp(deviceExtensions[i].extensionName, "VK_KHR_dynamic_rendering")    ||
                0 == strcmp(deviceExtensions[i].extensionName, "VK_NV_acquire_winrt_display") ||
                0 == strcmp(deviceExtensions[i].extensionName, "VK_NV_cuda_kernel_launch")    ||
                0 == strcmp(deviceExtensions[i].extensionName, "VK_NV_low_latency")           ||
                0 == strcmp(deviceExtensions[i].extensionName, "VK_NV_present_barrier")) {
                continue;
            }

            if (!hasKHRBufferDeviceAddress ||
                0 != strcmp(deviceExtensions[i].extensionName, "VK_EXT_buffer_device_address")) {
                deviceExtensionNames.push_back(deviceExtensions[i].extensionName);
            }
        }
    }

    extensions->init(getProc, inst, physDev,
                     (uint32_t) instanceExtensionNames.size(),
                     instanceExtensionNames.begin(),
                     (uint32_t) deviceExtensionNames.size(),
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
            destroy_instance(getInstProc, inst, debugCallback, hasDebugExtension);
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
        (uint32_t) deviceLayerNames.size(),          // layerCount
        deviceLayerNames.begin(),                    // ppEnabledLayerNames
        (uint32_t) deviceExtensionNames.size(),      // extensionCount
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
        destroy_instance(getInstProc, inst, debugCallback, hasDebugExtension);
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
    ctx->fProtectedContext = skgpu::Protected(isProtected);

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

}  // namespace sk_gpu_test

#endif
