/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTextBlob.h"
#include "src/core/SkSurfacePriv.h"
#include "src/gpu/text/GrTextBlob.h"
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

DEF_TEST(GrBagOfBytesBasic, r) {
    const int k4K = 1 << 12;
    {
        // GrBagOfBytes::MinimumSizeWithOverhead(-1); // This should fail
        GrBagOfBytes::PlatformMinimumSizeWithOverhead(0, 16);
        GrBagOfBytes::PlatformMinimumSizeWithOverhead(
                std::numeric_limits<int>::max() - k4K - 1, 16);
        // GrBagOfBytes::MinimumSizeWithOverhead(std::numeric_limits<int>::max() - k4K);  // Fail
        REPORTER_ASSERT(r, GrBagOfBytes::MinimumSizeWithOverhead(0, 1, 16, 16) == 31);
        REPORTER_ASSERT(r, GrBagOfBytes::MinimumSizeWithOverhead(1, 1, 16, 16) == 32);
        REPORTER_ASSERT(r, GrBagOfBytes::MinimumSizeWithOverhead(63, 1, 16, 16) == 94);
        REPORTER_ASSERT(r, GrBagOfBytes::MinimumSizeWithOverhead(0, 8, 16, 16) == 24);
        REPORTER_ASSERT(r, GrBagOfBytes::MinimumSizeWithOverhead(1, 8, 16, 16) == 32);
        REPORTER_ASSERT(r, GrBagOfBytes::MinimumSizeWithOverhead(63, 8, 16, 16) == 88);
        REPORTER_ASSERT(r, GrBagOfBytes::MinimumSizeWithOverhead(0, 16, 16, 16) == 16);
        REPORTER_ASSERT(r, GrBagOfBytes::MinimumSizeWithOverhead(1, 16, 16, 16) == 32);
        REPORTER_ASSERT(r, GrBagOfBytes::MinimumSizeWithOverhead(63, 16, 16, 16) == 80);

        REPORTER_ASSERT(r, GrBagOfBytes::MinimumSizeWithOverhead(0, 1, 8, 16) == 23);
        REPORTER_ASSERT(r, GrBagOfBytes::MinimumSizeWithOverhead(1, 1, 8, 16) == 24);
        REPORTER_ASSERT(r, GrBagOfBytes::MinimumSizeWithOverhead(63, 1, 8, 16) == 86);
        REPORTER_ASSERT(r, GrBagOfBytes::MinimumSizeWithOverhead(0, 8, 8, 16) == 16);
        REPORTER_ASSERT(r, GrBagOfBytes::MinimumSizeWithOverhead(1, 8, 8, 16) == 24);
        REPORTER_ASSERT(r, GrBagOfBytes::MinimumSizeWithOverhead(63, 8, 8, 16) == 80);
    }

    {
        GrBagOfBytes bob;
        // bob.alignedBytes(0, 1);  // This should fail
        // bob.alignedBytes(1, 0);  // This should fail
        // bob.alignedBytes(1, 3);  // This should fail

        struct Big {
            char stuff[std::numeric_limits<int>::max()];
        };
        // bob.alignedBytes(sizeof(Big), 1);  // this should fail
        // bob.allocateBytesFor<Big>();  // this should not compile
        // The following should run, but should not be regularly tested.
        // bob.allocateBytesFor<int>((std::numeric_limits<int>::max() - (1<<12)) / sizeof(int) - 1);
        // The following should fail
        // bob.allocateBytesFor<int>((std::numeric_limits<int>::max() - (1<<12)) / sizeof(int));
        bob.alignedBytes(1, 1);  // To avoid unused variable problems.
    }

    // Force multiple block allocation
    {
        GrBagOfBytes bob;
        const int k64K = 1 << 16;
        // By default allocation block sizes start at 1K and go up with fib. This should allocate
        // 10 individual blocks.
        for (int i = 0; i < 10; i++) {
            bob.alignedBytes(k64K, 1);
        }
    }
}
