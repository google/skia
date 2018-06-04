/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "vk/GrVkExtensions.h"

#include "GrVkUtil.h"
#include "vk/GrVkBackendContext.h"

#include "SkTSearch.h"
#include "SkTSort.h"

namespace { // This cannot be static because it is used as a template parameter.
inline bool extension_compare(const SkString& a, const SkString& b) {
    return strcmp(a.c_str(), b.c_str()) < 0;
}
}

// finds the index of ext in strings or a negative result if ext is not found.
static int find_string(const SkTArray<SkString>& strings, const char ext[]) {
    if (strings.empty()) {
        return -1;
    }
    SkString extensionStr(ext);
    int idx = SkTSearch<SkString, extension_compare>(&strings.front(),
                                                     strings.count(),
                                                     extensionStr,
                                                     sizeof(SkString));
    return idx;
}

GrVkExtensions::GrVkExtensions(uint32_t instanceExtensionCount,
                               const char* const* instanceExtensions,
                               uint32_t deviceExtensionCount,
                               const char* const* deviceExtensions)
        : fExtensionStrings(new SkTArray<SkString>) {
    SkTLessFunctionToFunctorAdaptor<SkString, extension_compare> cmp;

    for (uint32_t i = 0; i < instanceExtensionCount; ++i) {
        const char* extension = instanceExtensions[i];
        // if not already in the list, add it
        if (find_string(*fExtensionStrings, extension) < 0) {
            fExtensionStrings->push_back() = extension;
            SkTQSort(&fExtensionStrings->front(), &fExtensionStrings->back(), cmp);
        }
    }
    for (uint32_t i = 0; i < deviceExtensionCount; ++i) {
        const char* extension = deviceExtensions[i];
        // if not already in the list, add it
        if (find_string(*fExtensionStrings, extension) < 0) {
            fExtensionStrings->push_back() = extension;
            SkTQSort(&fExtensionStrings->front(), &fExtensionStrings->back(), cmp);
        }
    }
}

GrVkExtensions::GrVkExtensions(uint32_t extensionFlags)
        : fExtensionStrings(new SkTArray<SkString>) {
    SkTLessFunctionToFunctorAdaptor<SkString, extension_compare> cmp;

    SkTArray<const char*> extensionNames;
    GetExtensionArrayFromFlags(extensionFlags, &extensionNames);
    for (int i = 0; i < extensionNames.count(); ++i) {
        // if not already in the list, add it
        if (find_string(*fExtensionStrings, extensionNames[i]) < 0) {
            fExtensionStrings->push_back() = extensionNames[i];
            SkTQSort(&fExtensionStrings->front(), &fExtensionStrings->back(), cmp);
        }
    }
}

