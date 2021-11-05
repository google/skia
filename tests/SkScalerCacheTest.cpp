/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFont.h"
#include "include/core/SkTypeface.h"
#include "src/core/SkScalerCache.h"
#include "src/core/SkStrikeSpec.h"
#include "src/core/SkTaskGroup.h"
#include "tests/Test.h"
#include "tools/ToolUtils.h"

#include <atomic>

class Barrier {
public:
    Barrier(int threadCount) : fThreadCount(threadCount) { }
    void waitForAll() {
        fThreadCount -= 1;
        while (fThreadCount > 0) { }
    }

private:
    std::atomic<int> fThreadCount;
};

DEF_TEST(SkScalerCacheMultiThread, Reporter) {
    sk_sp<SkTypeface> typeface =
            ToolUtils::create_portable_typeface("serif", SkFontStyle::Italic());
    static constexpr int kThreadCount = 4;

    Barrier barrier{kThreadCount};

    SkFont font;
    font.setEdging(SkFont::Edging::kAntiAlias);
    font.setSubpixel(true);
    font.setTypeface(typeface);

    SkGlyphID glyphs['z'];
    SkPoint pos['z'];
    for (int c = ' '; c < 'z'; c++) {
        glyphs[c] = font.unicharToGlyph(c);
        pos[c] = {30.0f * c + 30, 30.0f};
    }
    constexpr size_t glyphCount = 'z' - ' ';
    auto data = SkMakeZip(glyphs, pos).subspan(SkTo<int>(' '), glyphCount);

    SkPaint defaultPaint;
    SkStrikeSpec strikeSpec = SkStrikeSpec::MakeMask(
            font, defaultPaint, SkSurfaceProps(0, kUnknown_SkPixelGeometry),
            SkScalerContextFlags::kNone, SkMatrix::I());

    // Make our own executor so the --threads parameter doesn't mess things up.
    auto executor = SkExecutor::MakeFIFOThreadPool(kThreadCount);
    for (int tries = 0; tries < 100; tries++) {
        SkScalerCache scalerCache{strikeSpec.createScalerContext()};

        auto perThread = [&](int threadIndex) {
            barrier.waitForAll();

            auto local = data.subspan(threadIndex * 2, data.size() - kThreadCount * 2);
            for (int i = 0; i < 100; i++) {
                SkDrawableGlyphBuffer drawable;
                SkSourceGlyphBuffer rejects;

                drawable.ensureSize(glyphCount);
                rejects.setSource(local);

                drawable.startBitmapDevice(rejects.source(), {0, 0}, SkMatrix::I(),
                                           scalerCache.roundingSpec());
                scalerCache.prepareForMaskDrawing(&drawable, &rejects);
                rejects.flipRejectsToSource();
                drawable.reset();
            }
        };

        SkTaskGroup(*executor).batch(kThreadCount, perThread);
    }
}
