/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "vk/GrVkBackendContext.h"
#include "vk/GrVkInterface.h"
#include "vk/GrVkUtil.h"

////////////////////////////////////////////////////////////////////////////////
// Helper code to set up Vulkan context objects

#ifdef ENABLE_VK_LAYERS
const char* kEnabledLayerNames[] = {
    // elements of VK_LAYER_LUNARG_standard_validation
    "VK_LAYER_LUNARG_threading",
    "VK_LAYER_LUNARG_param_checker",
    "VK_LAYER_LUNARG_device_limits",
    "VK_LAYER_LUNARG_object_tracker",
    "VK_LAYER_LUNARG_image",
    "VK_LAYER_LUNARG_mem_tracker",
    "VK_LAYER_LUNARG_draw_state",
    "VK_LAYER_LUNARG_swapchain",
    "VK_LAYER_GOOGLE_unique_objects",
    // not included in standard_validation
    //"VK_LAYER_LUNARG_api_dump",
};
const char* kEnabledInstanceExtensionNames[] = {
    VK_EXT_DEBUG_REPORT_EXTENSION_NAME
};

bool verify_instance_layers() {
    // make sure we can actually use the extensions and layers above
    uint32_t extensionCount;
    VkResult res = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    if (VK_SUCCESS != res) {
        return false;
    }
    VkExtensionProperties* extensions = new VkExtensionProperties[extensionCount];
    res = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions);
    if (VK_SUCCESS != res) {
        return false;
    }
    int instanceExtensionsFound = 0;
    for (uint32_t j = 0; j < ARRAYSIZE(kEnabledInstanceExtensionNames); ++j) {
        for (uint32_t i = 0; i < extensionCount; ++i) {
            if (!strncmp(extensions[i].extensionName, kEnabledInstanceExtensionNames[j],
                         strlen(kEnabledInstanceExtensionNames[j]))) {
                ++instanceExtensionsFound;
                break;
            }
        }
    }
    delete[] extensions;

    uint32_t layerCount;
    res = vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    if (VK_SUCCESS != res) {
        return false;
    }
    VkLayerProperties* layers = new VkLayerProperties[layerCount];
    res = vkEnumerateInstanceLayerProperties(&layerCount, layers);
    if (VK_SUCCESS != res) {
        return false;
    }
    int instanceLayersFound = 0;
    for (uint32_t j = 0; j < ARRAYSIZE(kEnabledLayerNames); ++j) {
        for (uint32_t i = 0; i < layerCount; ++i) {
            if (!strncmp(layers[i].layerName, kEnabledLayerNames[j],
                         strlen(kEnabledLayerNames[j]))) {
                ++instanceLayersFound;
                break;
            }
        }
    }
    delete[] layers;

    return instanceExtensionsFound == ARRAYSIZE(kEnabledInstanceExtensionNames) &&
           instanceLayersFound == ARRAYSIZE(kEnabledLayerNames);
}

bool verify_device_layers(VkPhysicalDevice physDev) {
    uint32_t layerCount;
    VkResult res = vkEnumerateDeviceLayerProperties(physDev, &layerCount, nullptr);
    if (VK_SUCCESS != res) {
        return false;
    }
    VkLayerProperties* layers = new VkLayerProperties[layerCount];
    res = vkEnumerateDeviceLayerProperties(physDev, &layerCount, layers);
    if (VK_SUCCESS != res) {
        return false;
    }
    int deviceLayersFound = 0;
    for (uint32_t j = 0; j < ARRAYSIZE(kEnabledLayerNames); ++j) {
        for (uint32_t i = 0; i < layerCount; ++i) {
            if (!strncmp(layers[i].layerName, kEnabledLayerNames[j],
                         strlen(kEnabledLayerNames[j]))) {
                ++deviceLayersFound;
                break;
            }
        }
    }
    delete[] layers;

    return deviceLayersFound == ARRAYSIZE(kEnabledLayerNames);
}
#endif

