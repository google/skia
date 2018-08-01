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

void GrVkExtensions::init(uint32_t instanceExtensionCount,
                          const char* const* instanceExtensions,
                          uint32_t deviceExtensionCount,
                          const char* const* deviceExtensions) {
    SkTLessFunctionToFunctorAdaptor<SkString, extension_compare> cmp;

    for (uint32_t i = 0; i < instanceExtensionCount; ++i) {
        const char* extension = instanceExtensions[i];
        // if not already in the list, add it
        if (find_string(fExtensionStrings, extension) < 0) {
            fExtensionStrings.push_back() = extension;
            SkTQSort(&fExtensionStrings.front(), &fExtensionStrings.back(), cmp);
        }
    }
    for (uint32_t i = 0; i < deviceExtensionCount; ++i) {
        const char* extension = deviceExtensions[i];
        // if not already in the list, add it
        if (find_string(fExtensionStrings, extension) < 0) {
            fExtensionStrings.push_back() = extension;
            SkTQSort(&fExtensionStrings.front(), &fExtensionStrings.back(), cmp);
        }
    }
}

bool GrVkExtensions::hasExtension(const char ext[]) const {
    return find_string(fExtensionStrings, ext) >= 0;
}

