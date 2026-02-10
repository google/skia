/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrBackendSemaphore_DEFINED
#define GrBackendSemaphore_DEFINED

#include "include/gpu/ganesh/GrTypes.h"
#include "include/private/base/SkAPI.h"
#include "include/private/base/SkAnySubclass.h"

#include <cstddef>

class GrBackendSemaphoreData;

/**
 * Wrapper class for passing into and receiving data from Ganesh about a backend semaphore object.
 */
class SK_API GrBackendSemaphore {
public:
    // An empty semaphore. To instantiate, see GrBackendSemaphores::MakeVk and similar
    GrBackendSemaphore();
    ~GrBackendSemaphore();
    GrBackendSemaphore(const GrBackendSemaphore&);
    GrBackendSemaphore& operator=(const GrBackendSemaphore&);

    GrBackendApi backend() const { return fBackend; }
    bool isInitialized() const { return fBackend != GrBackendApi::kUnsupported; }

private:
    friend class GrBackendSemaphorePriv;
    friend class GrBackendSemaphoreData;
    // Size determined by looking at the GrBackendSemaphoreData subclasses, then
    // guessing-and-checking. Compiler will complain if this is too small - in that case,
    // just increase the number.
    inline constexpr static size_t kMaxSubclassSize = 24;
    using AnySemaphoreData = SkAnySubclass<GrBackendSemaphoreData, kMaxSubclassSize>;

    template <typename SemaphoreData>
    GrBackendSemaphore(GrBackendApi api, SemaphoreData data) : fBackend(api) {
        fSemaphoreData.emplace<SemaphoreData>(data);
    }

    GrBackendApi fBackend = GrBackendApi::kUnsupported;
    AnySemaphoreData fSemaphoreData;
};

#endif
