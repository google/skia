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
#include "vk/GrVkDefines.h"
#include "vk/GrVkInterface.h"

// TODO: Once android no longer uses our helpers to build GrVkBackendContext, move this helper into
// our tools directory.
#if GR_TEST_UTILS || defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
/**
 * This helper queries the Vulkan driver for available extensions and layers, remembers them,
 * and can be queried. It supports queries for both instance and device extensions and layers.
 */
class SK_API GrVkExtensionsHelper {
public:
    GrVkExtensionsHelper(GrVkInterface::GetProc getProc)
            : fGetProc(getProc)
            , fInstanceExtensionStrings(new SkTArray<SkString>)
            , fDeviceExtensionStrings(new SkTArray<SkString>)
            , fInstanceLayerStrings(new SkTArray<SkString>)
            , fDeviceLayerStrings(new SkTArray<SkString>) {}

    bool initInstance(uint32_t specVersion);
    bool initDevice(uint32_t specVersion, VkInstance, VkPhysicalDevice);

    /**
     * Queries whether an extension or layer is present. Will fail if not initialized.
     */
    bool hasInstanceExtension(const char[]) const;
    bool hasDeviceExtension(const char[]) const;
    bool hasInstanceLayer(const char[]) const;
    bool hasDeviceLayer(const char[]) const;

    void print(const char* sep = "\n") const;

private:
    GrVkInterface::GetProc fGetProc;
    std::unique_ptr<SkTArray<SkString>>  fInstanceExtensionStrings;
    std::unique_ptr<SkTArray<SkString>>  fDeviceExtensionStrings;
    std::unique_ptr<SkTArray<SkString>>  fInstanceLayerStrings;
    std::unique_ptr<SkTArray<SkString>>  fDeviceLayerStrings;
};
#endif

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
    std::unique_ptr<SkTArray<SkString>>  fExtensionStrings;
};

#endif
