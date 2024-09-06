/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#if defined(SK_GANESH)

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "tests/CtsEnforcement.h"
#include "tools/gpu/ProtectedUtils.h"

static const int kSize = 8;

DEF_GANESH_TEST_FOR_ALL_CONTEXTS(Protected_SmokeTest, reporter, ctxInfo, CtsEnforcement::kNever) {
    auto dContext = ctxInfo.directContext();

    if (!dContext->supportsProtectedContent()) {
        // Protected content not supported
        return;
    }

    for (bool textureable : { true, false }) {
        for (bool isProtected : { true, false }) {
            if (!isProtected && GrBackendApi::kVulkan == dContext->backend()) {
                continue;
            }

            sk_sp<SkSurface> surface = ProtectedUtils::CreateProtectedSkSurface(dContext,
                                                                                { kSize, kSize },
                                                                                textureable,
                                                                                isProtected);
            if (!surface) {
                continue;
            }

            sk_sp<SkImage> image = surface->makeImageSnapshot();
            if (!image) {
                ERRORF(reporter, "Could not makeImageSnapshot from a %s surface.",
                       isProtected ? "protected" : "unprotected");
                continue;
            }

            dContext->submit(GrSyncCpu::kYes);

            REPORTER_ASSERT(reporter, image->isProtected() == isProtected);
            ProtectedUtils::CheckImageBEProtection(image.get(), isProtected);
        }
    }

    for (bool isProtected : { true, false }) {
        if (!isProtected && GrBackendApi::kVulkan == dContext->backend()) {
            continue;
        }

        sk_sp<SkImage> image = ProtectedUtils::CreateProtectedSkImage(dContext,
                                                                      { kSize, kSize },
                                                                      SkColors::kBlue,
                                                                      isProtected);
        if (!image) {
            continue;
        }

        dContext->submit(GrSyncCpu::kYes);

        REPORTER_ASSERT(reporter, image->isProtected() == isProtected);
        ProtectedUtils::CheckImageBEProtection(image.get(), isProtected);
    }

    for (bool renderable : { true, false }) {
        for (bool isProtected : { true, false }) {
            GrBackendTexture beTex = dContext->createBackendTexture(16,
                                                                    16,
                                                                    kRGBA_8888_SkColorType,
                                                                    SkColors::kTransparent,
                                                                    skgpu::Mipmapped::kNo,
                                                                    GrRenderable(renderable),
                                                                    GrProtected(isProtected));

            REPORTER_ASSERT(reporter, beTex.isValid());
            REPORTER_ASSERT(reporter, beTex.isProtected() == isProtected);

            dContext->flushAndSubmit(GrSyncCpu::kYes);

            {
                sk_sp<SkImage> img = SkImages::BorrowTextureFrom(dContext, beTex,
                                                                 kTopLeft_GrSurfaceOrigin,
                                                                 kRGBA_8888_SkColorType,
                                                                 kPremul_SkAlphaType,
                                                                 /* colorSpace= */ nullptr);

                REPORTER_ASSERT(reporter, img->isProtected() == isProtected);
            }

            if (beTex.isValid()) {
                dContext->deleteBackendTexture(beTex);
            }
        }
    }
}

// Verify that readPixels fails on protected surfaces
DEF_GANESH_TEST_FOR_ALL_CONTEXTS(Protected_readPixelsFromSurfaces, reporter, ctxInfo,
                                 CtsEnforcement::kNever) {
    auto dContext = ctxInfo.directContext();

    if (!dContext->supportsProtectedContent()) {
        // Protected content not supported
        return;
    }

    sk_sp<SkSurface> surface = ProtectedUtils::CreateProtectedSkSurface(dContext,
                                                                        { kSize, kSize },
                                                                        /* textureable= */ true,
                                                                        /* isProtected= */ true);
    if (!surface) {
        return;
    }

    SkBitmap readback;
    readback.allocPixels(surface->imageInfo());
    REPORTER_ASSERT(reporter, !surface->readPixels(readback, 0, 0));
}

// Graphite does not perform Copy-on-Write which is why there is no DEF_GRAPHITE_TEST correlate
DEF_GANESH_TEST_FOR_ALL_CONTEXTS(Protected_CopyOnWrite, reporter, ctxInfo, CtsEnforcement::kNever) {
    auto dContext = ctxInfo.directContext();

    if (!dContext->supportsProtectedContent()) {
        // Protected content not supported
        return;
    }

    SkImageInfo ii = SkImageInfo::Make({ kSize, kSize },
                                       kRGBA_8888_SkColorType,
                                       kPremul_SkAlphaType);

    // We can't use ProtectedUtils::CreateProtectedSkSurface here bc that will wrap a backend
    // texture which blocks the copy-on-write-behavior
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(dContext,
                                                        skgpu::Budgeted::kNo,
                                                        ii,
                                                        /* sampleCount= */ 1,
                                                        kBottomLeft_GrSurfaceOrigin,
                                                        /* surfaceProps= */ nullptr,
                                                        /* shouldCreateWithMips= */ false,
                                                        /* isProtected= */ true);
    if (!surface) {
        return;
    }

    SkCanvas* canvas = surface->getCanvas();
    REPORTER_ASSERT(reporter, canvas);

    sk_sp<SkImage> imageBefore = surface->makeImageSnapshot();
    REPORTER_ASSERT(reporter, imageBefore);
    REPORTER_ASSERT(reporter, imageBefore->isProtected());

    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    canvas->drawRect(SkRect::MakeWH(4, 4), paint);

    sk_sp<SkImage> imageAfter = surface->makeImageSnapshot();
    REPORTER_ASSERT(reporter, imageAfter);
    REPORTER_ASSERT(reporter, imageAfter->isProtected());

    REPORTER_ASSERT(reporter, imageBefore != imageAfter);

    SkBitmap readback;
    readback.allocPixels(imageAfter->imageInfo());
    REPORTER_ASSERT(reporter, !imageAfter->readPixels(dContext, readback.pixmap(), 0, 0));
}


namespace {

struct AsyncContext {
    bool fCalled = false;
    std::unique_ptr<const SkSurface::AsyncReadResult> fResult;
};

static void async_callback(void* c, std::unique_ptr<const SkSurface::AsyncReadResult> result) {
    auto context = static_cast<AsyncContext*>(c);
    context->fResult = std::move(result);
    context->fCalled = true;
}

}  // anonymous namespace

// Verify that asyncRescaleAndReadPixels fails on protected surfaces
DEF_GANESH_TEST_FOR_ALL_CONTEXTS(Protected_asyncRescaleAndReadPixelsFromSurfaces, reporter, ctxInfo,
                                 CtsEnforcement::kNever) {
    auto dContext = ctxInfo.directContext();

    if (!dContext->supportsProtectedContent()) {
        // Protected content not supported
        return;
    }

    sk_sp<SkSurface> surface = ProtectedUtils::CreateProtectedSkSurface(dContext,
                                                                        { kSize, kSize },
                                                                        /* textureable= */ true,
                                                                        /* isProtected= */ true);
    if (!surface) {
        return;
    }

    AsyncContext cbContext;

    surface->asyncRescaleAndReadPixels(surface->imageInfo(),
                                       SkIRect::MakeWH(surface->width(), surface->height()),
                                       SkSurface::RescaleGamma::kSrc,
                                       SkSurface::RescaleMode::kNearest,
                                       async_callback, &cbContext);
    dContext->submit();
    while (!cbContext.fCalled) {
        dContext->checkAsyncWorkCompletion();
    }
    REPORTER_ASSERT(reporter, !cbContext.fResult);
}

#endif  // defined(SK_GANESH)
