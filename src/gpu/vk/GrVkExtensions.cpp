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

GrVkExtensions::GrVkExtensions(const GrVkExtensions& that) 
    : fInstanceExtensionStrings(new SkTArray<SkString>)
    , fDeviceExtensionStrings(new SkTArray<SkString>)
    , fInstanceLayerStrings(new SkTArray<SkString>)
    , fDeviceLayerStrings(new SkTArray<SkString>) {
    *this = that;
}

GrVkExtensions& GrVkExtensions::operator=(const GrVkExtensions& that) {
    *fInstanceExtensionStrings = *that.fInstanceExtensionStrings;
    *fDeviceExtensionStrings = *that.fDeviceExtensionStrings;
    *fInstanceLayerStrings = *that.fInstanceLayerStrings;
    *fDeviceLayerStrings = *that.fDeviceLayerStrings;

    fInitialized = that.fInitialized;
    return *this;
}

bool GrVkExtensions::init(
    uint32_t specVersion,
    VkPhysicalDevice physDev,
    PFN_vkEnumerateInstanceExtensionProperties enumerateInstanceExtensionProperties,
    PFN_vkEnumerateDeviceExtensionProperties enumerateDeviceExtensionProperties,
    PFN_vkEnumerateInstanceLayerProperties enumerateInstanceLayerProperties,
    PFN_vkEnumerateDeviceLayerProperties enumerateDeviceLayerProperties) {
    fInitialized = false;
    this->reset();
    SkTLessFunctionToFunctorAdaptor<SkString, extension_compare> cmp;

    if (!enumerateInstanceExtensionProperties ||
        !enumerateDeviceExtensionProperties ||
        !enumerateInstanceLayerProperties ||
        !enumerateDeviceLayerProperties) {
        return false;
    }

    // instance layers
    uint32_t layerCount = 0;
    VkResult res = enumerateInstanceLayerProperties(&layerCount, nullptr);
    if (VK_SUCCESS != res) {
        return false;
    }
    VkLayerProperties* layers = new VkLayerProperties[layerCount];
    res = enumerateInstanceLayerProperties(&layerCount, layers);
    if (VK_SUCCESS != res) {
        return false;
    }
    for (uint32_t i = 0; i < layerCount; ++i) {
        if (specVersion >= layers[i].specVersion) {
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
    res = enumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    if (VK_SUCCESS != res) {
        return false;
    }
    VkExtensionProperties* extensions = new VkExtensionProperties[extensionCount];
    res = enumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions);
    if (VK_SUCCESS != res) {
        return false;
    }
    for (uint32_t i = 0; i < extensionCount; ++i) {
        if (specVersion >= extensions[i].specVersion) {
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
        res = enumerateInstanceExtensionProperties((*fInstanceLayerStrings)[layerIndex].c_str(), 
                                                   &extensionCount, nullptr);
        if (VK_SUCCESS != res) {
            return false;
        }
        VkExtensionProperties* extensions = new VkExtensionProperties[extensionCount];
        res = enumerateInstanceExtensionProperties((*fInstanceLayerStrings)[layerIndex].c_str(),
                                                   &extensionCount, extensions);
        if (VK_SUCCESS != res) {
            return false;
        }
        for (uint32_t i = 0; i < extensionCount; ++i) {
            // if not already in the list, add it
            if (specVersion >= extensions[i].specVersion &&
                find_string(*fInstanceExtensionStrings, extensions[i].extensionName) < 0) {
                fInstanceExtensionStrings->push_back() = extensions[i].extensionName;
                SkTQSort(&fInstanceExtensionStrings->front(), &fInstanceExtensionStrings->back(), 
                         cmp);
            }
        }
        delete[] extensions;
    }

    // device layers
    layerCount = 0;
    res = enumerateDeviceLayerProperties(physDev, &layerCount, nullptr);
    if (VK_SUCCESS != res) {
        return false;
    }
    layers = new VkLayerProperties[layerCount];
    res = enumerateDeviceLayerProperties(physDev, &layerCount, layers);
    if (VK_SUCCESS != res) {
        return false;
    }
    for (uint32_t i = 0; i < layerCount; ++i) {
        if (specVersion >= layers[i].specVersion) {
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
    extensionCount = 0;
    res = enumerateDeviceExtensionProperties(physDev, nullptr, &extensionCount, nullptr);
    if (VK_SUCCESS != res) {
        return false;
    }
    extensions = new VkExtensionProperties[extensionCount];
    res = enumerateDeviceExtensionProperties(physDev, nullptr, &extensionCount, extensions);
    if (VK_SUCCESS != res) {
        return false;
    }
    for (uint32_t i = 0; i < extensionCount; ++i) {
        if (specVersion >= extensions[i].specVersion) {
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
        res = enumerateDeviceExtensionProperties(physDev, 
                                                 (*fDeviceLayerStrings)[layerIndex].c_str(),
                                                 &extensionCount, nullptr);
        if (VK_SUCCESS != res) {
            return false;
        }
        VkExtensionProperties* extensions = new VkExtensionProperties[extensionCount];
        res = enumerateDeviceExtensionProperties(physDev,
                                                 (*fDeviceLayerStrings)[layerIndex].c_str(),
                                                 &extensionCount, extensions);
        if (VK_SUCCESS != res) {
            return false;
        }
        for (uint32_t i = 0; i < extensionCount; ++i) {
            // if not already in the list, add it
            if (specVersion >= extensions[i].specVersion &&
                find_string(*fDeviceExtensionStrings, extensions[i].extensionName) < 0) {
                fDeviceExtensionStrings->push_back() = extensions[i].extensionName;
                SkTQSort(&fDeviceExtensionStrings->front(), &fDeviceExtensionStrings->back(), cmp);
            }
        }
        delete[] extensions;
    }

    fInitialized = true;
    return true;
}


bool GrVkExtensions::hasInstanceExtension(const char ext[]) const {
    SkASSERT(fInitialized);

    return find_string(*fInstanceExtensionStrings, ext) >= 0;
}

bool GrVkExtensions::hasDeviceExtension(const char ext[]) const {
    SkASSERT(fInitialized);

    return find_string(*fDeviceExtensionStrings, ext) >= 0;
}

bool GrVkExtensions::hasInstanceLayer(const char ext[]) const {
    SkASSERT(fInitialized);

    return find_string(*fInstanceLayerStrings, ext) >= 0;
}

bool GrVkExtensions::hasDeviceLayer(const char ext[]) const {
    SkASSERT(fInitialized);

    return find_string(*fDeviceLayerStrings, ext) >= 0;
}


bool GrVkExtensions::removeInstanceExtension(const char ext[]) {
    SkASSERT(fInitialized);
    int idx = find_string(*fInstanceExtensionStrings, ext);
    if (idx >= 0) {
        // This is not terribly effecient but we really only expect this function to be called at
        // most a handful of times when our test programs start.
        SkAutoTDelete< SkTArray<SkString> > oldStrings(fInstanceExtensionStrings.release());
        fInstanceExtensionStrings.reset(new SkTArray<SkString>(oldStrings->count() - 1));
        fInstanceExtensionStrings->push_back_n(idx, &oldStrings->front());
        fInstanceExtensionStrings->push_back_n(oldStrings->count() - idx-1, &(*oldStrings)[idx]+1);
        return true;
    } else {
        return false;
    }
}

bool GrVkExtensions::removeDeviceExtension(const char ext[]) {
    SkASSERT(fInitialized);
    int idx = find_string(*fDeviceExtensionStrings, ext);
    if (idx >= 0) {
        // This is not terribly effecient but we really only expect this function to be called at
        // most a handful of times when our test programs start.
        SkAutoTDelete< SkTArray<SkString> > oldStrings(fDeviceExtensionStrings.release());
        fDeviceExtensionStrings.reset(new SkTArray<SkString>(oldStrings->count() - 1));
        fDeviceExtensionStrings->push_back_n(idx, &oldStrings->front());
        fDeviceExtensionStrings->push_back_n(oldStrings->count() - idx-1, &(*oldStrings)[idx] + 1);
        return true;
    }
    else {
        return false;
    }
}

bool GrVkExtensions::removeInstanceLayer(const char ext[]) {
    SkASSERT(fInitialized);
    int idx = find_string(*fInstanceLayerStrings, ext);
    if (idx >= 0) {
        // This is not terribly effecient but we really only expect this function to be called at
        // most a handful of times when our test programs start.
        SkAutoTDelete< SkTArray<SkString> > oldStrings(fInstanceLayerStrings.release());
        fInstanceLayerStrings.reset(new SkTArray<SkString>(oldStrings->count() - 1));
        fInstanceLayerStrings->push_back_n(idx, &oldStrings->front());
        fInstanceLayerStrings->push_back_n(oldStrings->count() - idx - 1, &(*oldStrings)[idx] + 1);
        return true;
    }
    else {
        return false;
    }
}

bool GrVkExtensions::removeDeviceLayer(const char ext[]) {
    SkASSERT(fInitialized);
    int idx = find_string(*fDeviceLayerStrings, ext);
    if (idx >= 0) {
        // This is not terribly effecient but we really only expect this function to be called at
        // most a handful of times when our test programs start.
        SkAutoTDelete< SkTArray<SkString> > oldStrings(fDeviceLayerStrings.release());
        fDeviceLayerStrings.reset(new SkTArray<SkString>(oldStrings->count() - 1));
        fDeviceLayerStrings->push_back_n(idx, &oldStrings->front());
        fDeviceLayerStrings->push_back_n(oldStrings->count() - idx - 1, &(*oldStrings)[idx] + 1);
        return true;
    }
    else {
        return false;
    }
}

void GrVkExtensions::addInstanceExtension(const char ext[]) {
    int idx = find_string(*fInstanceExtensionStrings, ext);
    if (idx < 0) {
        // This is not the most effecient approach since we end up doing a full sort of the
        // extensions after the add
        fInstanceExtensionStrings->push_back().set(ext);
        SkTLessFunctionToFunctorAdaptor<SkString, extension_compare> cmp;
        SkTQSort(&fInstanceExtensionStrings->front(), &fInstanceExtensionStrings->back(), cmp);
    }
}

void GrVkExtensions::addDeviceExtension(const char ext[]) {
    int idx = find_string(*fDeviceExtensionStrings, ext);
    if (idx < 0) {
        // This is not the most effecient approach since we end up doing a full sort of the
        // extensions after the add
        fDeviceExtensionStrings->push_back().set(ext);
        SkTLessFunctionToFunctorAdaptor<SkString, extension_compare> cmp;
        SkTQSort(&fDeviceExtensionStrings->front(), &fDeviceExtensionStrings->back(), cmp);
    }
}

void GrVkExtensions::addInstanceLayer(const char ext[]) {
    int idx = find_string(*fInstanceLayerStrings, ext);
    if (idx < 0) {
        // This is not the most effecient approach since we end up doing a full sort of the
        // extensions after the add
        fInstanceLayerStrings->push_back().set(ext);
        SkTLessFunctionToFunctorAdaptor<SkString, extension_compare> cmp;
        SkTQSort(&fInstanceLayerStrings->front(), &fInstanceLayerStrings->back(), cmp);
    }
}

void GrVkExtensions::addDeviceLayer(const char ext[]) {
    int idx = find_string(*fDeviceLayerStrings, ext);
    if (idx < 0) {
        // This is not the most effecient approach since we end up doing a full sort of the
        // extensions after the add
        fDeviceLayerStrings->push_back().set(ext);
        SkTLessFunctionToFunctorAdaptor<SkString, extension_compare> cmp;
        SkTQSort(&fDeviceLayerStrings->front(), &fDeviceLayerStrings->back(), cmp);
    }
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
