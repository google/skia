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
#include "tools/gpu/ManagedBackendTexture.h"

namespace sk_gpu_test {

sk_sp<SkSurface> MakeBackendTextureSurface(GrDirectContext* context,
                                           SkISize dimensions,
                                           GrSurfaceOrigin origin,
                                           int sampleCnt,
                                           SkColorType colorType,
                                           sk_sp<SkColorSpace> colorSpace,
                                           GrMipmapped mipMapped,
                                           GrProtected isProtected,
                                           const SkSurfaceProps* props) {
    auto mbet = ManagedBackendTexture::MakeWithoutData(context,
                                                       dimensions.fWidth,
                                                       dimensions.fHeight,
                                                       colorType,
                                                       mipMapped,
                                                       GrRenderable::kYes,
                                                       isProtected);
    if (!mbet) {
        return nullptr;
    }
    return SkSurface::MakeFromBackendTexture(context,
                                             mbet->texture(),
                                             origin,
                                             sampleCnt,
                                             colorType,
                                             std::move(colorSpace),
                                             props,
                                             ManagedBackendTexture::ReleaseProc,
                                             mbet->releaseContext());
}

sk_sp<SkSurface> MakeBackendRenderTargetSurface(GrDirectContext* context,
                                                SkISize dimensions,
                                                GrSurfaceOrigin origin,
                                                int sampleCnt,
                                                SkColorType colorType,
                                                sk_sp<SkColorSpace> colorSpace,
                                                GrProtected isProtected,
                                                const SkSurfaceProps* props) {
    auto ct = SkColorTypeToGrColorType(colorType);

    struct ReleaseContext {
        GrDirectContext* fContext;
        GrBackendRenderTarget fRenderTarget;
    };

    auto bert = context->priv().getGpu()->createTestingOnlyBackendRenderTarget(
            dimensions, ct, sampleCnt, isProtected);
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

}  // namespace sk_gpu_test
