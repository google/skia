/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/ProtectedUtils.h"

#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "tools/gpu/BackendSurfaceFactory.h"
#include "tools/gpu/BackendTextureImageFactory.h"

namespace ProtectedUtils {

sk_sp<SkSurface> CreateProtectedSkSurface(GrDirectContext* dContext,
                                          SkISize size,
                                          bool textureable,
                                          bool isProtected) {
    SkSurfaceProps surfaceProps = SkSurfaceProps(0, kRGB_H_SkPixelGeometry);
    sk_sp<SkSurface> surface;
    if (textureable) {
        surface = sk_gpu_test::MakeBackendTextureSurface(dContext,
                                                         size,
                                                         kTopLeft_GrSurfaceOrigin,
                                                         1,
                                                         kRGBA_8888_SkColorType,
                                                         /* colorSpace= */ nullptr,
                                                         skgpu::Mipmapped::kNo,
                                                         skgpu::Protected(isProtected),
                                                         &surfaceProps);
    } else {
        surface = sk_gpu_test::MakeBackendRenderTargetSurface(dContext,
                                                              size,
                                                              kTopLeft_GrSurfaceOrigin,
                                                              1,
                                                              kRGBA_8888_SkColorType,
                                                              /* colorSpace= */ nullptr,
                                                              skgpu::Protected(isProtected),
                                                              &surfaceProps);
    }
    if (!surface) {
        SK_ABORT("Could not create %s surface.", isProtected ? "protected" : "unprotected");
        return nullptr;
    }

    SkCanvas* canvas = surface->getCanvas();

    canvas->clear(SkColors::kBlue);

    if (textureable) {
        GrBackendTexture backendTex = SkSurfaces::GetBackendTexture(
                surface.get(), SkSurfaces::BackendHandleAccess::kFlushRead);
        SkASSERT(backendTex.isValid());
        SkASSERT(backendTex.isProtected() == isProtected);
    } else {
        GrBackendRenderTarget backendRT = SkSurfaces::GetBackendRenderTarget(
                surface.get(), SkSurfaces::BackendHandleAccess::kFlushRead);
        SkASSERT(backendRT.isValid());
        SkASSERT(backendRT.isProtected() == isProtected);
    }

    return surface;
}

void CheckImageBEProtection(SkImage* image, bool expectingProtected) {
    GrBackendTexture beTex;
    GrSurfaceOrigin origin;
    bool result = SkImages::GetBackendTextureFromImage(image,
                                                       &beTex,
                                                       /* flushPendingGrContextIO= */ true,
                                                       &origin);
    if (!result) {
        SK_ABORT("GetBackendTextureFromImage failed");
        return;
    }

    SkASSERT(beTex.isValid());
    SkASSERT(beTex.isProtected() == expectingProtected);
}

sk_sp<SkImage> CreateProtectedSkImage(GrDirectContext* dContext,
                                      SkISize size,
                                      SkColor4f color,
                                      bool isProtected) {
    SkImageInfo ii = SkImageInfo::Make(size, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    sk_sp<SkImage> image = sk_gpu_test::MakeBackendTextureImage(dContext,
                                                                ii,
                                                                color,
                                                                skgpu::Mipmapped::kNo,
                                                                GrRenderable::kNo,
                                                                kTopLeft_GrSurfaceOrigin,
                                                                skgpu::Protected(isProtected));
    if (!image) {
        SK_ABORT("Could not create %s image.", isProtected ? "protected" : "unprotected");
        return nullptr;
    }

    CheckImageBEProtection(image.get(), isProtected);

    return image;
}

}  // namespace ProtectedUtils