bool GrVkExtensions::hasExtension(const char ext[]) const {
    return find_string(*fExtensionStrings, ext) >= 0;
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

#if GR_TEST_UTILS || defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)

#define GET_PROC_LOCAL(F, inst, device) PFN_vk ## F F = (PFN_vk ## F) fGetProc("vk" #F, inst, device)

static uint32_t remove_patch_version(uint32_t specVersion) {
    return (specVersion >> 12) << 12;
}

bool GrVkExtensionsHelper::initInstance(uint32_t specVersion) {
    if (fGetProc == nullptr) {
        return false;
    }

    uint32_t nonPatchVersion = remove_patch_version(specVersion);

    GET_PROC_LOCAL(EnumerateInstanceExtensionProperties, VK_NULL_HANDLE, VK_NULL_HANDLE);
    GET_PROC_LOCAL(EnumerateInstanceLayerProperties, VK_NULL_HANDLE, VK_NULL_HANDLE);

    SkTLessFunctionToFunctorAdaptor<SkString, extension_compare> cmp;

    if (!EnumerateInstanceExtensionProperties ||
        !EnumerateInstanceLayerProperties) {
        return false;
    }

    // instance layers
    uint32_t layerCount = 0;
    VkResult res = EnumerateInstanceLayerProperties(&layerCount, nullptr);
    if (VK_SUCCESS != res) {
        return false;
    }
    VkLayerProperties* layers = new VkLayerProperties[layerCount];
    res = EnumerateInstanceLayerProperties(&layerCount, layers);
    if (VK_SUCCESS != res) {
        delete[] layers;
        return false;
    }
    for (uint32_t i = 0; i < layerCount; ++i) {
        if (nonPatchVersion <= remove_patch_version(layers[i].specVersion)) {
            fInstanceLayerStrings->push_back() = layers[i].layerName;
        }
    }
    delete[] layers;
    if (!fInstanceLayerStrings->empty()) {
        SkTQSort(&fInstanceLayerStrings->front(), &fInstanceLayerStrings->back(), cmp);
    }

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
        fInstanceExtensionStrings->push_back() = extensions[i].extensionName;
    }
    delete [] extensions;
    // sort so we can search
    if (!fInstanceExtensionStrings->empty()) {
        SkTQSort(&fInstanceExtensionStrings->front(), &fInstanceExtensionStrings->back(), cmp);
    }
    // via explicitly enabled layers
    layerCount = fInstanceLayerStrings->count();
    for (uint32_t layerIndex = 0; layerIndex < layerCount; ++layerIndex) {
        uint32_t extensionCount = 0;
        res = EnumerateInstanceExtensionProperties((*fInstanceLayerStrings)[layerIndex].c_str(),
                                                   &extensionCount, nullptr);
        if (VK_SUCCESS != res) {
            return false;
        }
        VkExtensionProperties* extensions = new VkExtensionProperties[extensionCount];
        res = EnumerateInstanceExtensionProperties((*fInstanceLayerStrings)[layerIndex].c_str(),
                                                   &extensionCount, extensions);
        if (VK_SUCCESS != res) {
            delete[] extensions;
            return false;
        }
        for (uint32_t i = 0; i < extensionCount; ++i) {
            // if not already in the list, add it
            if (find_string(*fInstanceExtensionStrings, extensions[i].extensionName) < 0) {
                fInstanceExtensionStrings->push_back() = extensions[i].extensionName;
                SkTQSort(&fInstanceExtensionStrings->front(), &fInstanceExtensionStrings->back(),
                         cmp);
            }
        }
        delete[] extensions;
    }

    return true;
}