// Create the base Vulkan objects needed by the GrVkGpu object
const GrVkBackendContext* GrVkBackendContext::Create() {
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

    const char** enabledLayerNames = nullptr;
    int enabledLayerCount = 0;
    const char** enabledInstanceExtensionNames = nullptr;
    int enabledInstanceExtensionCount = 0;
#ifdef ENABLE_VK_LAYERS
    if (verify_instance_layers()) {
        enabledLayerNames = kEnabledLayerNames;
        enabledLayerCount = ARRAYSIZE(kEnabledLayerNames);
        enabledInstanceExtensionNames = kEnabledInstanceExtensionNames;
        enabledInstanceExtensionCount = ARRAYSIZE(kEnabledInstanceExtensionNames);
    }
#endif

    const VkInstanceCreateInfo instance_create = {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, // sType
        nullptr,                                // pNext
        0,                                      // flags
        &app_info,                              // pApplicationInfo
        enabledLayerCount,                      // enabledLayerNameCount
        enabledLayerNames,                      // ppEnabledLayerNames
        enabledInstanceExtensionCount,          // enabledExtensionNameCount
        enabledInstanceExtensionNames,          // ppEnabledExtensionNames
    };

    err = vkCreateInstance(&instance_create, nullptr, &inst);
    if (err < 0) {
        SkDebugf("vkCreateInstance failed: %d\n", err);
        SkFAIL("failing");
    }

    uint32_t gpuCount;
    err = vkEnumeratePhysicalDevices(inst, &gpuCount, nullptr);
    if (err) {
        SkDebugf("vkEnumeratePhysicalDevices failed: %d\n", err);
        SkFAIL("failing");
    }
    SkASSERT(gpuCount > 0);
    // Just returning the first physical device instead of getting the whole array.
    // TODO: find best match for our needs
    gpuCount = 1;
    err = vkEnumeratePhysicalDevices(inst, &gpuCount, &physDev);
    if (err) {
        SkDebugf("vkEnumeratePhysicalDevices failed: %d\n", err);
        SkFAIL("failing");
    }

    // query to get the initial queue props size
    uint32_t queueCount;
    vkGetPhysicalDeviceQueueFamilyProperties(physDev, &queueCount, nullptr);
    SkASSERT(queueCount >= 1);

    SkAutoMalloc queuePropsAlloc(queueCount * sizeof(VkQueueFamilyProperties));
    // now get the actual queue props
    VkQueueFamilyProperties* queueProps = (VkQueueFamilyProperties*)queuePropsAlloc.get();

    vkGetPhysicalDeviceQueueFamilyProperties(physDev, &queueCount, queueProps);

    // iterate to find the graphics queue
    uint32_t graphicsQueueIndex = -1;
    for (uint32_t i = 0; i < queueCount; i++) {
        if (queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsQueueIndex = i;
            break;
        }
    }
    SkASSERT(graphicsQueueIndex < queueCount);

#ifdef ENABLE_VK_LAYERS
    // unlikely that the device will have different layers than the instance, but good to check
    if (!verify_device_layers(physDev)) {
        enabledLayerNames = nullptr;
        enabledLayerCount = 0;
    }
#endif

    float queuePriorities[1] = { 0.0 };
    // Here we assume no need for swapchain queue
    // If one is needed, the client will need its own setup code
    const VkDeviceQueueCreateInfo queueInfo = {
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, // sType
        nullptr,                                    // pNext
        0,                                          // VkDeviceQueueCreateFlags
        graphicsQueueIndex,                         // queueFamilyIndex
        1,                                          // queueCount
        queuePriorities,                            // pQueuePriorities
    };
    const VkDeviceCreateInfo deviceInfo = {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,  // sType
        nullptr,                               // pNext
        0,                                     // VkDeviceCreateFlags
        1,                                     // queueCreateInfoCount
        &queueInfo,                            // pQueueCreateInfos
        enabledLayerCount,                     // layerCount
        enabledLayerNames,                     // ppEnabledLayerNames
        0,                                     // extensionCount
        nullptr,                               // ppEnabledExtensionNames
        nullptr                                // ppEnabledFeatures
    };

    err = vkCreateDevice(physDev, &deviceInfo, nullptr, &device);
    if (err) {
        SkDebugf("CreateDevice failed: %d\n", err);
        return nullptr;
    }

    VkQueue queue;
    vkGetDeviceQueue(device, graphicsQueueIndex, 0, &queue);

    GrVkBackendContext* ctx = new GrVkBackendContext();
    ctx->fInstance = inst;
    ctx->fPhysicalDevice = physDev;
    ctx->fDevice = device;
    ctx->fQueue = queue;
    ctx->fQueueFamilyIndex = graphicsQueueIndex;
    ctx->fInterface.reset(GrVkCreateInterface(inst, physDev, device));
  
    return ctx;
}

GrVkBackendContext::~GrVkBackendContext() {
    vkDestroyDevice(fDevice, nullptr);
    fDevice = VK_NULL_HANDLE;
    vkDestroyInstance(fInstance, nullptr);
    fInstance = VK_NULL_HANDLE;
}
