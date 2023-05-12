/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#if defined(SK_GANESH)

#include "include/core/SkSurface.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "tests/CtsEnforcement.h"
#include "tools/gpu/BackendSurfaceFactory.h"
#include "tools/gpu/BackendTextureImageFactory.h"

#ifdef SK_GL
#include "src/gpu/ganesh/gl/GrGLCaps.h"
#endif
#ifdef SK_VULKAN
#include "src/gpu/ganesh/vk/GrVkCaps.h"
#endif

namespace {

bool context_supports_protected(GrDirectContext* dContext) {
    [[maybe_unused]] const GrCaps* caps = dContext->priv().caps();

#ifdef SK_GL
    if (dContext->backend() == GrBackendApi::kOpenGL) {
        const GrGLCaps* glCaps = static_cast<const GrGLCaps*>(caps);
        return glCaps->supportsProtected();
    }
#endif
#ifdef SK_VULKAN
    if (dContext->backend() == GrBackendApi::kVulkan) {
        const GrVkCaps* vkCaps = static_cast<const GrVkCaps*>(caps);
        return vkCaps->supportsProtectedMemory();
    }
#endif
    if (dContext->backend() == GrBackendApi::kMock) {
        return true;
    }

    // Metal, Dawn and D3D don't support protected textures
    return false;
}

sk_sp<SkSurface> create_protected_sksurface(GrDirectContext* dContext,
                                            skiatest::Reporter* reporter,
                                            bool textureable,
                                            bool isProtected) {
    const int kW = 8;
    const int kH = 8;
    SkSurfaceProps surfaceProps = SkSurfaceProps(0, kRGB_H_SkPixelGeometry);
    sk_sp<SkSurface> surface;
    if (textureable) {
        surface = sk_gpu_test::MakeBackendTextureSurface(dContext,
                                                         {kW, kH},
                                                         kTopLeft_GrSurfaceOrigin,
                                                         1,
                                                         kRGBA_8888_SkColorType,
                                                         /* colorSpace= */ nullptr,
                                                         GrMipmapped::kNo,
                                                         skgpu::Protected(isProtected),
                                                         &surfaceProps);
    } else {
        surface = sk_gpu_test::MakeBackendRenderTargetSurface(dContext,
                                                              {kW, kH},
                                                              kTopLeft_GrSurfaceOrigin,
                                                              1,
                                                              kRGBA_8888_SkColorType,
                                                              /* colorSpace= */ nullptr,
                                                              skgpu::Protected(isProtected),
                                                              &surfaceProps);
    }
    if (!surface) {
        ERRORF(reporter, "Could not create %s surface.", isProtected ? "protected" : "unprotected");
        return nullptr;
    }

    if (textureable) {
        GrBackendTexture backendTex = SkSurfaces::GetBackendTexture(
                surface.get(), SkSurfaces::BackendHandleAccess::kFlushRead);
        REPORTER_ASSERT(reporter, backendTex.isValid());
        REPORTER_ASSERT(reporter, backendTex.isProtected() == isProtected);
    } else {
        GrBackendRenderTarget backendRT = SkSurfaces::GetBackendRenderTarget(
                surface.get(), SkSurfaces::BackendHandleAccess::kFlushRead);
        REPORTER_ASSERT(reporter, backendRT.isValid());
        REPORTER_ASSERT(reporter, backendRT.isProtected() == isProtected);
    }

    return surface;
}

void check_image_be_protection(SkImage* image,
                               skiatest::Reporter* reporter,
                               bool expectingProtected) {
    GrBackendTexture beTex;
    GrSurfaceOrigin origin;
    bool result = SkImages::GetBackendTextureFromImage(image,
                                                       &beTex,
                                                       true,
                                                       &origin);
    if (!result) {
        ERRORF(reporter, "GetBackendTextureFromImage failed");
        return;
    }

    REPORTER_ASSERT(reporter, beTex.isValid());
    REPORTER_ASSERT(reporter, beTex.isProtected() == expectingProtected);
}

sk_sp<SkImage> create_protected_skimage(GrDirectContext* dContext,
                                        skiatest::Reporter* reporter,
                                        bool isProtected) {
    const int kW = 8;
    const int kH = 8;

    SkImageInfo ii = SkImageInfo::Make(kW, kH, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    sk_sp<SkImage> image = sk_gpu_test::MakeBackendTextureImage(dContext,
                                                                ii,
                                                                SkColors::kBlue,
                                                                GrMipmapped::kNo,
                                                                GrRenderable::kNo,
                                                                kTopLeft_GrSurfaceOrigin,
                                                                skgpu::Protected(isProtected));
    if (!image) {
        ERRORF(reporter, "Could not create %s image.", isProtected ? "protected" : "unprotected");
        return nullptr;
    }

    check_image_be_protection(image.get(), reporter, isProtected);

    return image;
}


} // anonymous namespace

DEF_GANESH_TEST_FOR_ALL_CONTEXTS(Protected_SmokeTest, reporter, ctxInfo, CtsEnforcement::kNever) {
    auto dContext = ctxInfo.directContext();

    if (!context_supports_protected(dContext)) {
        // Protected content not supported
        return;
    }

    for (bool textureable : { true, false }) {
        for (bool isProtected : { true, false }) {
            sk_sp<SkSurface> surface = create_protected_sksurface(dContext, reporter, textureable,
                                                                  isProtected);

            sk_sp<SkImage> image = surface->makeImageSnapshot();

            dContext->submit(/* syncCpu= */ true);

            check_image_be_protection(image.get(), reporter, isProtected);
        }
    }

    for (bool isProtected : { true, false }) {
        create_protected_skimage(dContext, reporter, isProtected);
    }

    for (bool renderable : { true, false }) {
        for (bool isProtected : { true, false }) {
            GrBackendTexture beTex = dContext->createBackendTexture(16,
                                                                    16,
                                                                    kRGBA_8888_SkColorType,
                                                                    SkColors::kTransparent,
                                                                    GrMipmapped::kNo,
                                                                    GrRenderable(renderable),
                                                                    GrProtected(isProtected));

            REPORTER_ASSERT(reporter, beTex.isValid());
            REPORTER_ASSERT(reporter, beTex.isProtected() == isProtected);
        }
    }
}

#endif  // defined(SK_GANESH)
