/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkEnumerate.h"
#include "src/core/SkGlyphBuffer.h"
#include "src/core/SkGlyphRunPainter.h"
#include "src/core/SkScalerContext.h"
#include "tests/Test.h"


DEF_TEST(SkSourceGlyphBufferBasic, reporter) {
    SkSourceGlyphBuffer rejects;
    // Positions are picked to avoid precision problems.
    const SkPoint positions[] = {{10.25,10.25}, {20.5,10.25}, {30.75,10.25}, {40,10.25}};
    const SkGlyphID glyphIDs[] = {1, 2, 3, 4};
    auto source = SkMakeZip(glyphIDs, positions);

    rejects.setSource(source);
    for (auto t : SkMakeEnumerate(rejects.source())) {
        size_t i; SkGlyphID glyphID; SkPoint pos;
        std::forward_as_tuple(i, std::tie(glyphID, pos)) = t;
        REPORTER_ASSERT(reporter, glyphID == std::get<0>(source[i]));
        REPORTER_ASSERT(reporter, pos == std::get<1>(source[i]));
    }
    // Reject a couple of glyphs.
    rejects.reject(1);
    rejects.reject(2, 100);
    rejects.flipRejectsToSource();
    REPORTER_ASSERT(reporter, rejects.rejectedMaxDimension() == 100);
    for (auto t : SkMakeEnumerate(rejects.source())) {
        size_t i; SkGlyphID glyphID; SkPoint pos;
        std::forward_as_tuple(i, std::tie(glyphID, pos)) = t;
        // This will index 1 and 2 from the original source.
        size_t j = i + 1;
        REPORTER_ASSERT(reporter, glyphID == std::get<0>(source[j]));
        REPORTER_ASSERT(reporter, pos == std::get<1>(source[j]));
    }

    // Reject an additional glyph
    rejects.reject(0, 10);
    rejects.flipRejectsToSource();
    REPORTER_ASSERT(reporter, rejects.rejectedMaxDimension() == 10);
    for (auto t : SkMakeEnumerate(rejects.source())) {
        size_t i; SkGlyphID glyphID; SkPoint pos;
        std::forward_as_tuple(i, std::tie(glyphID, pos)) = t;
        // This will index 1 from the original source.
        size_t j = i + 1;
        REPORTER_ASSERT(reporter, glyphID == std::get<0>(source[j]));
        REPORTER_ASSERT(reporter, pos == std::get<1>(source[j]));
    }

    // Start all over
    rejects.setSource(source);
    for (auto t : SkMakeEnumerate(rejects.source())) {
        size_t i; SkGlyphID glyphID; SkPoint pos;
        std::forward_as_tuple(i, std::tie(glyphID, pos)) = t;
        REPORTER_ASSERT(reporter, glyphID == std::get<0>(source[i]));
        REPORTER_ASSERT(reporter, pos == std::get<1>(source[i]));
    }

    // Check that everything is working after calling setSource.
    rejects.reject(1);
    rejects.reject(2, 100);
    rejects.flipRejectsToSource();
    REPORTER_ASSERT(reporter, rejects.rejectedMaxDimension() == 100);
    for (auto t : SkMakeEnumerate(rejects.source())) {
        size_t i; SkGlyphID glyphID; SkPoint pos;
        std::forward_as_tuple(i, std::tie(glyphID, pos)) = t;
        // This will index 1 and 2 from the original source.
        size_t j = i + 1;
        REPORTER_ASSERT(reporter, glyphID == std::get<0>(source[j]));
        REPORTER_ASSERT(reporter, pos == std::get<1>(source[j]));
    }
}

DEF_TEST(SkDrawableGlyphBufferBasic, reporter) {
    // Positions are picked to avoid precision problems.
    const SkPoint positions[] = {{10.25,10.25}, {20.5,10.25}, {30.75,10.25}, {40,10.25}};
    const SkGlyphID glyphIDs[] = {1, 2, 3, 4};
    SkGlyph glyphs[100];
    auto source = SkMakeZip(glyphIDs, positions);

    {
        SkDrawableGlyphBuffer drawable;
        drawable.ensureSize(100);
        drawable.startSource(source, {100, 100});
        for (auto t : SkMakeEnumerate(drawable.input())) {
            size_t i; SkGlyphVariant packedID; SkPoint pos;
            std::forward_as_tuple(i, std::tie(packedID, pos)) = t;
            REPORTER_ASSERT(reporter, packedID.packedID().value() == glyphIDs[i]);
            REPORTER_ASSERT(reporter, pos == positions[i] + SkPoint::Make(100, 100));
        }
    }

    {
        SkDrawableGlyphBuffer drawable;
        drawable.ensureSize(100);
        SkMatrix matrix = SkMatrix::MakeScale(0.5);
        SkGlyphPositionRoundingSpec rounding{true, kX_SkAxisAlignment};
        drawable.startDevice(source, {100, 100}, matrix, rounding);
        for (auto t : SkMakeEnumerate(drawable.input())) {
            size_t i; SkGlyphVariant packedID; SkPoint pos;
            std::forward_as_tuple(i, std::tie(packedID, pos)) = t;
            REPORTER_ASSERT(reporter, glyphIDs[i] == packedID.packedID().glyphID());
            REPORTER_ASSERT(reporter, pos.x() == positions[i].x() * 0.5 + 50 + 0.125);
            REPORTER_ASSERT(reporter, pos.y() == positions[i].y() * 0.5 + 50 + 0.5);
        }
    }

    {
        SkDrawableGlyphBuffer drawable;
        drawable.ensureSize(100);
        drawable.startSource(source, {100, 100});
        for (auto t : SkMakeEnumerate(drawable.input())) {
            size_t i; SkGlyphVariant packedID; SkPoint pos;
            std::forward_as_tuple(i, std::tie(packedID, pos)) = t;
            drawable.push_back(&glyphs[i], i);
        }
        for (auto t : SkMakeEnumerate(drawable.drawable())) {
            size_t i; SkGlyphVariant glyph; SkPoint pos;
            std::forward_as_tuple(i, std::tie(glyph, pos)) = t;
            REPORTER_ASSERT(reporter, glyph.glyph() == &glyphs[i]);
        }
    }
}
