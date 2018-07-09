/*
 * Copyright 2018 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGlyphRun.h"

#include "SkTextBlob.h"

#include "Test.h"

DEF_TEST(GlyphRunGlyphIDSetBasic, reporter) {
    SkGlyphID glyphs[] = {100, 3, 240, 3, 234};
    auto glyphIDs = SkSpan<const SkGlyphID>(glyphs, SK_ARRAY_COUNT(glyphs));
    int universeSize = 1000;
    SkGlyphID uniqueGlyphs[SK_ARRAY_COUNT(glyphs)];
    uint16_t denseIndices[SK_ARRAY_COUNT(glyphs)];

    SkGlyphIDSet gs;
    auto uniqueGlyphIDs = gs.uniquifyGlyphIDs(universeSize, glyphIDs, uniqueGlyphs, denseIndices);

    std::vector<SkGlyphID> test{uniqueGlyphIDs.begin(), uniqueGlyphIDs.end()};
    std::sort(test.begin(), test.end());
    auto newEnd = std::unique(test.begin(), test.end());
    REPORTER_ASSERT(reporter, uniqueGlyphIDs.size() == newEnd - test.begin());
    REPORTER_ASSERT(reporter, uniqueGlyphIDs.size() == 4);
    {
        uint16_t answer[] = {0, 1, 2, 1, 3};
        REPORTER_ASSERT(reporter,
                        std::equal(answer, std::end(answer), denseIndices));
    }

    {
        SkGlyphID answer[] = {100, 3, 240, 234};
        REPORTER_ASSERT(reporter,
                        std::equal(answer, std::end(answer), uniqueGlyphs));
    }
}

DEF_TEST(GlyphRunBasic, reporter) {
    SkGlyphID glyphs[] = {100, 3, 240, 3, 234, 111, 3, 4, 10, 11};
    uint16_t count = SK_ARRAY_COUNT(glyphs);

    SkPaint paint;
    paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);

    SkGlyphRunBuilder builder;
    builder.prepareDrawText(paint, glyphs, count, SkPoint::Make(0, 0));
}
