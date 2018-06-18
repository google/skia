/*
 * Copyright 2018 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGlyphRun.h"

#include "SkTextBlob.h"

#include "Test.h"

DEF_TEST(GlyphSetBasic, reporter) {
    SkGlyphSet set;

    std::vector<SkGlyphID> unique;

    set.reuse(10, &unique);
    REPORTER_ASSERT(reporter, set.add(7) == 0);
    REPORTER_ASSERT(reporter, set.add(3) == 1);
    set.reuse(10, &unique);
    REPORTER_ASSERT(reporter, set.add(5) == 0);
    REPORTER_ASSERT(reporter, set.add(8) == 1);
    REPORTER_ASSERT(reporter, set.add(3) == 2);

    REPORTER_ASSERT(reporter, unique.size() == 5);
    REPORTER_ASSERT(reporter, unique[0] == 7);
    REPORTER_ASSERT(reporter, unique[1] == 3);
    REPORTER_ASSERT(reporter, unique[2] == 5);
    REPORTER_ASSERT(reporter, unique[3] == 8);
    REPORTER_ASSERT(reporter, unique[4] == 3);
}

DEF_TEST(GlyphRunBasic, reporter) {
    SkGlyphID glyphs[] = {100, 3, 240, 3, 234, 111, 3, 4, 10, 11};
    uint16_t count = SK_ARRAY_COUNT(glyphs);

    SkPaint paint;
    paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);

    SkGlyphRunBuilder builder;
    builder.prepareDrawText(paint, glyphs, count, SkPoint::Make(0, 0));
}

DEF_TEST(GlyphRunBlob, reporter) {
    constexpr uint16_t count = 10;

    auto tf = SkTypeface::MakeFromName("monospace", SkFontStyle());

    SkPaint font;
    font.setTypeface(tf);
    font.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
    font.setTextAlign(SkPaint::kLeft_Align);
    font.setStyle(SkPaint::kFill_Style);
    font.setHinting(SkPaint::kNormal_Hinting);
    font.setTextSize(1u);

    SkTextBlobBuilder blobBuilder;
    for (int runNum = 0; runNum < 2; runNum++) {
        const auto& runBuffer = blobBuilder.allocRunPosH(font, count, runNum);
        SkASSERT(runBuffer.utf8text == nullptr);
        SkASSERT(runBuffer.clusters == nullptr);

        for (int i = 0; i < count; i++) {
            runBuffer.glyphs[i] = static_cast<SkGlyphID>(i + runNum * 10);
            runBuffer.pos[i] = SkIntToScalar(i + runNum * 10);
        }
    }

    auto blob = blobBuilder.make();

    SkPaint paint;
    paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);

    SkGlyphRunBuilder runBuilder;
    runBuilder.prepareTextBlob(font, *blob, SkPoint::Make(0, 0));

    auto runList = runBuilder.useGlyphRunList();

    REPORTER_ASSERT(reporter, runList->size() == 2);
    for (auto& run : *runList) {
        REPORTER_ASSERT(reporter, run.runSize() == 10);
        REPORTER_ASSERT(reporter, run.uniqueSize() == 10);
    }
}