bool GrVkExtensionsHelper::initDevice(uint32_t specVersion, VkInstance inst, VkPhysicalDevice physDev) {
    if (fGetProc == nullptr) {
        return false;
    }

    uint32_t nonPatchVersion = remove_patch_version(specVersion);

    GET_PROC_LOCAL(EnumerateDeviceExtensionProperties, inst, VK_NULL_HANDLE);
    GET_PROC_LOCAL(EnumerateDeviceLayerProperties, inst, VK_NULL_HANDLE);

    SkTLessFunctionToFunctorAdaptor<SkString, extension_compare> cmp;

    if (!EnumerateDeviceExtensionProperties ||
        !EnumerateDeviceLayerProperties) {
        return false;
    }

    // device layers
    uint32_t layerCount = 0;
    VkResult res = EnumerateDeviceLayerProperties(physDev, &layerCount, nullptr);
    if (VK_SUCCESS != res) {
        return false;
    }
    VkLayerProperties* layers = new VkLayerProperties[layerCount];
    res = EnumerateDeviceLayerProperties(physDev, &layerCount, layers);
    if (VK_SUCCESS != res) {
        delete[] layers;
        return false;
    }
    for (uint32_t i = 0; i < layerCount; ++i) {
        if (nonPatchVersion <= remove_patch_version(layers[i].specVersion)) {
            fDeviceLayerStrings->push_back() = layers[i].layerName;
        }
    }
    delete[] layers;
    if (!fDeviceLayerStrings->empty()) {
        SkTLessFunctionToFunctorAdaptor<SkString, extension_compare> cmp;
        SkTQSort(&fDeviceLayerStrings->front(), &fDeviceLayerStrings->back(), cmp);
    }

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
        fDeviceExtensionStrings->push_back() = extensions[i].extensionName;
    }
    delete[] extensions;
    if (!fDeviceExtensionStrings->empty()) {
        SkTLessFunctionToFunctorAdaptor<SkString, extension_compare> cmp;
        SkTQSort(&fDeviceExtensionStrings->front(), &fDeviceExtensionStrings->back(), cmp);
    }
    // via explicitly enabled layers
    layerCount = fDeviceLayerStrings->count();
    for (uint32_t layerIndex = 0; layerIndex < layerCount; ++layerIndex) {
        uint32_t extensionCount = 0;
        res = EnumerateDeviceExtensionProperties(physDev,
            (*fDeviceLayerStrings)[layerIndex].c_str(),
            &extensionCount, nullptr);
        if (VK_SUCCESS != res) {
            return false;
        }
        VkExtensionProperties* extensions = new VkExtensionProperties[extensionCount];
        res = EnumerateDeviceExtensionProperties(physDev,
            (*fDeviceLayerStrings)[layerIndex].c_str(),
            &extensionCount, extensions);
        if (VK_SUCCESS != res) {
            delete[] extensions;
            return false;
        }
        for (uint32_t i = 0; i < extensionCount; ++i) {
            // if not already in the list, add it
            if (find_string(*fDeviceExtensionStrings, extensions[i].extensionName) < 0) {
                fDeviceExtensionStrings->push_back() = extensions[i].extensionName;
                SkTQSort(&fDeviceExtensionStrings->front(), &fDeviceExtensionStrings->back(), cmp);
            }
        }
        delete[] extensions;
    }

    return true;
}

bool GrVkExtensionsHelper::hasInstanceExtension(const char ext[]) const {
    return find_string(*fInstanceExtensionStrings, ext) >= 0;
}

bool GrVkExtensionsHelper::hasDeviceExtension(const char ext[]) const {
    return find_string(*fDeviceExtensionStrings, ext) >= 0;
}

bool GrVkExtensionsHelper::hasInstanceLayer(const char ext[]) const {
    return find_string(*fInstanceLayerStrings, ext) >= 0;
}

bool GrVkExtensionsHelper::hasDeviceLayer(const char ext[]) const {
    return find_string(*fDeviceLayerStrings, ext) >= 0;
}

void GrVkExtensionsHelper::print(const char* sep) const {
    if (nullptr == sep) {
        sep = " ";
    }
    int cnt = fInstanceExtensionStrings->count();
    SkDebugf("Instance Extensions: ");
    for (int i = 0; i < cnt; ++i) {
        SkDebugf("%s%s", (*fInstanceExtensionStrings)[i].c_str(), (i < cnt - 1) ? sep : "");
    }
    cnt = fDeviceExtensionStrings->count();
    SkDebugf("\nDevice Extensions: ");
    for (int i = 0; i < cnt; ++i) {
        SkDebugf("%s%s", (*fDeviceExtensionStrings)[i].c_str(), (i < cnt - 1) ? sep : "");
    }
    cnt = fInstanceLayerStrings->count();
    SkDebugf("\nInstance Layers: ");
    for (int i = 0; i < cnt; ++i) {
        SkDebugf("%s%s", (*fInstanceLayerStrings)[i].c_str(), (i < cnt - 1) ? sep : "");
    }
    cnt = fDeviceLayerStrings->count();
    SkDebugf("\nDevice Layers: ");
    for (int i = 0; i < cnt; ++i) {
        SkDebugf("%s%s", (*fDeviceLayerStrings)[i].c_str(), (i < cnt - 1) ? sep : "");
    }
}
#endif // GR_TEST_UTILS || defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)

