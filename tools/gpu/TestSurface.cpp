/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/TestSurface.h"

#include "include/core/SkSurface.h"
#include "include/gpu/GrContext.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGpu.h"

sk_sp<SkSurface> MakeBackendRenderTargetSurface(GrContext* context,
                                                SkISize dimensions,
                                                int sampleCnt,
                                                GrSurfaceOrigin origin,
                                                SkColorType colorType,
                                                sk_sp<SkColorSpace> colorSpace,
                                                const SkSurfaceProps* props) {
    auto ct = SkColorTypeToGrColorType(colorType);

    auto bert = context->priv().getGpu()->createTestingOnlyBackendRenderTarget(dimensions, ct, sampleCnt);
    if (!bert.isValid()) {
        return {};
    }
    struct ReleaseContext {
        GrContext* fContext;
        GrBackendRenderTarget fRenderTarget;
    };
    auto proc = [](void* c) {
        const auto* rc = static_cast<ReleaseContext*>(c);
        rc->fContext->flushAndSubmit(true);
        if (auto gpu = rc->fContext->priv().getGpu()) {
            gpu->deleteTestingOnlyBackendRenderTarget(rc->fRenderTarget);
        }
        delete rc;
    };
    return SkSurface::MakeFromBackendRenderTarget(context, bert, origin, colorType, std::move(colorSpace), props, proc, new ReleaseContext{context, bert});
}
