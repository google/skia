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

// Test extreme positions for all combinations of positions, origins, and translation matrices.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrTextBlobMoveAround, reporter, ctxInfo) {
    auto tf = ToolUtils::create_portable_typeface("Mono", SkFontStyle());
    SkFont font{tf};
    font.setHinting(SkFontHinting::kNormal);
    font.setSize(12);
    font.setEdging(SkFont::Edging::kAntiAlias);
    font.setSubpixel(true);

    auto makeBlob = [&](SkPoint delta) {
        SkTextBlobBuilder builder;
        const auto& runBuffer = builder.allocRunPos(font, 30, nullptr);

        for (int i = 0; i < 30; i++) {
            runBuffer.glyphs[i] = static_cast<SkGlyphID>(i);
            runBuffer.points()[i] = SkPoint::Make(SkIntToScalar(i*10) + delta.x(), 50 + delta.y());
        }
        return builder.make();
    };

    auto dContext = ctxInfo.directContext();
    auto rasterizeBlob = [&](SkTextBlob* blob, SkPoint origin, const SkMatrix& matrix) {
        SkPaint paint;
        const SkImageInfo info =
                SkImageInfo::Make(350, 80, kN32_SkColorType, kPremul_SkAlphaType);
        auto surface = SkSurface::MakeRenderTarget(dContext, SkBudgeted::kNo, info);
        auto canvas = surface->getCanvas();
        canvas->drawColor(SK_ColorWHITE);
        canvas->concat(matrix);
        canvas->drawTextBlob(blob, 10 + origin.x(), 40 + origin.y(), paint);
        SkBitmap bitmap;
        bitmap.allocN32Pixels(350, 80);
        surface->readPixels(bitmap, 0, 0);
        return bitmap;
    };

    SkBitmap benchMark;
    {
        auto blob = makeBlob({0, 0});
        benchMark = rasterizeBlob(blob.get(), {0,0}, SkMatrix::I());
    }

    auto checkBitmap = [&](const SkBitmap& bitmap) {
        REPORTER_ASSERT(reporter, benchMark.width() == bitmap.width());
        REPORTER_ASSERT(reporter, benchMark.width() == bitmap.width());

        for (int y = 0; y < benchMark.height(); y++) {
            for (int x = 0; x < benchMark.width(); x++) {
                if (benchMark.getColor(x, y) != bitmap.getColor(x, y)) {
                    return false;
                }
            }
        }
        return true;
    };

    SkScalar interestingNumbers[] = {-10'000'000, -1'000'000, -1, 0, +1, +1'000'000, +10'000'000};
    for (auto originX : interestingNumbers) {
        for (auto originY : interestingNumbers) {
            for (auto translateX : interestingNumbers) {
                for (auto translateY : interestingNumbers) {
                    // Make sure everything adds to zero.
                    SkScalar deltaPosX = -(originX + translateX);
                    SkScalar deltaPosY = -(originY + translateY);
                    auto blob = makeBlob({deltaPosX, deltaPosY});
                    SkMatrix t = SkMatrix::Translate(translateX, translateY);
                    auto bitmap = rasterizeBlob(blob.get(), {originX, originY}, t);
                    REPORTER_ASSERT(reporter, checkBitmap(bitmap));
                }
            }
        }
    }
}
