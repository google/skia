/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "include/private/GrWrappedImageFactory.h"
#include "tests/Test.h"
#include "tests/TestUtils.h"
#include "tools/ToolUtils.h"

constexpr int kTextureSize = 32;

struct ReleaseContext {
    ReleaseContext(GrContext* context, const GrBackendTexture& backendTex)
        : fContext(context)
        , fBackendTex(backendTex) {
    }

    GrContext*       fContext = nullptr;
    GrBackendTexture fBackendTex;

    bool             fCalled = false;
};

static void releaseProc(void* context) {
    ReleaseContext* releaseContext = (ReleaseContext*) context;

    SkASSERT(!releaseContext->fCalled);
    releaseContext->fContext->deleteBackendTexture(releaseContext->fBackendTex);
    releaseContext->fCalled = true;
}

// Create an SkData that holds the raw pixel data required to fill in 'ii'.
static sk_sp<SkData> make_pixel_data(const SkImageInfo& ii, SkColor color) {
    sk_sp<SkData> tmp = SkData::MakeUninitialized(ii.computeMinByteSize());

    SkBitmap bm;
    bm.installPixels(ii, tmp->writable_data(), ii.minRowBytes());
    bm.eraseColor(color);
    return tmp;
}

// This test wraps a single backend texture in a GrWrappedImageFactory and then uses it
// to draw the pattern:
//
//    Red  |  Green
//  ----------------
//   Blue  |  Yellow
//
// uploading to the same backend texture between each draw.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(WrappedImageFactoryTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    GrSurfaceOrigin origin = kTopLeft_GrSurfaceOrigin;
    SkColorType ct = kRGBA_8888_SkColorType;
    SkAlphaType at = kPremul_SkAlphaType;
    sk_sp<SkColorSpace> cs = SkColorSpace::MakeSRGB();

    // For now, while developing the API, we cheat and make the backendTexture renderable
    GrBackendTexture backendTex = context->createBackendTexture(kTextureSize, kTextureSize,
                                                                ct,
                                                                SkColors::kWhite,
                                                                GrMipMapped::kNo,
                                                                GrRenderable::kYes);

    ReleaseContext releaseContext(context, backendTex);

    auto wif = GrWrappedImageFactory::Make(context, backendTex, origin, ct, at, cs,
                                           releaseProc, &releaseContext);

    SkImageInfo ii = SkImageInfo::Make({ 2*kTextureSize, 2*kTextureSize }, ct, at, cs);

    sk_sp<SkSurface> surf = SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, ii);

    SkCanvas* canvas = surf->getCanvas();

    SkColor colors[4] = { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorYELLOW };

    canvas->clear(SK_ColorBLACK);
    for (int y = 0; y < 2; ++y) {
        for (int x = 0; x < 2; ++x) {
            REPORTER_ASSERT(reporter, wif->canWritePixels());

            sk_sp<SkData> data = make_pixel_data(ii, colors[y * 2 + x]);
            SkAssertResult(wif->writePixels(0, std::move(data)));

            sk_sp<SkImage> tmp = wif->makeImageSnapshot();
            SkASSERT(tmp);

            REPORTER_ASSERT(reporter, !wif->canWritePixels());

            canvas->drawImage(std::move(tmp), x*kTextureSize, y*kTextureSize);
        }
    }

    wif.reset();
    context->flush();
    REPORTER_ASSERT(reporter, releaseContext.fCalled);

    SkBitmap readback;
    readback.allocPixels(ii);

    SkAssertResult(surf->readPixels(readback, 0, 0));

    for (int y = 0; y < 2; ++y) {
        for (int x = 0; x < 2; ++x) {
            SkColor col = readback.getColor(x*kTextureSize + kTextureSize/2,
                                            y*kTextureSize + kTextureSize/2);

            REPORTER_ASSERT(reporter, col == colors[y*2+x]);
        }
    }
}
