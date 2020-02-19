/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "include/gpu/GrWrappedImageFactory.h"
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

static SkBitmap make_bitmap(SkColor color) {
    SkBitmap bm;

    SkImageInfo ii = SkImageInfo::Make({ kTextureSize, kTextureSize },
                                       kRGBA_8888_SkColorType,
                                       kPremul_SkAlphaType);

    bm.allocPixels(ii);
    bm.eraseColor(color);
    return bm;
}

// This test wraps a single backend texture in a GrWrappedImageFactory and then uses it
// to draw the pattern:
//
//    Red  |  Green
//  ----------------
//   Blue  |  Yellow
//
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(WrappedImageFactoryTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    // For now, while developing the API, we cheat and make the backendTexture renderable
    GrBackendTexture backendTex = context->createBackendTexture(kTextureSize, kTextureSize,
                                                                kRGBA_8888_SkColorType,
                                                                SkColors::kWhite,
                                                                GrMipMapped::kNo,
                                                                GrRenderable::kYes);

    ReleaseContext releaseContext(context, backendTex);

    auto wif = GrWrappedImageFactory::Make(context, backendTex, releaseProc, &releaseContext);

    SkImageInfo ii = SkImageInfo::Make({ 2*kTextureSize, 2*kTextureSize },
                                       kRGBA_8888_SkColorType,
                                       kPremul_SkAlphaType,
                                       SkColorSpace::MakeSRGB());

    sk_sp<SkSurface> surf = SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, ii);

    SkCanvas* canvas = surf->getCanvas();

    SkColor colors[4] = { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorYELLOW };

    canvas->clear(SK_ColorBLACK);
    for (int y = 0; y < 2; ++y) {
        for (int x = 0; x < 2; ++x) {
            REPORTER_ASSERT(reporter, wif->canWritePixels());

            SkBitmap bm = make_bitmap(colors[y * 2 + x]);
            SkAssertResult(wif->writePixels(bm));

            sk_sp<SkImage> tmp = wif->makeImageSnapshot();
            SkASSERT(tmp);

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
