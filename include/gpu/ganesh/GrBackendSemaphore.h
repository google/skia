/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBackendSemaphore_DEFINED
#define GrBackendSemaphore_DEFINED

#include "include/gpu/ganesh/GrTypes.h"  // IWYU pragma: keep
#include "include/private/base/SkAPI.h"
#include "include/private/base/SkAnySubclass.h"

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
    inline constexpr static size_t kMaxSubclassSize = 24;
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
#ifdef SK_DIRECT3D
        GrD3DFenceInfo* fD3DFenceInfo;
#endif
    };
    bool fIsInitialized;
};

#endif
