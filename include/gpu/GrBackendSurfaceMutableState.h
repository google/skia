/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBackendSurfaceMutableState_DEFINED
#define GrBackendSurfaceMutableState_DEFINED

#include "include/gpu/GrTypes.h"

#ifdef SK_VULKAN
#include "include/private/GrVkTypesPriv.h"
#endif

/**
 * Since Skia and clients can both modify gpu textures and their connected state, Skia needs a way
 * for clients to inform us if they have modifiend any of this state. In order to not need setters
 * for every single API and state, we use this class to be a generic wrapper around all the mutable
 * state. This class is used for calls that inform Skia of these texture/image state changes by the
 * client as well as for requesting state changes to be done by Skia. The backend specific state
 * that is wrapped by this class are:
 *
 * Vulkan: VkImageLayout and QueueFamilyIndex
 */
class GrBackendSurfaceMutableState {
public:
#ifdef SK_VULKAN
    GrBackendSurfaceMutableState(VkImageLayout layout, uint32_t queueFamilyIndex)
            : fVkState(layout, queueFamilyIndex)
            , fBackend(GrBackend::kVulkan) {}
#endif

    GrBackendSurfaceMutableState& operator=(const GrBackendSurfaceMutableState& that) {
        switch (fBackend) {
            case GrBackend::kVulkan:
#ifdef SK_VULKAN
                SkASSERT(that.fBackend == GrBackend::kVulkan);
                fVkState = that.fVkState;
#endif
                break;

            default:
                (void)that;
                SkUNREACHABLE;
        }
        fBackend = that.fBackend;
        return *this;
    }

private:
    friend class GrBackendSurfaceMutableStateImpl;

    union {
        char fDummy;
#ifdef SK_VULKAN
        GrVkSharedImageInfo fVkState;
#endif
    };

    GrBackend fBackend;
};

#endif
