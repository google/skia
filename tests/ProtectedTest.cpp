/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#if defined(SK_GANESH)

#include "include/core/SkBitmap.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
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

            dContext->submit(/* syncCpu= */ true);

            REPORTER_ASSERT(reporter, image->isProtected() == isProtected);
            ProtectedUtils::CheckImageBEProtection(image.get(), isProtected);
        }
    }

    for (bool isProtected : { true, false }) {
        ProtectedUtils::CreateProtectedSkImage(dContext, { kSize, kSize }, SkColors::kBlue,
                                               isProtected);
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

            dContext->flushAndSubmit(/* syncCpu= */ true);

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

    for (bool isProtected : { true, false }) {
        sk_sp<SkSurface> surface = ProtectedUtils::CreateProtectedSkSurface(dContext,
                                                                            { kSize, kSize },
                                                                            /* textureable= */ true,
                                                                            isProtected);
        if (!surface) {
            continue;
        }

        SkBitmap readback;
        readback.allocPixels(surface->imageInfo());
        REPORTER_ASSERT(reporter, isProtected != surface->readPixels(readback, 0, 0));
    }
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

    for (bool isProtected : { true, false }) {
        sk_sp<SkSurface> surface = ProtectedUtils::CreateProtectedSkSurface(dContext,
                                                                            { kSize, kSize },
                                                                            /* textureable= */ true,
                                                                            isProtected);
        if (!surface) {
            continue;
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
        REPORTER_ASSERT(reporter, isProtected != SkToBool(cbContext.fResult));
    }
}

#endif  // defined(SK_GANESH)
