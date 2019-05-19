/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#if SK_SUPPORT_GPU && defined(SK_VULKAN)

#include "include/gpu/vk/GrVkTypes.h"
#include "src/core/SkAutoMalloc.h"
#include "tests/Test.h"
#include "tools/gpu/vk/VkTestUtils.h"

#define ACQUIRE_VK_PROC_NOCHECK(name, instance, device) \
    PFN_vk##name grVk##name = reinterpret_cast<PFN_vk##name>(getProc("vk" #name, instance, device))

#define ACQUIRE_VK_PROC(name, instance, device)                                    \
    PFN_vk##name grVk##name =                                                      \
            reinterpret_cast<PFN_vk##name>(getProc("vk" #name, instance, device)); \
    do {                                                                           \
        if (grVk##name == nullptr) {                                               \
            if (device != VK_NULL_HANDLE) {                                        \
                destroy_instance(getProc, inst);                                   \
            }                                                                      \
            return;                                                                \
        }                                                                          \
    } while (0)

#define ACQUIRE_VK_PROC_LOCAL(name, instance, device)                              \
    PFN_vk##name grVk##name =                                                      \
            reinterpret_cast<PFN_vk##name>(getProc("vk" #name, instance, device)); \
    do {                                                                           \
        if (grVk##name == nullptr) {                                               \
            return;                                                                \
        }                                                                          \
    } while (0)

#define GET_PROC_LOCAL(F, inst, device) PFN_vk ## F F = (PFN_vk ## F) getProc("vk" #F, inst, device)

static void destroy_instance(GrVkGetProc getProc, VkInstance inst) {
    ACQUIRE_VK_PROC_LOCAL(DestroyInstance, inst, VK_NULL_HANDLE);
    grVkDestroyInstance(inst, nullptr);
}

