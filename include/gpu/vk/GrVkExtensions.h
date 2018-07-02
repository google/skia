/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkExtensions_DEFINED
#define GrVkExtensions_DEFINED

#include "../private/SkTArray.h"
#include "SkString.h"

/**
 * Helper class that eats in an array of extensions strings for instance and device and allows for
 * quicker querying if an extension is present.
 */
class GrVkExtensions {
public:
    GrVkExtensions(uint32_t instanceExtensionCount, const char* const* instanceExtensions,
                   uint32_t deviceExtensionCount, const char* const* deviceExtensions);
    // TODO: Remove once we remove the old fExtensions from GrVkBackendContext
    GrVkExtensions(uint32_t extensionFlags);

    // TODO: Remove once we remove the old fExtensions from GrVkBackendContext
    static void GetExtensionArrayFromFlags(uint32_t extensionFlags,
                                           SkTArray<const char*>* extensions);

    bool hasExtension(const char[]) const;

private:
    SkTArray<SkString>  fExtensionStrings;
};

#endif
