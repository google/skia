/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBackendSemaphore_DEFINED
#define GrBackendSemaphore_DEFINED

#include "include/gpu/GrTypes.h"  // IWYU pragma: keep
#include "include/private/base/SkAPI.h"
#include "include/private/base/SkAnySubclass.h"

#ifdef SK_METAL
#include "include/gpu/mtl/GrMtlTypes.h"
#endif

#if defined(SK_VULKAN) && !defined(SK_DISABLE_LEGACY_VULKAN_BACKENDSEMAPHORE)
#include "include/private/gpu/vk/SkiaVulkan.h"
#endif

#ifdef SK_DIRECT3D
#include "include/private/gpu/ganesh/GrD3DTypesMinimal.h"
#endif

#include <cstddef>

class GrBackendSemaphoreData;

/**
 * Wrapper class for passing into and receiving data from Ganesh about a backend semaphore object.
 */
class SK_API GrBackendSemaphore {
public:
    // The GrBackendSemaphore cannot be used until either init* is called, which will set the
    // appropriate GrBackend.
    GrBackendSemaphore();
    ~GrBackendSemaphore();
    GrBackendSemaphore(const GrBackendSemaphore&);
    GrBackendSemaphore& operator=(const GrBackendSemaphore&);

#if defined(SK_VULKAN) && !defined(SK_DISABLE_LEGACY_VULKAN_BACKENDSEMAPHORE)
    void initVulkan(VkSemaphore semaphore);
    VkSemaphore vkSemaphore() const;
#endif

#ifdef SK_METAL
    // It is the creator's responsibility to ref the MTLEvent passed in here, via __bridge_retained.
    // The other end will wrap this BackendSemaphore and take the ref, via __bridge_transfer.
    void initMetal(GrMTLHandle event, uint64_t value) {
        fBackend = GrBackendApi::kMetal;
        fMtlEvent = event;
        fMtlValue = value;

        fIsInitialized = true;
    }

    GrMTLHandle mtlSemaphore() const {
        if (!fIsInitialized || GrBackendApi::kMetal != fBackend) {
            return nullptr;
        }
        return fMtlEvent;
    }

    uint64_t mtlValue() const {
        if (!fIsInitialized || GrBackendApi::kMetal != fBackend) {
            return 0;
        }
        return fMtlValue;
    }

#endif

#ifdef SK_DIRECT3D
    void initDirect3D(const GrD3DFenceInfo& info) {
        fBackend = GrBackendApi::kDirect3D;
        this->assignD3DFenceInfo(info);
        fIsInitialized = true;
    }
#endif

    bool isInitialized() const { return fIsInitialized; }
    GrBackendApi backend() const { return fBackend; }

#ifdef SK_DIRECT3D
    bool getD3DFenceInfo(GrD3DFenceInfo* outInfo) const;
#endif

private:
    friend class GrBackendSemaphorePriv;
    friend class GrBackendSemaphoreData;
    // Size determined by looking at the GrBackendSemaphoreData subclasses, then
    // guessing-and-checking. Compiler will complain if this is too small - in that case,
    // just increase the number.
    inline constexpr static size_t kMaxSubclassSize = 16;
    using AnySemaphoreData = SkAnySubclass<GrBackendSemaphoreData, kMaxSubclassSize>;

    template <typename SemaphoreData>
    GrBackendSemaphore(GrBackendApi api, SemaphoreData data) : fBackend(api), fIsInitialized(true) {
        fSemaphoreData.emplace<SemaphoreData>(data);
    }

#ifdef SK_DIRECT3D
    void assignD3DFenceInfo(const GrD3DFenceInfo& info);
#endif

    GrBackendApi fBackend;
    AnySemaphoreData fSemaphoreData;

    union {
        void* fPlaceholder;  // TODO(293490566)
#ifdef SK_METAL
        GrMTLHandle fMtlEvent;    // Expected to be an id<MTLEvent>
#endif
#ifdef SK_DIRECT3D
        GrD3DFenceInfo* fD3DFenceInfo;
#endif
    };
#ifdef SK_METAL
    uint64_t fMtlValue;
#endif
    bool fIsInitialized;
};

#endif
