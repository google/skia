/*
 * Copyright 2017 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/base/SkMacros.h"
#include "src/gpu/vk/VulkanInterface.h"
#include "tools/gpu/vk/VkTestMemoryAllocator.h"
#include "tools/gpu/vk/VkTestUtils.h"

#include <functional>

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

#include <algorithm>

#if defined(__GLIBC__)
#include <execinfo.h>
#endif
#include "include/gpu/vk/VulkanBackendContext.h"
#include "include/gpu/vk/VulkanExtensions.h"
#include "src/base/SkAutoMalloc.h"
#include "tools/library/LoadDynamicLibrary.h"

#if defined(SK_ENABLE_SCOPED_LSAN_SUPPRESSIONS)
#include <sanitizer/lsan_interface.h>
#endif

using namespace skia_private;

namespace sk_gpu_test {

bool LoadVkLibraryAndGetProcAddrFuncs(PFN_vkGetInstanceProcAddr* instProc) {
    static void* vkLib = nullptr;
    static PFN_vkGetInstanceProcAddr localInstProc = nullptr;
    if (!vkLib) {
        vkLib = SkLoadDynamicLibrary(SK_MACRO_STRINGIFY(SK_GPU_TOOLS_VK_LIBRARY_NAME));
        if (!vkLib) {
            // vulkaninfo tries to load the library from two places, so we do as well
            // https://github.com/KhronosGroup/Vulkan-Tools/blob/078d44e4664b7efa0b6c96ebced1995c4425d57a/vulkaninfo/vulkaninfo.h#L249
#ifdef SK_GPU_TOOLS_VK_LIBRARY_NAME_BACKUP
            vkLib = SkLoadDynamicLibrary(SK_MACRO_STRINGIFY(SK_GPU_TOOLS_VK_LIBRARY_NAME_BACKUP));
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

static void print_backtrace() {
#if defined(__GLIBC__)
    void* stack[64];
    int count = backtrace(stack, std::size(stack));
    backtrace_symbols_fd(stack, count, 2);
#else
    // Please add implementations for other platforms.
#endif
}

VKAPI_ATTR VkBool32 VKAPI_CALL
DebugUtilsMessenger(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
                    void* userData) {
    // VUID-VkDebugUtilsMessengerCallbackDataEXT-pMessage-parameter
    // pMessage must be a null-terminated UTF-8 string
    SkASSERT(callbackData->pMessage != nullptr);

    static constexpr const char* kSkippedMessages[] = {
            "Nothing for now, this string works around msvc bug with empty array",
    };

    // See if it's an issue we are aware of and don't want to be spammed about.
    // Always report the debug message if message ID is missing
    if (callbackData->pMessageIdName != nullptr) {
        for (const char* skipped : kSkippedMessages) {
            if (strstr(callbackData->pMessageIdName, skipped) != nullptr) {
                return VK_FALSE;
            }
        }
    }

    bool printStackTrace = true;
    bool fail = false;

    const char* severity = "message";
    if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0) {
        severity = "error";
        fail = true;
    } else if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) != 0) {
        severity = "warning";
    } else if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) != 0) {
        severity = "info";
        printStackTrace = false;
    }

    std::string type;
    if ((messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) != 0) {
        type += " <general>";
    }
    if ((messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) != 0) {
        type += " <validation>";
    }
    if ((messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) != 0) {
        type += " <performance>";
    }

    SkDebugf("Vulkan %s%s [%s]: %s\n",
             severity,
             type.c_str(),
             callbackData->pMessageIdName ? callbackData->pMessageIdName : "<no id>",
             callbackData->pMessage);

    if (printStackTrace) {
        print_backtrace();
    }

    if (fail) {
        SkDEBUGFAIL("Vulkan debug layer error");
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

#define GET_PROC_LOCAL(F, inst, device) PFN_vk ## F F = (PFN_vk ## F) getProc("vk" #F, inst, device)

#define ACQUIRE_VK_INST_PROC_NOCHECK(name, instance) \
    PFN_vk##name grVk##name = reinterpret_cast<PFN_vk##name>(getInstProc(instance, "vk" #name))

#define ACQUIRE_VK_INST_PROC(name, instance)                                                     \
    PFN_vk##name grVk##name = reinterpret_cast<PFN_vk##name>(getInstProc(instance, "vk" #name)); \
    do {                                                                                         \
        if (grVk##name == nullptr) {                                                             \
            SkDebugf("Function ptr for vk%s could not be acquired\n", #name);                    \
            if (inst != VK_NULL_HANDLE) {                                                        \
                destroy_instance(getInstProc, inst, debugMessenger, hasDebugExtension);          \
            }                                                                                    \
            return false;                                                                        \
        }                                                                                        \
    } while (0)

#define ACQUIRE_VK_PROC_NOCHECK(name, instance, device) \
    PFN_vk##name grVk##name = reinterpret_cast<PFN_vk##name>(getProc("vk" #name, instance, device))

#define ACQUIRE_VK_PROC(name, instance, device)                                         \
    PFN_vk##name grVk##name =                                                           \
            reinterpret_cast<PFN_vk##name>(getProc("vk" #name, instance, device));      \
    do {                                                                                \
        if (grVk##name == nullptr) {                                                    \
            SkDebugf("Function ptr for vk%s could not be acquired\n", #name);           \
            if (inst != VK_NULL_HANDLE) {                                               \
                destroy_instance(getInstProc, inst, debugMessenger, hasDebugExtension); \
            }                                                                           \
            return false;                                                               \
        }                                                                               \
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

static bool destroy_instance(PFN_vkGetInstanceProcAddr getInstProc,
                             VkInstance inst,
                             VkDebugUtilsMessengerEXT* debugMessenger,
                             bool hasDebugExtension) {
    if (hasDebugExtension && *debugMessenger != VK_NULL_HANDLE) {
        ACQUIRE_VK_INST_PROC_LOCAL(DestroyDebugUtilsMessengerEXT, inst);
        grVkDestroyDebugUtilsMessengerEXT(inst, *debugMessenger, nullptr);
        *debugMessenger = VK_NULL_HANDLE;
    }
    ACQUIRE_VK_INST_PROC_LOCAL(DestroyInstance, inst);
    grVkDestroyInstance(inst, nullptr);
    return true;
}
namespace {

uint32_t remove_patch_version(uint32_t specVersion) {
    return (specVersion >> 12) << 12;
}

#if defined (SK_ENABLE_VK_LAYERS)
// Checks a list of availableLayers for layers we want to use (kDebugLayerNames). If the desired
// layers are available and compatible with our Vulkan version, append their VkLayerProperties to
// layersToUse.
void append_desired_layers(const VkLayerProperties* availableLayers,
                           const uint32_t availLayerCount,
                           const uint32_t majorSpecVersion,
                           TArray<VkLayerProperties>& layersToUse,
                           TArray<const char*>& layersToUseNames) {
    // Comment out or include different debugging layers by modifying the array below:
    const char* kDebugLayerNames[] = {
        "VK_LAYER_KHRONOS_validation",
        // Not included in standard_validation:
        // "VK_LAYER_LUNARG_api_dump",
        // "VK_LAYER_LUNARG_vktrace",
        // "VK_LAYER_LUNARG_screenshot",
    };

    // Check for each desired debug layer within the list of available layers. If we encounter a
    // match, verify that the layer was authored against a version >= the major Vulkan version we
    // are using before appending it to the list of layers to use. This ensures the validation
    // layers will have all the expected API entry points in order to intercept Vulkan calls.
    for (size_t i = 0; i < std::size(kDebugLayerNames); ++i) {
        for (size_t j = 0; j < availLayerCount; ++j) {
            if (!strcmp(kDebugLayerNames[i], availableLayers[j].layerName) &&
                majorSpecVersion <= remove_patch_version(availableLayers[j].specVersion)) {
                layersToUse.push_back(availableLayers[i]);
                layersToUseNames.push_back(kDebugLayerNames[i]);
            }
        }
    }
}

bool append_desired_available_layers(std::function<VkResult(VkLayerProperties*)> QueryLayers,
                                     uint32_t* availLayerCount,
                                     uint32_t majorSpecVersion,
                                     TArray<VkLayerProperties>& layersToEnable,
                                     TArray<const char*>& enabledLayerNames) {
    if (QueryLayers(nullptr) != VK_SUCCESS) {
        return false;
    }
    VkLayerProperties* availLayers = new VkLayerProperties[*availLayerCount];
    if (QueryLayers(availLayers) != VK_SUCCESS) {
        delete[] availLayers;
        return false;
    }

    append_desired_layers(
            availLayers, *availLayerCount, majorSpecVersion, layersToEnable, enabledLayerNames);

    delete[] availLayers;
    return true;
}

// Given a list of enabled layers and a ptr to the Vulkan driver call to query layer extension
// information, tack on to the list and quantity of available extensions.
bool add_enabled_layer_extensions_to_available_list(
        uint32_t* availExtCount,
        TArray<VkExtensionProperties>& availExtensions,
        const TArray<VkLayerProperties>& layersToEnable,
        std::function<VkResult(const char*, uint32_t*, VkExtensionProperties*)> getExtensions) {
    // Add extensions from layers we intend to enable to our list of available extensions
    for (const auto& layer : layersToEnable) {
        uint32_t layerAvailExtCount = 0;
        if (getExtensions(layer.layerName, &layerAvailExtCount, nullptr) != VK_SUCCESS) {
            return false;
        }

        // Pushing default entries that the query call can write over preserves TArray size
        // attribute accuracy as opposed to reserve calls which only update capacity (useful for
        // for/each loops, not having to separately track + pass around more size values, etc.).
        availExtensions.push_back_n(layerAvailExtCount);
        if (getExtensions(layer.layerName,
                          &layerAvailExtCount,
                          &availExtensions[*availExtCount]) != VK_SUCCESS) {
            return false;
        }
        *availExtCount += layerAvailExtCount;
    }

    return true;
}
#endif // SK_ENABLE_VK_LAYERS

bool should_include_extension(const char* extensionName) {
    const char* kExtensionsForTests[] = {
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
            VK_EXT_DEVICE_FAULT_EXTENSION_NAME,
            VK_EXT_LAYER_SETTINGS_EXTENSION_NAME,
            VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME,
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            // Currently only used by Ganesh:
            VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME,
            VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME, // Required for above extension if using 1.1
            VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME,
            // Below are all platform specific extensions. The name macros like we use above are
            // all defined in platform specific vulkan headers. We currently don't include these
            // headers as they are a little bit of a pain (e.g. windows headers requires including
            // <windows.h> which causes all sorts of fun annoyances/problems. So instead we are
            // just listing the strings these macros are defined to. This really shouldn't cause
            // any long term issues as the chances of the strings connected to the name macros
            // changing is next to zero.
            "VK_KHR_win32_surface",  // VK_KHR_WIN32_SURFACE_EXTENSION_NAME
            "VK_KHR_xcb_surface",    // VK_KHR_XCB_SURFACE_EXTENSION_NAME,
            "VK_KHR_android_surface",  // VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
    };

    for (size_t i = 0; i < std::size(kExtensionsForTests); i++) {
        if (!strcmp(extensionName, kExtensionsForTests[i])) {
            return true;
        }
    }

    return false;
}

// Checks a list of availExtensions, populating a list of all available extension names. If we see
// an extension we want to enable, add the extension and its name to the relvant enabled
// extensions + extension names lists.
void append_desired_extensions(TArray<VkExtensionProperties>& availableExtensions,
                               TArray<const char*>& availExtNames,
                               TArray<VkExtensionProperties>& extensionsToEnable,
                               std::vector<const char*>& enabledExtNames) {
    for (const auto& ext : availableExtensions) {
        availExtNames.push_back(ext.extensionName);
        if (should_include_extension(ext.extensionName)) {
            extensionsToEnable.push_back(ext);
            enabledExtNames.push_back(ext.extensionName);
        }
    }
}

// Provided a function ptr with which to perform the Vulkan query, populate the quantity and
// extension properties of all available extensions.
bool get_avail_extensions(
        std::function<VkResult(const char*, uint32_t*, VkExtensionProperties*)> QueryExtensions,
        TArray<VkExtensionProperties>& availExtensions,
        uint32_t* availExtCount) {
    if (QueryExtensions(nullptr, availExtCount, nullptr) != VK_SUCCESS) {
        return false;
    }

    // Pushing default entries that the query call can write over preserves TArray size attribute
    // accuracy as opposed to reserve calls which only update capacity (useful for for/each loops,
    // not having to separately track + pass around more size values, etc.).
    availExtensions.push_back_n(*availExtCount);
    if (QueryExtensions(nullptr, availExtCount, availExtensions.begin()) != VK_SUCCESS) {
        return false;
    }

    return true;
}

// Determine all available instance extensions, layers, and those layers' extensions. From that,
// decide which layers to include. Also decide which instance extensions to enable, making sure to
// allow Skia to add to that list. Returns true upon success; false otherwise.
bool init_instance_extensions_and_layers(PFN_vkGetInstanceProcAddr getInstProc,
                                         uint32_t majorSpecVersion,
                                         TArray<VkExtensionProperties>& availableInstExts,
                                         TArray<const char*>& availInstExtNames,
                                         TArray<VkExtensionProperties>& instExtensionsToEnable,
                                         std::vector<const char*>& enabledInstExtNames,
                                         TArray<VkLayerProperties>& instLayersToEnable,
                                         TArray<const char*>& enabledInstLayerNames,
                                         skgpu::VulkanPreferredFeatures& skiaFeatures) {
    // First populate the list of all available extensions
    ACQUIRE_VK_INST_PROC_LOCAL(EnumerateInstanceExtensionProperties, VK_NULL_HANDLE);
    uint32_t availExtCount = 0;
    auto getExtensions = [&](const char* layerName,
                             uint32_t* extensionCount,
                             VkExtensionProperties* extensionList) {
        return grVkEnumerateInstanceExtensionProperties(layerName, extensionCount, extensionList);
    };
    if (!get_avail_extensions(getExtensions, availableInstExts, &availExtCount)) {
        return false;
    }

#if defined SK_ENABLE_VK_LAYERS
    ACQUIRE_VK_INST_PROC_LOCAL(EnumerateInstanceLayerProperties, VK_NULL_HANDLE);
    uint32_t availLayerCount = 0;

    auto getLayers = [&](VkLayerProperties* layersList) {
        return grVkEnumerateInstanceLayerProperties(&availLayerCount, layersList);
    };
    if (!append_desired_available_layers(getLayers,
                                         &availLayerCount,
                                         majorSpecVersion,
                                         instLayersToEnable,
                                         enabledInstLayerNames)) {
        return false;
    }

    if (!add_enabled_layer_extensions_to_available_list(
            &availExtCount, availableInstExts, instLayersToEnable, getExtensions)) {
        return false;
    }
#endif

    // Now that we have compiled a list of all available instance extensions, populate a list of
    // ones we actually want to enable.
    append_desired_extensions(
            availableInstExts, availInstExtNames, instExtensionsToEnable, enabledInstExtNames);

    // Allow Skia the chance to add to the list of enabled instance extensions.
    skiaFeatures.addToInstanceExtensions(
            availableInstExts.data(), availExtCount, enabledInstExtNames);

    return true;
}

void setup_feature_query(TestVkFeatures& testFeatures, bool isProtected) {
    // Note: Any structs chained on to the feature query must stay in scope until vkCreateDevice.
    // This is why these structs are located in TestVkFeatures and passed into this function.

    // Initialize physical device feature query struct
    testFeatures.deviceFeatures = {};
    testFeatures.deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

    // Add features that would be useful to enable for testing to the feature query.
    void** tailPNext = &testFeatures.deviceFeatures.pNext;

    // If |isProtected| is given, attach that first
    testFeatures.protectedMemoryFeatures = {};
    if (isProtected) {
        testFeatures.protectedMemoryFeatures.sType =
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES;
        *tailPNext = &testFeatures.protectedMemoryFeatures;
        tailPNext = &testFeatures.protectedMemoryFeatures.pNext;
    }
}

bool init_device_extensions_and_layers(const skgpu::VulkanGetProc& getProc,
                                       uint32_t majorSpecVersion,
                                       VkInstance inst,
                                       VkPhysicalDevice physDev,
                                       TArray<VkExtensionProperties>& availableDevExts,
                                       TArray<const char*>& availDevExtNames,
                                       TArray<VkExtensionProperties>& devExtsToEnable,
                                       std::vector<const char*>& enabledDevExtNames,
                                       TArray<VkLayerProperties>& devLayersToEnable,
                                       TArray<const char*>& enabledDevLayerNames,
                                       TestVkFeatures& testFeatures,
                                       bool isProtected) {
    // First populate the list of all available extensions
    GET_PROC_LOCAL(EnumerateDeviceExtensionProperties, inst, VK_NULL_HANDLE);
    uint32_t availExtCount = 0;
    auto getExtensions = [&](const char* layerName,
                             uint32_t* extensionCount,
                             VkExtensionProperties* extensionList) {
        return EnumerateDeviceExtensionProperties(
                physDev, layerName, extensionCount, extensionList);
    };
    if (!get_avail_extensions(getExtensions, availableDevExts, &availExtCount)) {
        return false;
    }

#if defined SK_ENABLE_VK_LAYERS
    GET_PROC_LOCAL(EnumerateDeviceLayerProperties, inst, VK_NULL_HANDLE);
    uint32_t availLayerCount = 0;

    auto getLayers = [&](VkLayerProperties* layersList) {
        return EnumerateDeviceLayerProperties(physDev, &availLayerCount, layersList);
    };
    append_desired_available_layers(
            getLayers, &availLayerCount, majorSpecVersion, devLayersToEnable, enabledDevLayerNames);

    if (!add_enabled_layer_extensions_to_available_list(
            &availExtCount, availableDevExts, devLayersToEnable, getExtensions)) {
        return false;
    }
#endif

    // Now that we have compiled a list of all available device extensions, populate a list of
    // ones we actually want to enable.
    append_desired_extensions(
            availableDevExts, availDevExtNames, devExtsToEnable, enabledDevExtNames);

    // First, add any features we want for testing to the query. Then allow Skia to add to it.
    setup_feature_query(testFeatures, isProtected);

    testFeatures.skiaFeatures.addFeaturesToQuery(
            availableDevExts.data(), availExtCount, testFeatures.deviceFeatures);

    return true;
}
} // anonymous namespace

bool CreateVkBackendContext(PFN_vkGetInstanceProcAddr getInstProc,
                            skgpu::VulkanBackendContext* ctx,
                            skgpu::VulkanExtensions* extensions,
                            TestVkFeatures* testVkFeatures,
                            VkDebugUtilsMessengerEXT* debugMessenger,
                            uint32_t* presentQueueIndexPtr,
                            const CanPresentFn& canPresent,
                            bool isProtected) {
    if (!getInstProc || !testVkFeatures) {
        return false;
    }

    VkResult err;

    ACQUIRE_VK_INST_PROC_NOCHECK(EnumerateInstanceVersion, VK_NULL_HANDLE);
    uint32_t instanceVersion = 0;
    // Vulkan 1.1 is required, so vkEnumerateInstanceVersion should always be available.
    SkASSERT(grVkEnumerateInstanceVersion != nullptr);
    err = grVkEnumerateInstanceVersion(&instanceVersion);
    if (err) {
        SkDebugf("failed to enumerate instance version. Err: %d\n", err);
        return false;
    }
    SkASSERT(instanceVersion >= VK_API_VERSION_1_1);

    // We could set the Vulkan API version to be the newest version supported by Skia, but for now,
    // set it to 1.1 (the most common Vulkan version on Android devices).
    static const uint32_t kApiVersion = VK_API_VERSION_1_1;
    instanceVersion = std::min(instanceVersion, kApiVersion);

    STArray<2, VkPhysicalDevice> physDevs;
    VkDevice device;
    VkInstance inst = VK_NULL_HANDLE;

    static constexpr VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO,
                                                   /*pNext=*/nullptr,
                                                   "vkTestApp",
                                                   /*applicationVersion=*/0,
                                                   "vkTestEngine",
                                                   /*engineVerison=*/0,
                                                   kApiVersion };
    // Initialize VulkanPreferredFeatures using whatever API value matches that of the version
    // assigned to VkApplicationInfo.
    testVkFeatures->skiaFeatures.init(kApiVersion);

    // Determine all available instance extensions - which is used by Skia's VulkanPreferredFeatures
    // and VulkanExtensions - in addition to which instance extensions we want to enable for
    // testing. Skia and/or Vulkan API calls eventually expect to be provided with a list of
    // extension/layer *names*, so populate those lists at the same time.
    TArray<VkExtensionProperties> availableInstExts;
    TArray<const char*> availInstExtNames;
    TArray<VkExtensionProperties> instExtsToEnable;
    std::vector<const char*> enabledInstExtNames;
    // For layers, however, we can get away with only tracking which ones we want to enable
    // since Skia doesn't currently parse all available layers in order to make any decisions.
    TArray<VkLayerProperties> instLayersToEnable;
    TArray<const char*> enabledInstLayerNames;
    if (!init_instance_extensions_and_layers(getInstProc,
                                             remove_patch_version(instanceVersion),
                                             availableInstExts,
                                             availInstExtNames,
                                             instExtsToEnable,
                                             enabledInstExtNames,
                                             instLayersToEnable,
                                             enabledInstLayerNames,
                                             testVkFeatures->skiaFeatures)) {
        return false;
    }

    // Set up VkInstanceCreateInfo
    VkInstanceCreateInfo createInstanceInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                                /*pNext=*/nullptr,
                                                /*flags=*/0,
                                                &appInfo,
                                                (uint32_t)enabledInstLayerNames.size(),
                                                enabledInstLayerNames.data(),
                                                (uint32_t)enabledInstExtNames.size(),
                                                enabledInstExtNames.data() };
    bool hasDebugExtension = false;
    *debugMessenger = VK_NULL_HANDLE;

