/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBackendMutableState_DEFINED
#define GrBackendMutableState_DEFINED

#include "include/gpu/GrTypes.h"

#ifdef SK_VULKAN
#include "include/private/GrVkTypesPriv.h"
#endif

class GrBackendMutableState {
public:
#ifdef SK_VULKAN
    GrBackendMutableState(VkImageLayout layout, uint32_t queueFamilyIndex)
            : fVkState(layout, queueFamilyIndex)
            , fBackend(GrBackend::kVulkan) {}
#endif

    void set(const GrBackendMutableState& state) {
        switch (fBackend) {
#ifdef SK_VULKAN
            case GrBackend::kVulkan:
                SkASSERT(state.fBackend == GrBackend::kVulkan);
                fVkState = state.fVkState;
                break;
#endif
            default:
                break;
        }
    }

protected:
    union {
        char fDummy;
#ifdef SK_VULKAN
        GrVkSharedImageInfo fVkState;
#endif
    };

    GrBackend fBackend;
};

#endif
