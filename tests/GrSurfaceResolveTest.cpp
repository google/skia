/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrPixmap.h"
#include "tests/TestUtils.h"
#include "tools/gpu/ManagedBackendTexture.h"

using namespace sk_gpu_test;

bool check_pixels(skiatest::Reporter* reporter,
                  GrDirectContext* dContext,
                  const GrBackendTexture& tex,
                  const SkImageInfo& info,
                  SkColor expectedColor) {
    // We have to do the readback of the backend texture wrapped in a different Skia surface than
    // the one used in the main body of the test or else the readPixels call will trigger resolves
    // itself.
    sk_sp<SkSurface> surface = SkSurface::MakeFromBackendTexture(dContext,
                                                                 tex,
                                                                 kTopLeft_GrSurfaceOrigin,
                                                                 /*sampleCnt=*/4,
                                                                 kRGBA_8888_SkColorType,
                                                                 nullptr, nullptr);
    SkBitmap actual;
    actual.allocPixels(info);
    if (!surface->readPixels(actual, 0, 0)) {
        return false;
    }

    SkBitmap expected;
    expected.allocPixels(info);
    SkCanvas tmp(expected);
    tmp.clear(expectedColor);
    expected.setImmutable();

    const float tols[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    auto error = std::function<ComparePixmapsErrorReporter>(
        [reporter](int x, int y, const float diffs[4]) {
            SkASSERT(x >= 0 && y >= 0);
            ERRORF(reporter, "mismatch at %d, %d (%f, %f, %f %f)",
                   x, y, diffs[0], diffs[1], diffs[2], diffs[3]);
        });

    return ComparePixels(expected.pixmap(), actual.pixmap(), tols, error);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SurfaceResolveTest, reporter, ctxInfo) {
    auto dContext = ctxInfo.directContext();

    SkImageInfo info = SkImageInfo::Make(8, 8, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    auto managedTex = ManagedBackendTexture::MakeFromInfo(dContext,
                                                          info,
                                                          GrMipmapped::kNo,
                                                          GrRenderable::kYes);
    if (!managedTex) {
        return;
    }
    auto tex = managedTex->texture();
    // Wrap the backend surface but tell it rendering with MSAA so that the wrapped texture is the
    // resolve.
    sk_sp<SkSurface> surface = SkSurface::MakeFromBackendTexture(dContext,
                                                                 tex,
                                                                 kTopLeft_GrSurfaceOrigin,
                                                                 /*sampleCnt=*/4,
                                                                 kRGBA_8888_SkColorType,
                                                                 nullptr, nullptr);

    if (!surface) {
        return;
    }

    const GrCaps* caps = dContext->priv().caps();
    // In metal and vulkan if we prefer discardable msaa attachments we will also auto resolve. The
    // GrBackendTexture and SkSurface are set up in a way that is compatible with discardable msaa
    // for both backends.
    bool autoResolves = caps->msaaResolvesAutomatically() ||
                        caps->preferDiscardableMSAAAttachment();

    // First do a simple test where we clear the surface than flush with SkSurface::flush. This
    // should trigger the resolve and the texture should have the correct data.
    surface->getCanvas()->clear(SK_ColorRED);
    surface->flush();
    dContext->submit();
    REPORTER_ASSERT(reporter, check_pixels(reporter, dContext, tex, info, SK_ColorRED));

    // Next try doing a GrDirectContext::flush which will not trigger a resolve on gpus without
    // automatic msaa resolves.
    surface->getCanvas()->clear(SK_ColorBLUE);
    dContext->flush();
    dContext->submit();
    if (autoResolves) {
        REPORTER_ASSERT(reporter, check_pixels(reporter, dContext, tex, info, SK_ColorBLUE));
    } else {
        REPORTER_ASSERT(reporter, check_pixels(reporter, dContext, tex, info, SK_ColorRED));
    }

    // Now doing a surface flush (even without any queued up normal work) should still resolve the
    // surface.
    surface->flush();
    dContext->submit();
    REPORTER_ASSERT(reporter, check_pixels(reporter, dContext, tex, info, SK_ColorBLUE));

    // Test using SkSurface::resolve with a GrDirectContext::flush
    surface->getCanvas()->clear(SK_ColorRED);
    surface->resolveMSAA();
    dContext->flush();
    dContext->submit();
    REPORTER_ASSERT(reporter, check_pixels(reporter, dContext, tex, info, SK_ColorRED));

    // Calling resolve again should cause no issues as it is a no-op (there is an assert in the
    // resolve op that the surface's msaa is dirty, we shouldn't hit that assert).
    surface->resolveMSAA();
    dContext->flush();
    dContext->submit();
    REPORTER_ASSERT(reporter, check_pixels(reporter, dContext, tex, info, SK_ColorRED));

    // Try resolving in the middle of draw calls. Non automatic resolve gpus should only see the
    // results of the first draw.
    surface->getCanvas()->clear(SK_ColorGREEN);
    surface->resolveMSAA();
    surface->getCanvas()->clear(SK_ColorBLUE);
    dContext->flush();
    dContext->submit();
    if (autoResolves) {
        REPORTER_ASSERT(reporter, check_pixels(reporter, dContext, tex, info, SK_ColorBLUE));
    } else {
        REPORTER_ASSERT(reporter, check_pixels(reporter, dContext, tex, info, SK_ColorGREEN));
    }
}