#ifdef SK_ENABLE_VK_LAYERS
    bool hasLayerSettingsExt = false;
    for (size_t i = 0;
         i < enabledInstExtNames.size() && !hasDebugExtension && !hasLayerSettingsExt; ++i) {
        if (!strcmp(enabledInstExtNames[i], VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
            hasDebugExtension = true;
        } else if (!strcmp(enabledInstExtNames[i], VK_EXT_LAYER_SETTINGS_EXTENSION_NAME)) {
            hasLayerSettingsExt = true;
        }
    }

    // Fine grain control of validation layer features
    const char* name = "VK_LAYER_KHRONOS_validation";
    const VkBool32 settingValidateCore = VK_TRUE;
    // Syncval is disabled for now, but would be useful to enable eventually.
    const VkBool32 settingValidateSync = VK_FALSE;
    const VkBool32 settingThreadSafety = VK_TRUE;
    // Shader validation could be useful (previously broken on Android, might already be fixed:
    // http://anglebug.com/42265520).
    const VkBool32 settingCheckShaders = VK_FALSE;
    // If syncval is enabled, submit time validation could stay disabled due to performance issues:
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7285
    const VkBool32 settingSyncvalSubmitTimeValidation = VK_FALSE;
    // Extra properties in syncval make it easier to filter the messages.
    const VkBool32 settingSyncvalMessageExtraProperties = VK_TRUE;
    const VkLayerSettingEXT layerSettings[] = {
            {name, "validate_core", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &settingValidateCore},
            {name, "validate_sync", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &settingValidateSync},
            {name, "thread_safety", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &settingThreadSafety},
            {name, "check_shaders", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &settingCheckShaders},
            {name,
             "syncval_submit_time_validation",
             VK_LAYER_SETTING_TYPE_BOOL32_EXT,
             1,
             &settingSyncvalSubmitTimeValidation},
            {name,
             "syncval_message_extra_properties",
             VK_LAYER_SETTING_TYPE_BOOL32_EXT,
             1,
             &settingSyncvalMessageExtraProperties},
    };
    VkLayerSettingsCreateInfoEXT layerSettingsCreateInfo = {};
    layerSettingsCreateInfo.sType = VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT;
    layerSettingsCreateInfo.settingCount = static_cast<uint32_t>(std::size(layerSettings));
    layerSettingsCreateInfo.pSettings = layerSettings;
    if (hasDebugExtension && hasLayerSettingsExt) {
        createInstanceInfo.pNext = &layerSettingsCreateInfo;
    }
#endif

    ACQUIRE_VK_INST_PROC(CreateInstance, VK_NULL_HANDLE);
    err = grVkCreateInstance(&createInstanceInfo, nullptr, &inst);
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
    if (hasDebugExtension) {
        VkDebugUtilsMessengerCreateInfoEXT messengerInfo = {};

        constexpr VkDebugUtilsMessageSeverityFlagsEXT kSeveritiesToLog =
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;

        constexpr VkDebugUtilsMessageTypeFlagsEXT kMessagesToLog =
                VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

        messengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        messengerInfo.messageSeverity = kSeveritiesToLog;
        messengerInfo.messageType = kMessagesToLog;
        messengerInfo.pfnUserCallback = &DebugUtilsMessenger;

        ACQUIRE_VK_PROC(CreateDebugUtilsMessengerEXT, inst, VK_NULL_HANDLE);
        // Register the callback
        grVkCreateDebugUtilsMessengerEXT(inst, &messengerInfo, nullptr, debugMessenger);
    }
#endif

    ACQUIRE_VK_PROC(EnumeratePhysicalDevices, inst, VK_NULL_HANDLE);
    ACQUIRE_VK_PROC(GetPhysicalDeviceProperties, inst, VK_NULL_HANDLE);
    ACQUIRE_VK_PROC(GetPhysicalDeviceQueueFamilyProperties, inst, VK_NULL_HANDLE);
    ACQUIRE_VK_PROC(CreateDevice, inst, VK_NULL_HANDLE);
    ACQUIRE_VK_PROC(GetDeviceQueue, inst, VK_NULL_HANDLE);
    ACQUIRE_VK_PROC(DeviceWaitIdle, inst, VK_NULL_HANDLE);
    ACQUIRE_VK_PROC(DestroyDevice, inst, VK_NULL_HANDLE);

    uint32_t gpuCount;
    err = grVkEnumeratePhysicalDevices(inst, &gpuCount, nullptr);
    if (err) {
        SkDebugf("vkEnumeratePhysicalDevices failed: %d\n", err);
        destroy_instance(getInstProc, inst, debugMessenger, hasDebugExtension);
        return false;
    }
    if (!gpuCount) {
        SkDebugf("vkEnumeratePhysicalDevices returned no supported devices.\n");
        destroy_instance(getInstProc, inst, debugMessenger, hasDebugExtension);
        return false;
    }
    // Allocate enough storage for all available physical devices. We should be able to just ask for
    // the first one, but a bug in RenderDoc (https://github.com/baldurk/renderdoc/issues/2766)
    // will smash the stack if we do that.
    physDevs.resize(gpuCount);
    err = grVkEnumeratePhysicalDevices(inst, &gpuCount, physDevs.data());
    if (err) {
        SkDebugf("vkEnumeratePhysicalDevices failed: %d\n", err);
        destroy_instance(getInstProc, inst, debugMessenger, hasDebugExtension);
        return false;
    }
    // We just use the first physical device.
    // TODO: find best match for our needs
    VkPhysicalDevice physDev = physDevs.front();

    VkPhysicalDeviceProperties physDeviceProperties;
    grVkGetPhysicalDeviceProperties(physDev, &physDeviceProperties);
    uint32_t physDeviceVersion = std::min(physDeviceProperties.apiVersion, kApiVersion);

    // Now, for the device rather than the instance, repeat the process of gathering all available
    // extensions + their names, determining layers we want to enable for testing + their associated
    // available extensions, and determining which extensions we actually want to enable (+ their
    // names).
    TArray<VkExtensionProperties> availableDevExts;
    TArray<const char*> availDevExtNames;
    TArray<VkExtensionProperties> devExtsToEnable;
    std::vector<const char*> enabledDevExtNames;
    TArray<VkLayerProperties> devLayersToEnable;
    TArray<const char*> enabledDevLayerNames;
    if (!init_device_extensions_and_layers(getProc,
                                           remove_patch_version(physDeviceVersion),
                                           inst,
                                           physDev,
                                           availableDevExts,
                                           availDevExtNames,
                                           devExtsToEnable,
                                           enabledDevExtNames,
                                           devLayersToEnable,
                                           enabledDevLayerNames,
                                           *testVkFeatures,
                                           isProtected)) {
        destroy_instance(getInstProc, inst, debugMessenger, hasDebugExtension);
        return false;
    }

    // Now that we have determined all available instance and device extensions, we can initialize
    // VulkanExtensions.
    extensions->init(getProc,
                     inst,
                     physDev,
                     (uint32_t)availInstExtNames.size(),
                     availInstExtNames.data(),
                     (uint32_t)availDevExtNames.size(),
                     availDevExtNames.data());

    // Finally, perform the query.
    ACQUIRE_VK_INST_PROC(GetPhysicalDeviceFeatures2, inst);
    grVkGetPhysicalDeviceFeatures2(physDev, &testVkFeatures->deviceFeatures);

    if (isProtected && !testVkFeatures->protectedMemoryFeatures.protectedMemory) {
        SkDebugf("Device does not support protected memory.\n");
        destroy_instance(getInstProc, inst, debugMessenger, hasDebugExtension);
        return false;
    }

    // We can manually disable any undesired features here. Currently, we only disable robust
    // buffer access which can negatively impact performance on some GPUs.
    testVkFeatures->deviceFeatures.features.robustBufferAccess = VK_FALSE;

    // Allow Skia to enable available features
    testVkFeatures->skiaFeatures.addFeaturesToEnable(enabledDevExtNames,
                                                     testVkFeatures->deviceFeatures);

    // Query for queue property size, then fetch the properties.
    uint32_t queueCount;
    grVkGetPhysicalDeviceQueueFamilyProperties(physDev, &queueCount, nullptr);
    if (!queueCount) {
        SkDebugf("vkGetPhysicalDeviceQueueFamilyProperties returned no queues.\n");
        destroy_instance(getInstProc, inst, debugMessenger, hasDebugExtension);
        return false;
    }

    SkAutoMalloc queuePropsAlloc(queueCount * sizeof(VkQueueFamilyProperties));
    VkQueueFamilyProperties* queueProps = (VkQueueFamilyProperties*)queuePropsAlloc.get();
    grVkGetPhysicalDeviceQueueFamilyProperties(physDev, &queueCount, queueProps);

    // Iterate to find the graphics queue
    uint32_t graphicsQueueIndex = queueCount;
    for (uint32_t i = 0; i < queueCount; i++) {
        if (queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsQueueIndex = i;
            break;
        }
    }
    if (graphicsQueueIndex == queueCount) {
        SkDebugf("Could not find any supported graphics queues.\n");
        destroy_instance(getInstProc, inst, debugMessenger, hasDebugExtension);
        return false;
    }

    // Iterate to find the present queue, if needed
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
            destroy_instance(getInstProc, inst, debugMessenger, hasDebugExtension);
            return false;
        }
        *presentQueueIndexPtr = presentQueueIndex;
    } else {
        // Just setting this so we end up make a single queue for graphics since there was no
        // request for a present queue.
        presentQueueIndex = graphicsQueueIndex;
    }

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

    const VkDeviceCreateInfo deviceInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                                            &testVkFeatures->deviceFeatures,
                                            /*flags=*/0,
                                            queueInfoCount,
                                            queueInfo,
                                            (uint32_t) enabledDevLayerNames.size(),
                                            enabledDevLayerNames.data(),
                                            (uint32_t) enabledDevExtNames.size(),
                                            enabledDevExtNames.data(),
                                            /*ppEnabledFeatures*/nullptr };
    {
#if defined(SK_ENABLE_SCOPED_LSAN_SUPPRESSIONS)
        // skbug.com/40040003
        __lsan::ScopedDisabler lsanDisabler;
#endif
        err = grVkCreateDevice(physDev, &deviceInfo, nullptr, &device);
    }
    if (err) {
        SkDebugf("CreateDevice failed: %d\n", err);
        destroy_instance(getInstProc, inst, debugMessenger, hasDebugExtension);
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

    skgpu::VulkanInterface interface = skgpu::VulkanInterface(
            getProc, inst, device, instanceVersion, physDeviceVersion, extensions);
    SkASSERT(interface.validate(instanceVersion, physDeviceVersion, extensions));

    sk_sp<skgpu::VulkanMemoryAllocator> memoryAllocator = VkTestMemoryAllocator::Make(
            inst, physDev, device, physDeviceVersion, extensions, &interface);

    ctx->fInstance = inst;
    ctx->fPhysicalDevice = physDev;
    ctx->fDevice = device;
    ctx->fQueue = queue;
    ctx->fGraphicsQueueIndex = graphicsQueueIndex;
    ctx->fMaxAPIVersion = kApiVersion;
    ctx->fVkExtensions = extensions;
    ctx->fDeviceFeatures2 = &testVkFeatures->deviceFeatures;
    ctx->fGetProc = getProc;
    ctx->fProtectedContext = skgpu::Protected(isProtected);
    ctx->fMemoryAllocator = memoryAllocator;

    return true;
}

}  // namespace sk_gpu_test

#endif
