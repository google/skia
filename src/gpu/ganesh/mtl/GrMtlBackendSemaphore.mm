/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/gpu/ganesh/mtl/GrMtlBackendSemaphore.h"

#include "include/gpu/GrTypes.h"
#include "include/private/base/SkAssert.h"
#include "src/gpu/ganesh/GrBackendSemaphorePriv.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

class GrMtlBackendSemaphoreData final : public GrBackendSemaphoreData {
public:
    GrMtlBackendSemaphoreData(GrMTLHandle event, uint64_t value) : fEvent(event), fValue(value) {}

    GrMTLHandle event() const { return fEvent; }
    uint64_t value() const { return fValue; }

private:
    void copyTo(AnySemaphoreData& data) const override {
        data.emplace<GrMtlBackendSemaphoreData>(fEvent, fValue);
    }

#if defined(SK_DEBUG)
    GrBackendApi type() const override { return GrBackendApi::kMetal; }
#endif

    GrMTLHandle fEvent;
    uint64_t fValue;
};

static const GrMtlBackendSemaphoreData* get_and_cast_data(const GrBackendSemaphore& sem) {
    auto data = GrBackendSemaphorePriv::GetBackendData(sem);
    SkASSERT(!data || data->type() == GrBackendApi::kMetal);
    return static_cast<const GrMtlBackendSemaphoreData*>(data);
}

namespace GrBackendSemaphores {
GrBackendSemaphore MakeMtl(GrMTLHandle event, uint64_t value) {
    GrMtlBackendSemaphoreData data(event, value);
    return GrBackendSemaphorePriv::MakeGrBackendSemaphore(GrBackendApi::kMetal, data);
}

GrMTLHandle GetMtlHandle(const GrBackendSemaphore& sem) {
    SkASSERT(sem.backend() == GrBackendApi::kMetal);
    const GrMtlBackendSemaphoreData* data = get_and_cast_data(sem);
    SkASSERT(data);
    return data->event();
}

uint64_t GetMtlValue(const GrBackendSemaphore& sem) {
    SkASSERT(sem.backend() == GrBackendApi::kMetal);
    const GrMtlBackendSemaphoreData* data = get_and_cast_data(sem);
    SkASSERT(data);
    return data->value();
}
}  // namespace GrBackendSemaphores
