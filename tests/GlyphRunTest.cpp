/*
 * Copyright 2018 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGlyphRun.h"

#include "SkTextBlob.h"

#include "Test.h"

DEF_TEST(GlyphRunBasic, reporter) {
    SkGlyphID glyphs[] = {100, 3, 240, 3, 234, 111, 3, 4, 10, 11};
    uint16_t count = SK_ARRAY_COUNT(glyphs);

    SkPaint paint;
    paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);

    SkGlyphRunBuilder builder;
    builder.prepareDrawText(paint, glyphs, count, SkPoint::Make(0, 0));
}

DEF_TEST(GlyphRunBlob, reporter) {
    uint16_t count = 10;

    auto tf = SkTypeface::MakeFromName("monospace", SkFontStyle());

    SkPaint font;
    font.setTypeface(tf);
    font.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
    font.setTextAlign(SkPaint::kLeft_Align);
    font.setStyle(SkPaint::kFill_Style);
    font.setHinting(SkPaint::kNormal_Hinting);
    font.setTextSize(1u);

    SkTextBlobBuilder blobBuilder;
    SkRect bounds = SkRect::MakeWH(10, 10);
    for (int runNum = 0; runNum < 2; runNum++) {
        const auto& runBuffer = blobBuilder.allocRunPosH(font, count, runNum, &bounds);
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