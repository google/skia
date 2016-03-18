/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkExtensions_DEFINED
#define GrVkExtensions_DEFINED

#include "../../private/SkTArray.h"
#include "SkString.h"
#include "vulkan/vulkan.h"

/**
 * This helper queries the current Vulkan context for its extensions and layers, remembers them, 
 * and can be queried. It supports queries for both instance and device extensions and layers.
 */
class SK_API GrVkExtensions {
public:
    GrVkExtensions() : fInitialized(false)
                     , fInstanceExtensionStrings(new SkTArray<SkString>)
                     , fDeviceExtensionStrings(new SkTArray<SkString>)
                     , fInstanceLayerStrings(new SkTArray<SkString>)
                     , fDeviceLayerStrings(new SkTArray<SkString>) {}

    GrVkExtensions(const GrVkExtensions&);

    GrVkExtensions& operator=(const GrVkExtensions&);

    void swap(GrVkExtensions* that) {
        fInstanceExtensionStrings.swap(that->fInstanceExtensionStrings);
        fDeviceExtensionStrings.swap(that->fDeviceExtensionStrings);
        fInstanceLayerStrings.swap(that->fInstanceLayerStrings);
        fDeviceLayerStrings.swap(that->fDeviceLayerStrings);

        SkTSwap(fInitialized, that->fInitialized);
    }

    /**
     * We sometimes need to use this class without having yet created a GrVkInterface. 
     */
    bool init(uint32_t specVersion,
              PFN_vkEnumerateInstanceExtensionProperties enumerateInstanceExtensionProperties,
              PFN_vkEnumerateDeviceExtensionProperties enumerateDeviceExtensionProperties,
              PFN_vkEnumerateInstanceLayerProperties enumerateInstanceLayerProperties,
              PFN_vkEnumerateDeviceLayerProperties enumerateDeviceLayerProperties);

    bool isInitialized() const { return fInitialized; }

    /**
     * Queries whether an extension or layer is present. Will fail if init() has not been called.
     */
    bool hasInstanceExtension(const char[]) const;
    bool hasDeviceExtension(const char[]) const;
    bool hasInstanceLayer(const char[]) const;
    bool hasDeviceLayer(const char[]) const;

    /**
     * Removes an extension or layer if present. Returns true if it was present before the call.
     */
    bool removeInstanceExtension(const char[]);
    bool removeDeviceExtension(const char[]);
    bool removeInstanceLayer(const char[]);
    bool removeDeviceLayer(const char[]);

    /**
     * Adds an extension or layer to list
     */
    void addInstanceExtension(const char[]);
    void addDeviceExtension(const char[]);
    void addInstanceLayer(const char[]);
    void addDeviceLayer(const char[]);

    void reset() {
        fInstanceExtensionStrings->reset();
        fDeviceExtensionStrings->reset();
        fInstanceLayerStrings->reset();
        fDeviceLayerStrings->reset();
    }

    void print(const char* sep = "\n") const;

private:
    bool                                fInitialized;
    SkAutoTDelete<SkTArray<SkString> >  fInstanceExtensionStrings;
    SkAutoTDelete<SkTArray<SkString> >  fDeviceExtensionStrings;
    SkAutoTDelete<SkTArray<SkString> >  fInstanceLayerStrings;
    SkAutoTDelete<SkTArray<SkString> >  fDeviceLayerStrings;
};

#endif
