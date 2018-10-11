/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBackendDrawableInfo_DEFINED
#define GrBackendDrawableInfo_DEFINED

#include "GrTypes.h"

#include "vk/GrVkTypes.h"

class SK_API GrBackendDrawableInfo {
public:
    // Creates an invalid backend drawable info.
    GrBackendDrawableInfo() : fIsValid(false) {}

    GrBackendDrawableInfo(const GrVkDrawableInfo& info)
            : fIsValid(true)
            , fBackend(kVulkan_GrBackend)
            , fVkInfo(info) {}

    // Returns true if the backend texture has been initialized.
    bool isValid() const { return fIsValid; }

    GrBackend backend() const { return fBackend; }

    bool getVkDrawableInfo(GrVkDrawableInfo* outInfo) const {
        if (this->isValid() && kVulkan_GrBackend == fBackend) {
            *outInfo = fVkInfo;
            return true;
        }
        return false;
    }

private:
    bool             fIsValid;
    GrBackend        fBackend;
    GrVkDrawableInfo fVkInfo;
};

#endif
