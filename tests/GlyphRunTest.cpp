/*
 * Copyright 2018 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkGlyphRun.h"

#include "include/core/SkTextBlob.h"
#include "tests/Test.h"

#include <algorithm>
#include <memory>


#if 0   // should we revitalize this by consing up a device for drawTextBlob() ?
DEF_TEST(GlyphRunBlob, reporter) {
    constexpr uint16_t count = 5;
    constexpr int runCount = 2;

    auto tf = SkTypeface::MakeFromName("monospace", SkFontStyle());

    SkFont font;
    font.setTypeface(tf);
    font.setHinting(SkFontHinting::kNormal);
    font.setSize(1u);

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

    SkGlyphRunBuilder runBuilder;
    SkPaint legacy_paint;
    font.LEGACY_applyToPaint(&legacy_paint);
    runBuilder.drawTextBlob(legacy_paint, *blob, SkPoint::Make(0, 0));

    auto runList = runBuilder.useGlyphRunList();

    REPORTER_ASSERT(reporter, runList.size() == runCount);
    int runIndex = 0;
    for (auto& run : runList) {
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
#endif
