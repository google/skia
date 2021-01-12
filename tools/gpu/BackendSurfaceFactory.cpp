/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/BackendSurfaceFactory.h"

#include "include/core/SkSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "tools/gpu/ManagedBackendTexture.h"

namespace sk_gpu_test {

sk_sp<SkSurface> MakeBackendTextureSurface(GrDirectContext* dContext,
                                           const SkImageInfo& ii,
                                           GrSurfaceOrigin origin,
                                           int sampleCnt,
                                           GrMipmapped mipMapped,
                                           GrProtected isProtected,
                                           const SkSurfaceProps* props) {
    if (ii.alphaType() == kUnpremul_SkAlphaType) {
        return nullptr;
    }
    auto mbet = ManagedBackendTexture::MakeWithoutData(dContext,
                                                       ii.width(),
                                                       ii.height(),
                                                       ii.colorType(),
                                                       mipMapped,
                                                       GrRenderable::kYes,
                                                       isProtected);
    if (!mbet) {
        return nullptr;
    }
    return SkSurface::MakeFromBackendTexture(dContext,
                                             mbet->texture(),
                                             origin,
                                             sampleCnt,
                                             ii.colorType(),
                                             ii.refColorSpace(),
                                             props,
                                             ManagedBackendTexture::ReleaseProc,
                                             mbet->releaseContext());
}

sk_sp<SkSurface> MakeBackendTextureSurface(GrDirectContext* dContext,
                                           SkISize dimensions,
                                           GrSurfaceOrigin origin,
                                           int sampleCnt,
                                           SkColorType colorType,
                                           sk_sp<SkColorSpace> colorSpace,
                                           GrMipmapped mipMapped,
                                           GrProtected isProtected,
                                           const SkSurfaceProps* props) {
    auto ii = SkImageInfo::Make(dimensions, colorType, kPremul_SkAlphaType, std::move(colorSpace));
    return MakeBackendTextureSurface(
            dContext, ii, origin, sampleCnt, mipMapped, isProtected, props);
}
sk_sp<SkSurface> MakeBackendRenderTargetSurface(GrDirectContext* dContext,
                                                const SkImageInfo& ii,
                                                GrSurfaceOrigin origin,
                                                int sampleCnt,
                                                GrProtected isProtected,
                                                const SkSurfaceProps* props) {
    if (ii.alphaType() == kUnpremul_SkAlphaType || ii.alphaType() == kUnknown_SkAlphaType) {
        return nullptr;
    }
    auto ct = SkColorTypeToGrColorType(ii.colorType());

    struct ReleaseContext {
        sk_sp<GrDirectContext> fContext;
        GrBackendRenderTarget fRenderTarget;
    };

    auto bert = dContext->priv().getGpu()->createTestingOnlyBackendRenderTarget(
            ii.dimensions(), ct, sampleCnt, isProtected);
    auto rc = new ReleaseContext{sk_ref_sp(dContext), bert};
    SkASSERT(!bert.isValid() || bert.sampleCnt() >= sampleCnt);

    auto proc = [](void* c) {
        const auto* rc = static_cast<ReleaseContext*>(c);
        if (auto gpu = rc->fContext->priv().getGpu(); gpu && rc->fRenderTarget.isValid()) {
            gpu->deleteTestingOnlyBackendRenderTarget(rc->fRenderTarget);
        }
        delete rc;
    };

    return SkSurface::MakeFromBackendRenderTarget(
            dContext, bert, origin, ii.colorType(), ii.refColorSpace(), props, proc, rc);
}

sk_sp<SkSurface> MakeBackendRenderTargetSurface(GrDirectContext* dContext,
                                                SkISize dimensions,
                                                GrSurfaceOrigin origin,
                                                int sampleCnt,
                                                SkColorType colorType,
                                                sk_sp<SkColorSpace> colorSpace,
                                                GrProtected isProtected,
                                                const SkSurfaceProps* props) {
    auto ii = SkImageInfo::Make(dimensions, colorType, kPremul_SkAlphaType, std::move(colorSpace));
    return MakeBackendRenderTargetSurface(dContext, ii, origin, sampleCnt, isProtected, props);
}

}  // namespace sk_gpu_test
