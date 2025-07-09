/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_BackendSemaphorePriv_DEFINED
#define skgpu_graphite_BackendSemaphorePriv_DEFINED

#include "include/gpu/graphite/BackendSemaphore.h"
#include "include/private/base/SkDebug.h"

namespace skgpu { enum class BackendApi : unsigned int; }

namespace skgpu::graphite {

class BackendSemaphoreData {
public:
    virtual ~BackendSemaphoreData();

#if defined(SK_DEBUG)
    virtual skgpu::BackendApi type() const = 0;
#endif
protected:
    BackendSemaphoreData() = default;
    BackendSemaphoreData(const BackendSemaphoreData&) = default;

    using AnyBackendSemaphoreData = BackendSemaphore::AnyBackendSemaphoreData;

private:
    friend class BackendSemaphore;

    virtual void copyTo(AnyBackendSemaphoreData& dstData) const = 0;
};

class BackendSemaphorePriv {
public:
    template <typename SomeBackendSemaphoreData>
    static BackendSemaphore Make(BackendApi backend, const SomeBackendSemaphoreData& textureData) {
        return BackendSemaphore(backend, textureData);
    }

    static const BackendSemaphoreData* GetData(const BackendSemaphore& info) {
        return info.fSemaphoreData.get();
    }
};

}  // namespace skgpu::graphite

#endif