// If the extension VK_EXT_GLOBAL_PRIORITY is supported, this test just tries to create a VkDevice
// using the various global priorities. The test passes if no errors are reported or the test
// doesn't crash.
DEF_GPUTEST(VulkanPriorityExtension, reporter, options) {
    PFN_vkGetInstanceProcAddr instProc;
    PFN_vkGetDeviceProcAddr devProc;
    if (!sk_gpu_test::LoadVkLibraryAndGetProcAddrFuncs(&instProc, &devProc)) {
        return;
    }
    auto getProc = [instProc, devProc](const char* proc_name,
                                       VkInstance instance, VkDevice device) {
        if (device != VK_NULL_HANDLE) {
            return devProc(device, proc_name);
        }
        return instProc(instance, proc_name);
    };

    VkResult err;

    ACQUIRE_VK_PROC_NOCHECK(EnumerateInstanceVersion, VK_NULL_HANDLE, VK_NULL_HANDLE);
    uint32_t instanceVersion = 0;
    if (!grVkEnumerateInstanceVersion) {
        instanceVersion = VK_MAKE_VERSION(1, 0, 0);
    } else {
        err = grVkEnumerateInstanceVersion(&instanceVersion);
        if (err) {
            ERRORF(reporter, "failed ot enumerate instance version. Err: %d", err);
            return;
        }
    }
    SkASSERT(instanceVersion >= VK_MAKE_VERSION(1, 0, 0));
    uint32_t apiVersion = VK_MAKE_VERSION(1, 0, 0);
    if (instanceVersion >= VK_MAKE_VERSION(1, 1, 0)) {
        // If the instance version is 1.0 we must have the apiVersion also be 1.0. However, if the
        // instance version is 1.1 or higher, we can set the apiVersion to be whatever the highest
        // api we may use in skia (technically it can be arbitrary). So for now we set it to 1.1
        // since that is the highest vulkan version.
        apiVersion = VK_MAKE_VERSION(1, 1, 0);
    }

    instanceVersion = SkTMin(instanceVersion, apiVersion);

    VkPhysicalDevice physDev;
    VkDevice device;
    VkInstance inst;

    const VkApplicationInfo app_info = {
        VK_STRUCTURE_TYPE_APPLICATION_INFO, // sType
        nullptr,                            // pNext
        "vktest",                           // pApplicationName
        0,                                  // applicationVersion
        "vktest",                           // pEngineName
        0,                                  // engineVersion
        apiVersion,                         // apiVersion
    };

    const VkInstanceCreateInfo instance_create = {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,    // sType
        nullptr,                                   // pNext
        0,                                         // flags
        &app_info,                                 // pApplicationInfo
        0,                                         // enabledLayerNameCount
        nullptr,                                   // ppEnabledLayerNames
        0,                                         // enabledExtensionNameCount
        nullptr,                                   // ppEnabledExtensionNames
    };

    ACQUIRE_VK_PROC(CreateInstance, VK_NULL_HANDLE, VK_NULL_HANDLE);
    err = grVkCreateInstance(&instance_create, nullptr, &inst);
    if (err < 0) {
        ERRORF(reporter, "Failed to create VkInstance");
        return;
    }

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
        ERRORF(reporter, "vkEnumeratePhysicalDevices failed: %d", err);
        destroy_instance(getProc, inst);
        return;
    }
    if (!gpuCount) {
        ERRORF(reporter, "vkEnumeratePhysicalDevices returned no supported devices.");
        destroy_instance(getProc, inst);
        return;
    }
    // Just returning the first physical device instead of getting the whole array.
    // TODO: find best match for our needs
    gpuCount = 1;
    err = grVkEnumeratePhysicalDevices(inst, &gpuCount, &physDev);
    // VK_INCOMPLETE is returned when the count we provide is less than the total device count.
    if (err && VK_INCOMPLETE != err) {
        ERRORF(reporter, "vkEnumeratePhysicalDevices failed: %d", err);
        destroy_instance(getProc, inst);
        return;
    }

    // query to get the initial queue props size
    uint32_t queueCount;
    grVkGetPhysicalDeviceQueueFamilyProperties(physDev, &queueCount, nullptr);
    if (!queueCount) {
        ERRORF(reporter, "vkGetPhysicalDeviceQueueFamilyProperties returned no queues.");
        destroy_instance(getProc, inst);
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
        ERRORF(reporter, "Could not find any supported graphics queues.");
        destroy_instance(getProc, inst);
        return;
    }

    GET_PROC_LOCAL(EnumerateDeviceExtensionProperties, inst, VK_NULL_HANDLE);
    GET_PROC_LOCAL(EnumerateDeviceLayerProperties, inst, VK_NULL_HANDLE);

    if (!EnumerateDeviceExtensionProperties ||
        !EnumerateDeviceLayerProperties) {
        destroy_instance(getProc, inst);
        return;
    }

    // device extensions
    // via Vulkan implementation and implicitly enabled layers
    uint32_t extensionCount = 0;
    err = EnumerateDeviceExtensionProperties(physDev, nullptr, &extensionCount, nullptr);
    if (VK_SUCCESS != err) {
        ERRORF(reporter, "Could not  enumerate device extension properties.");
        destroy_instance(getProc, inst);
        return;
    }
    VkExtensionProperties* extensions = new VkExtensionProperties[extensionCount];
    err = EnumerateDeviceExtensionProperties(physDev, nullptr, &extensionCount, extensions);
    if (VK_SUCCESS != err) {
        delete[] extensions;
        ERRORF(reporter, "Could not  enumerate device extension properties.");
        destroy_instance(getProc, inst);
        return;
    }
    bool hasPriorityExt = false;
    for (uint32_t i = 0; i < extensionCount; ++i) {
        if (!strcmp(extensions[i].extensionName, VK_EXT_GLOBAL_PRIORITY_EXTENSION_NAME)) {
            hasPriorityExt = true;
        }
    }
    delete[] extensions;

    if (!hasPriorityExt) {
        destroy_instance(getProc, inst);
        return;
    }

    const char* priorityExt = VK_EXT_GLOBAL_PRIORITY_EXTENSION_NAME;

    VkPhysicalDeviceFeatures deviceFeatures;
    grVkGetPhysicalDeviceFeatures(physDev, &deviceFeatures);

    // this looks like it would slow things down,
    // and we can't depend on it on all platforms
    deviceFeatures.robustBufferAccess = VK_FALSE;

    float queuePriorities[1] = { 0.0 };

    VkDeviceQueueGlobalPriorityCreateInfoEXT queuePriorityCreateInfo;
    queuePriorityCreateInfo.sType =
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_GLOBAL_PRIORITY_CREATE_INFO_EXT;
    queuePriorityCreateInfo.pNext = nullptr;

    VkDeviceQueueCreateInfo queueInfo = {
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, // sType
        &queuePriorityCreateInfo,                   // pNext
        0,                                          // VkDeviceQueueCreateFlags
        graphicsQueueIndex,                         // queueFamilyIndex
        1,                                          // queueCount
        queuePriorities,                            // pQueuePriorities
    };

    for (VkQueueGlobalPriorityEXT globalPriority : { VK_QUEUE_GLOBAL_PRIORITY_LOW_EXT,
                                                     VK_QUEUE_GLOBAL_PRIORITY_MEDIUM_EXT,
                                                     VK_QUEUE_GLOBAL_PRIORITY_HIGH_EXT,
                                                     VK_QUEUE_GLOBAL_PRIORITY_REALTIME_EXT }) {
        queuePriorityCreateInfo.globalPriority = globalPriority;

        const VkDeviceCreateInfo deviceInfo = {
            VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,        // sType
            nullptr,                                     // pNext
            0,                                           // VkDeviceCreateFlags
            1,                                           // queueCreateInfoCount
            &queueInfo,                                  // pQueueCreateInfos
            0,                                           // layerCount
            nullptr,                                     // ppEnabledLayerNames
            1,                                           // extensionCount
            &priorityExt,                                // ppEnabledExtensionNames
            &deviceFeatures                              // ppEnabledFeatures
        };

        err = grVkCreateDevice(physDev, &deviceInfo, nullptr, &device);

        if (err != VK_SUCCESS && err != VK_ERROR_NOT_PERMITTED_EXT) {
            ERRORF(reporter, "CreateDevice failed: %d, priority %d", err, globalPriority);
            destroy_instance(getProc, inst);
            continue;
        }
        if (err != VK_ERROR_NOT_PERMITTED_EXT) {
            grVkDestroyDevice(device, nullptr);
        }
    }
    destroy_instance(getProc, inst);
}

#endif
