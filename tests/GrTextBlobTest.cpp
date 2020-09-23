/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTextBlob.h"
#include "src/core/SkSurfacePriv.h"
#include "tests/Test.h"
#include "tools/ToolUtils.h"

SkBitmap rasterize_blob(SkTextBlob* blob,
                        const SkPaint& paint,
                        GrRecordingContext* rContext,
                        const SkMatrix& matrix) {
    const SkImageInfo info =
            SkImageInfo::Make(500, 500, kN32_SkColorType, kPremul_SkAlphaType);
    auto surface = SkSurface::MakeRenderTarget(rContext, SkBudgeted::kNo, info);
    auto canvas = surface->getCanvas();
    canvas->drawColor(SK_ColorWHITE);
    canvas->concat(matrix);
    canvas->drawTextBlob(blob, 10, 250, paint);
    SkBitmap bitmap;
    bitmap.allocN32Pixels(500, 500);
    surface->readPixels(bitmap, 0, 0);
    return bitmap;
}

bool check_for_black(const SkBitmap& bm) {
    for (int y = 0; y < bm.height(); y++) {
        for (int x = 0; x < bm.width(); x++) {
            if (bm.getColor(x, y) == SK_ColorBLACK) {
                return true;
            }
        }
    }
    return false;
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrTextBlobScaleAnimation, reporter, ctxInfo) {
    auto tf = ToolUtils::create_portable_typeface("Mono", SkFontStyle());
    SkFont font{tf};
    font.setHinting(SkFontHinting::kNormal);
    font.setSize(12);
    font.setEdging(SkFont::Edging::kAntiAlias);
    font.setSubpixel(true);

    SkTextBlobBuilder builder;
    const auto& runBuffer = builder.allocRunPosH(font, 30, 0, nullptr);

    for (int i = 0; i < 30; i++) {
        runBuffer.glyphs[i] = static_cast<SkGlyphID>(i);
        runBuffer.pos[i] = SkIntToScalar(i);
    }
    auto blob = builder.make();

    auto dContext = ctxInfo.directContext();
    bool anyBlack = false;
    for (int n = -13; n < 5; n++) {
        SkMatrix m = SkMatrix::Scale(std::exp2(n), std::exp2(n));
        auto bm = rasterize_blob(blob.get(), SkPaint(), dContext, m);
        anyBlack |= check_for_black(bm);
    }
    REPORTER_ASSERT(reporter, anyBlack);
}
