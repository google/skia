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
    PFN_vkEnumerateInstanceExtensionProperties enumerateInstanceExtensionProperties,
    PFN_vkEnumerateDeviceExtensionProperties enumerateDeviceExtensionProperties,
    PFN_vkEnumerateInstanceLayerProperties enumerateInstanceLayerProperties,
    PFN_vkEnumerateDeviceLayerProperties enumerateDeviceLayerProperties) {
    fInitialized = false;
    this->reset();

    if (!enumerateInstanceExtensionProperties ||
        !enumerateDeviceExtensionProperties ||
        !enumerateInstanceLayerProperties ||
        !enumerateDeviceLayerProperties) {
        return false;
    }

    // instance extensions
    uint32_t extensionCount = 0;
    VkResult res = enumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    VkExtensionProperties* extensions = new VkExtensionProperties[extensionCount];
    res = enumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions);

    fInstanceExtensionStrings->push_back_n(extensionCount);
    for (uint32_t i = 0; i < extensionCount; ++i) {
        if (specVersion >= extensions[i].specVersion) {
            (*fInstanceExtensionStrings)[i] = extensions[i].extensionName;
        }
    }
    delete [] extensions;

    if (!fInstanceExtensionStrings->empty()) {
        SkTLessFunctionToFunctorAdaptor<SkString, extension_compare> cmp;
        SkTQSort(&fInstanceExtensionStrings->front(), &fInstanceExtensionStrings->back(), cmp);
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
