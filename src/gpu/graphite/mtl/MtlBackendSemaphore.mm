/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkString.h"
#include "include/gpu/MutableTextureState.h"
#include "include/gpu/graphite/mtl/MtlGraphiteTypes_cpp.h"
#include "src/gpu/graphite/BackendSemaphorePriv.h"
#include "src/gpu/graphite/mtl/MtlGraphiteUtils.h"
#include "src/gpu/mtl/MtlUtilsPriv.h"

#include <cstdint>

#import <Metal/Metal.h>

namespace skgpu::graphite {

class MtlBackendSemaphoreData final : public BackendSemaphoreData {
public:
    MtlBackendSemaphoreData(CFTypeRef mtlEvent, uint64_t value)
            : fMtlEvent(mtlEvent), fMtlValue(value) {}

#if defined(SK_DEBUG)
    skgpu::BackendApi type() const override { return skgpu::BackendApi::kMetal; }
#endif

    CFTypeRef event() const { return fMtlEvent; }
    uint64_t value() const { return fMtlValue; }

private:
    CFTypeRef fMtlEvent;  // Expected to be an id<MTLEvent>
    uint64_t fMtlValue;

    void copyTo(AnyBackendSemaphoreData& dstData) const override {
        // Don't assert that dstData is a metal type because it could be
        // uninitialized and that assert would fail.
        dstData.emplace<MtlBackendSemaphoreData>(fMtlEvent, fMtlValue);
    }
};

static const MtlBackendSemaphoreData* get_and_cast_data(const BackendSemaphore& sem) {
    auto data = BackendSemaphorePriv::GetData(sem);
    SkASSERT(!data || data->type() == skgpu::BackendApi::kMetal);
    return static_cast<const MtlBackendSemaphoreData*>(data);
}

namespace BackendSemaphores {
BackendSemaphore MakeMetal(CFTypeRef mtlEvent, uint64_t value) {
    return BackendSemaphorePriv::Make(skgpu::BackendApi::kMetal,
                                      MtlBackendSemaphoreData(mtlEvent, value));
}

CFTypeRef GetMtlEvent(const BackendSemaphore& sem) {
    if (!sem.isValid() || sem.backend() != skgpu::BackendApi::kMetal) {
        return nullptr;
    }
    const MtlBackendSemaphoreData* mtlData = get_and_cast_data(sem);
    SkASSERT(mtlData);
    return mtlData->event();
}

uint64_t GetMtlValue(const BackendSemaphore& sem) {
    if (!sem.isValid() || sem.backend() != skgpu::BackendApi::kMetal) {
        return 0;
    }
    const MtlBackendSemaphoreData* mtlData = get_and_cast_data(sem);
    SkASSERT(mtlData);
    return mtlData->value();
}

}  // namespace BackendSemaphores

}  // namespace skgpu::graphite
