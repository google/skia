/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/BackendSurfaceFactory.h"

#include "include/core/SkSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "tools/gpu/ManagedBackendTexture.h"

#ifdef SK_GRAPHITE
#include "include/gpu/graphite/Surface.h"
#endif

namespace sk_gpu_test {

sk_sp<SkSurface> MakeBackendTextureSurface(GrDirectContext* dContext,
                                           const SkImageInfo& ii,
                                           GrSurfaceOrigin origin,
                                           int sampleCnt,
                                           skgpu::Mipmapped mipmapped,
                                           GrProtected isProtected,
                                           const SkSurfaceProps* props) {
    if (ii.alphaType() == kUnpremul_SkAlphaType) {
        return nullptr;
    }
    auto mbet = ManagedBackendTexture::MakeWithoutData(dContext,
                                                       ii.width(),
                                                       ii.height(),
                                                       ii.colorType(),
                                                       mipmapped,
                                                       GrRenderable::kYes,
                                                       isProtected);
    if (!mbet) {
        return nullptr;
    }
    return SkSurfaces::WrapBackendTexture(dContext,
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
                                           skgpu::Mipmapped mipmapped,
                                           GrProtected isProtected,
                                           const SkSurfaceProps* props) {
    auto ii = SkImageInfo::Make(dimensions, colorType, kPremul_SkAlphaType, std::move(colorSpace));
    return MakeBackendTextureSurface(
            dContext, ii, origin, sampleCnt, mipmapped, isProtected, props);
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

    return SkSurfaces::WrapBackendRenderTarget(
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

#ifdef SK_GRAPHITE
sk_sp<SkSurface> MakeBackendTextureSurface(skgpu::graphite::Recorder* recorder,
                                           const SkImageInfo& ii,
                                           skgpu::Origin origin,
                                           skgpu::Mipmapped mipmapped,
                                           skgpu::Protected isProtected,
                                           const SkSurfaceProps* props) {
    if (ii.alphaType() == kUnpremul_SkAlphaType) {
        return nullptr;
    }
    sk_sp<ManagedGraphiteTexture> mbet = ManagedGraphiteTexture::MakeUnInit(recorder,
                                                                            ii,
                                                                            mipmapped,
                                                                            skgpu::Renderable::kYes,
                                                                            isProtected);
    if (!mbet) {
        return nullptr;
    }
    return SkSurfaces::WrapBackendTexture(recorder,
                                          mbet->texture(),
                                          ii.colorType(),
                                          ii.refColorSpace(),
                                          props,
                                          ManagedGraphiteTexture::ReleaseProc,
                                          mbet->releaseContext());
}
#endif  // SK_GRAPHITE

}  // namespace sk_gpu_test
