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
    REPORTER_ASSERT(reporter, uniqueGlyphIDs.size() == (size_t)(newEnd - test.begin()));
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
    builder.drawText(paint, glyphs, count, SkPoint::Make(0, 0));
}

DEF_TEST(GlyphRunBlob, reporter) {
    constexpr uint16_t count = 5;
    constexpr int runCount = 2;

    auto tf = SkTypeface::MakeFromName("monospace", SkFontStyle());

    SkPaint font;
    font.setTypeface(tf);
    font.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
    font.setTextAlign(SkPaint::kLeft_Align);
    font.setStyle(SkPaint::kFill_Style);
    font.setHinting(SkPaint::kNormal_Hinting);
    font.setTextSize(1u);

    SkTextBlobBuilder blobBuilder;
    for (int runNum = 0; runNum < runCount; runNum++) {
        const auto& runBuffer = blobBuilder.allocRunPosH(font, count, runNum);
        SkASSERT(runBuffer.utf8text == nullptr);
        SkASSERT(runBuffer.clusters == nullptr);

        for (int i = 0; i < count; i++) {
            runBuffer.glyphs[i] = static_cast<SkGlyphID>(i + runNum * count);
            runBuffer.pos[i] = SkIntToScalar(i + runNum * count);
        }
    }

    auto blob = blobBuilder.make();

    SkPaint paint;
    paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);

    SkGlyphRunBuilder runBuilder;
    runBuilder.drawTextBlob(font, *blob, SkPoint::Make(0, 0));

    auto runList = runBuilder.useGlyphRunList();

    REPORTER_ASSERT(reporter, runList->size() == runCount);
    int runIndex = 0;
    for (auto& run : *runList) {
        REPORTER_ASSERT(reporter, run.runSize() == count);

        int index = 0;
        for (auto p : run.positions()) {
            if (p.x() != runIndex * count + index) {
                ERRORF(reporter, "x: %g != k: %d", p.x(), runIndex * count + index);
                break;
            }
            index += 1;
        }

        runIndex += 1;
    }
}
