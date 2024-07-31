/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_BackendSemaphore_DEFINED
#define skgpu_graphite_BackendSemaphore_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/private/base/SkAnySubclass.h"

namespace skgpu::graphite {

class BackendSemaphoreData;

class SK_API BackendSemaphore {
public:
    BackendSemaphore();

    BackendSemaphore(const BackendSemaphore&);

    ~BackendSemaphore();

    BackendSemaphore& operator=(const BackendSemaphore&);

    bool isValid() const { return fIsValid; }
    BackendApi backend() const { return fBackend; }

private:
    friend class BackendSemaphoreData;
    friend class BackendSemaphorePriv;

    // Size determined by looking at the BackendSemaphoreData subclasses, then
    // guessing-and-checking. Compiler will complain if this is too small - in that case, just
    // increase the number.
    inline constexpr static size_t kMaxSubclassSize = 24;
    using AnyBackendSemaphoreData = SkAnySubclass<BackendSemaphoreData, kMaxSubclassSize>;

    template <typename SomeBackendSemaphoreData>
    BackendSemaphore(BackendApi backend, const SomeBackendSemaphoreData& data)
            : fBackend(backend), fIsValid(true) {
        fSemaphoreData.emplace<SomeBackendSemaphoreData>(data);
    }

    BackendApi fBackend;
    AnyBackendSemaphoreData fSemaphoreData;

    bool fIsValid = false;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_BackendSemaphore_DEFINED

