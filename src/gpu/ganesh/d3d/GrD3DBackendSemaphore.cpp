/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/gpu/ganesh/d3d/GrD3DBackendSemaphore.h"

#include "include/gpu/ganesh/GrTypes.h"
#include "include/gpu/ganesh/d3d/GrD3DTypes.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "src/gpu/ganesh/GrBackendSemaphorePriv.h"

class GrD3DBackendSemaphoreData final : public GrBackendSemaphoreData {
public:
    GrD3DBackendSemaphoreData(const GrD3DFenceInfo& info) : fFenceInfo(info) {}

    GrD3DFenceInfo fenceInfo() const { return fFenceInfo; }

private:
    void copyTo(AnySemaphoreData& data) const override {
        data.emplace<GrD3DBackendSemaphoreData>(fFenceInfo);
    }

#if defined(SK_DEBUG)
    GrBackendApi type() const override { return GrBackendApi::kDirect3D; }
#endif

    GrD3DFenceInfo fFenceInfo;
};

static const GrD3DBackendSemaphoreData* get_and_cast_data(const GrBackendSemaphore& sem) {
    auto data = GrBackendSemaphorePriv::GetBackendData(sem);
    SkASSERT(!data || data->type() == GrBackendApi::kDirect3D);
    return static_cast<const GrD3DBackendSemaphoreData*>(data);
}

namespace GrBackendSemaphores {
GrBackendSemaphore MakeD3D(const GrD3DFenceInfo& info) {
    return GrBackendSemaphorePriv::MakeGrBackendSemaphore<GrD3DBackendSemaphoreData>(
            GrBackendApi::kDirect3D, {info});
}

GrD3DFenceInfo GetD3DFenceInfo(const GrBackendSemaphore& sem) {
    SkASSERT(sem.backend() == GrBackendApi::kDirect3D);
    const GrD3DBackendSemaphoreData* data = get_and_cast_data(sem);
    SkASSERT(data);
    return data->fenceInfo();
}
}  // namespace GrBackendSemaphores
