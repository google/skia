/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/BackendSurfaceFactory.h"

#include "include/core/SkSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGpu.h"

sk_sp<SkSurface> MakeBackendRenderTargetSurface(GrDirectContext* context,
                                                SkISize dimensions,
                                                int sampleCnt,
                                                GrSurfaceOrigin origin,
                                                SkColorType colorType,
                                                sk_sp<SkColorSpace> colorSpace,
                                                const SkSurfaceProps* props) {
    auto ct = SkColorTypeToGrColorType(colorType);

    struct ReleaseContext {
        GrContext* fContext;
        GrBackendRenderTarget fRenderTarget;
    };

    auto bert = context->priv().getGpu()->createTestingOnlyBackendRenderTarget(
            dimensions, ct, sampleCnt);
    auto rc = new ReleaseContext{context, bert};
    SkASSERT(!bert.isValid() || bert.sampleCnt() >= sampleCnt);

    auto proc = [](void* c) {
        const auto* rc = static_cast<ReleaseContext*>(c);
        if (auto gpu = rc->fContext->priv().getGpu(); gpu && rc->fRenderTarget.isValid()) {
            gpu->deleteTestingOnlyBackendRenderTarget(rc->fRenderTarget);
        }
        delete rc;
    };

    return SkSurface::MakeFromBackendRenderTarget(
            context, bert, origin, colorType, std::move(colorSpace), props, proc, rc);
}
