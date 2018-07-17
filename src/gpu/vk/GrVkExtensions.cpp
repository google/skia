/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "vk/GrVkExtensions.h"

// Can remove this once we get rid of the extension flags.
#include "vk/GrVkBackendContext.h"

#include "SkTSearch.h"
#include "SkTSort.h"

// finds the index of ext in infos or a negative result if ext is not found.
static int find_info(const SkTArray<GrVkExtensions::Info>& infos, const char ext[]) {
    if (infos.empty()) {
        return -1;
    }
    SkString extensionStr(ext);
    GrVkExtensions::Info::Less less;
    int idx = SkTSearch<GrVkExtensions::Info, SkString, GrVkExtensions::Info::Less>(
            &infos.front(), infos.count(), extensionStr, sizeof(GrVkExtensions::Info),
            less);
    return idx;
}

namespace { // This cannot be static because it is used as a template parameter.
inline bool extension_compare(const GrVkExtensions::Info& a, const GrVkExtensions::Info& b) {
    return strcmp(a.fName.c_str(), b.fName.c_str()) < 0;
}
}

void GrVkExtensions::init(GrVkGetProc getProc,
                          VkInstance instance,
                          VkPhysicalDevice physDev,
                          uint32_t instanceExtensionCount,
                          const char* const* instanceExtensions,
                          uint32_t deviceExtensionCount,
                          const char* const* deviceExtensions) {
    SkTLessFunctionToFunctorAdaptor<GrVkExtensions::Info, extension_compare> cmp;

    for (uint32_t i = 0; i < instanceExtensionCount; ++i) {
        const char* extension = instanceExtensions[i];
        // if not already in the list, add it
        if (find_info(fExtensions, extension) < 0) {
            fExtensions.push_back() = Info(extension);
            SkTQSort(&fExtensions.front(), &fExtensions.back(), cmp);
        }
    }
    for (uint32_t i = 0; i < deviceExtensionCount; ++i) {
        const char* extension = deviceExtensions[i];
        // if not already in the list, add it
        if (find_info(fExtensions, extension) < 0) {
            fExtensions.push_back() = Info(extension);
            SkTQSort(&fExtensions.front(), &fExtensions.back(), cmp);
        }
    }
    this->getSpecVersions(getProc, instance, physDev);
}

void GrVkExtensions::init(GrVkGetProc getProc, VkInstance instance, uint32_t extensionFlags) {
    SkTLessFunctionToFunctorAdaptor<GrVkExtensions::Info, extension_compare> cmp;

    SkTArray<const char*> extensionNames;
    GetExtensionArrayFromFlags(extensionFlags, &extensionNames);
    for (int i = 0; i < extensionNames.count(); ++i) {
        // if not already in the list, add it
        if (find_info(fExtensions, extensionNames[i]) < 0) {
            fExtensions.push_back() = Info(extensionNames[i]);
            SkTQSort(&fExtensions.front(), &fExtensions.back(), cmp);
        }
    }
    this->getSpecVersions(getProc, instance, VK_NULL_HANDLE);
}

#define GET_PROC(F, inst)                                                        \
        PFN_vk##F grVk##F = (PFN_vk ## F) getProc("vk" #F, inst, VK_NULL_HANDLE)


void GrVkExtensions::getSpecVersions(GrVkGetProc getProc, VkInstance instance,
                                     VkPhysicalDevice physDevice) {
    // We grab all the extensions for the VkInstance and VkDevice so we can look up what spec
    // version each of the supported extensions are. We do not grab the extensions for layers
    // because we don't know what layers the client has enabled and in general we don't do anything
    // special for those extensions.

    if (instance == VK_NULL_HANDLE) {
        return;
    }
    GET_PROC(EnumerateInstanceExtensionProperties, VK_NULL_HANDLE);
    SkASSERT(grVkEnumerateInstanceExtensionProperties);

    VkResult res;
    // instance extensions
    uint32_t extensionCount = 0;
    res = grVkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    if (VK_SUCCESS != res) {
        return;
    }
    VkExtensionProperties* extensions = new VkExtensionProperties[extensionCount];
    res = grVkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions);
    if (VK_SUCCESS != res) {
        delete[] extensions;
        return;
    }
    for (uint32_t i = 0; i < extensionCount; ++i) {
        int idx = find_info(fExtensions, extensions[i].extensionName);
        if (idx >= 0) {
            fExtensions[idx].fSpecVersion = extensions[i].specVersion;
        }
    }
    delete[] extensions;

    if (physDevice == VK_NULL_HANDLE) {
        return;
    }
    GET_PROC(EnumerateDeviceExtensionProperties, instance);
    SkASSERT(grVkEnumerateDeviceExtensionProperties);

    // device extensions
    extensionCount = 0;
    res = grVkEnumerateDeviceExtensionProperties(physDevice, nullptr, &extensionCount, nullptr);
    if (VK_SUCCESS != res) {
        return;
    }
    extensions = new VkExtensionProperties[extensionCount];
    res = grVkEnumerateDeviceExtensionProperties(physDevice, nullptr, &extensionCount, extensions);
    if (VK_SUCCESS != res) {
        delete[] extensions;
        return;
    }
    for (uint32_t i = 0; i < extensionCount; ++i) {
        int idx = find_info(fExtensions, extensions[i].extensionName);
        if (idx >= 0) {
            fExtensions[idx].fSpecVersion = extensions[i].specVersion;
        }
    }
    delete[] extensions;
}

bool GrVkExtensions::hasExtension(const char ext[], uint32_t minVersion) const {
    int idx = find_info(fExtensions, ext);
    return  idx >= 0 && fExtensions[idx].fSpecVersion >= minVersion;
}

void GrVkExtensions::GetExtensionArrayFromFlags(uint32_t extensionFlags,
                                                SkTArray<const char*>* extensions) {
#ifdef SK_ENABLE_VK_LAYERS
    if (extensionFlags & kEXT_debug_report_GrVkExtensionFlag) {
        extensions->push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }
#endif
    if (extensionFlags & kKHR_surface_GrVkExtensionFlag) {
        extensions->push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    }
    if (extensionFlags & kKHR_swapchain_GrVkExtensionFlag) {
        extensions->push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }
#ifdef SK_BUILD_FOR_WIN
    if (extensionFlags & kKHR_win32_surface_GrVkExtensionFlag) {
        extensions->push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
    }
#elif defined(SK_BUILD_FOR_ANDROID)
    if (extensionFlags & kKHR_android_surface_GrVkExtensionFlag) {
        extensions->push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
    }
#elif defined(SK_BUILD_FOR_UNIX) && !defined(__Fuchsia__)
    if (extensionFlags & kKHR_xcb_surface_GrVkExtensionFlag) {
        extensions->push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
    }
#endif
    // Device extensions
    if (extensionFlags & kKHR_swapchain_GrVkExtensionFlag) {
        extensions->push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }
    if (extensionFlags & kNV_glsl_shader_GrVkExtensionFlag) {
        extensions->push_back("VK_NV_glsl_shader");
    }
}

