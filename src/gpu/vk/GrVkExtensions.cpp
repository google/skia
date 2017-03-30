/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "vk/GrVkExtensions.h"
#include "vk/GrVkUtil.h"

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

#define GET_PROC_LOCAL(F, inst, device) PFN_vk ## F F = (PFN_vk ## F) fGetProc("vk" #F, inst, device)

static uint32_t remove_patch_version(uint32_t specVersion) {
    return (specVersion >> 12) << 12;
}

bool GrVkExtensions::initInstance(uint32_t specVersion) {
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
        if (nonPatchVersion >= remove_patch_version(layers[i].specVersion)) {
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
        if (nonPatchVersion >= remove_patch_version(extensions[i].specVersion)) {
            fInstanceExtensionStrings->push_back() = extensions[i].extensionName;
        }
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
            if (nonPatchVersion >= remove_patch_version(extensions[i].specVersion) &&
                find_string(*fInstanceExtensionStrings, extensions[i].extensionName) < 0) {
                fInstanceExtensionStrings->push_back() = extensions[i].extensionName;
                SkTQSort(&fInstanceExtensionStrings->front(), &fInstanceExtensionStrings->back(),
                         cmp);
            }
        }
        delete[] extensions;
    }

    return true;
}

bool GrVkExtensions::initDevice(uint32_t specVersion, VkInstance inst, VkPhysicalDevice physDev) {
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
        if (nonPatchVersion >= remove_patch_version(layers[i].specVersion)) {
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
        if (nonPatchVersion >= remove_patch_version(extensions[i].specVersion)) {
            fDeviceExtensionStrings->push_back() = extensions[i].extensionName;
        }
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
            if (nonPatchVersion >= remove_patch_version(extensions[i].specVersion) &&
                find_string(*fDeviceExtensionStrings, extensions[i].extensionName) < 0) {
                fDeviceExtensionStrings->push_back() = extensions[i].extensionName;
                SkTQSort(&fDeviceExtensionStrings->front(), &fDeviceExtensionStrings->back(), cmp);
            }
        }
        delete[] extensions;
    }

    return true;
}

bool GrVkExtensions::hasInstanceExtension(const char ext[]) const {
    return find_string(*fInstanceExtensionStrings, ext) >= 0;
}

bool GrVkExtensions::hasDeviceExtension(const char ext[]) const {
    return find_string(*fDeviceExtensionStrings, ext) >= 0;
}

bool GrVkExtensions::hasInstanceLayer(const char ext[]) const {
    return find_string(*fInstanceLayerStrings, ext) >= 0;
}

bool GrVkExtensions::hasDeviceLayer(const char ext[]) const {
    return find_string(*fDeviceLayerStrings, ext) >= 0;
}

void GrVkExtensions::print(const char* sep) const {
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
