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

DEF_TEST(SkPackedGlyphIDCtor, reporter) {
    using PG = SkPackedGlyphID;
    // x and y are in one quarter the sub-pixel sampling frequency.
    // Number of steps on the interval [0, 1)
    const int perUnit = 1u << (PG::kSubPixelPosLen + 2);
    const float step = 1.f / perUnit;
    const int testLimit = 2 * perUnit;
    auto freqRound = [](uint32_t x) -> uint32_t {
        return ((x + 2) >> 2) & PG::kSubPixelPosMask;
    };

    {
        // Normal sub-pixel with y-axis snapping.
        auto roundingSpec = SkGlyphPositionRoundingSpec(true, kX_SkAxisAlignment);
        SkIPoint mask = roundingSpec.ignorePositionFieldMask;
        for (int x = -testLimit; x < testLimit; x++) {
            float fx = x * step;
            SkPoint roundedPos = SkPoint{fx, 0} + roundingSpec.halfAxisSampleFreq;
            SkPackedGlyphID packedID{3, roundedPos, mask};
            uint32_t subX = freqRound(x);
            uint32_t subY = 0;
            SkPackedGlyphID correctID(3, subX, subY);
            SkASSERT(packedID == correctID);
            REPORTER_ASSERT(reporter, packedID == correctID);
        }
    }

    {
        // No subpixel positioning.
        auto roundingSpec = SkGlyphPositionRoundingSpec(false, kNone_SkAxisAlignment);
        SkIPoint mask = roundingSpec.ignorePositionFieldMask;
        for (int y = -testLimit; y < testLimit; y++) {
            for (int x = -testLimit; x < testLimit; x++) {
                float fx = x * step, fy = y * step;
                SkPoint roundedPos = SkPoint{fx, fy} + roundingSpec.halfAxisSampleFreq;
                SkPackedGlyphID packedID{3, roundedPos, mask};
                uint32_t subX = 0;
                uint32_t subY = 0;
                SkPackedGlyphID correctID(3, subX, subY);
                REPORTER_ASSERT(reporter, packedID == correctID);
            }
        }
    }

    {
        // Subpixel with no axis snapping.
        auto roundingSpec = SkGlyphPositionRoundingSpec(true, kNone_SkAxisAlignment);
        SkIPoint mask = roundingSpec.ignorePositionFieldMask;
        for (int y = -testLimit; y < testLimit; y++) {
            for (int x = -testLimit; x < testLimit; x++) {
                float fx = x * step, fy = y * step;
                SkPoint roundedPos = SkPoint{fx, fy} + roundingSpec.halfAxisSampleFreq;
                SkPackedGlyphID packedID{3, roundedPos, mask};
                uint32_t subX = freqRound(x);
                uint32_t subY = freqRound(y);
                SkPackedGlyphID correctID(3, subX, subY);
                REPORTER_ASSERT(reporter, packedID == correctID);
            }
        }
    }

    {
        // Test dynamic range by transposing a large distance.
        // Floating point numbers have 24 bits of precision. The largest distance is 24 - 2 (for
        // sub-pixel) - 1 (for truncation to floor trick in the code). This leaves 21 bits. Large
        // Distance is 2^21 - 2 (because the test is on the interval [-2, 2).
        const uint32_t kLogLargeDistance = 24 - PG::kSubPixelPosLen - 1;
        const int64_t kLargeDistance = (1ull << kLogLargeDistance) - 2;
        auto roundingSpec = SkGlyphPositionRoundingSpec(true, kNone_SkAxisAlignment);
        SkIPoint mask = roundingSpec.ignorePositionFieldMask;
        for (int y = -32; y < 33; y++) {
            for (int x = -32; x < 33; x++) {
                float fx = x * step + kLargeDistance, fy = y * step + kLargeDistance;
                SkPoint roundedPos = SkPoint{fx, fy} + roundingSpec.halfAxisSampleFreq;
                SkPackedGlyphID packedID{3, roundedPos, mask};
                uint32_t subX = freqRound(x);
                uint32_t subY = freqRound(y);
                SkPackedGlyphID correctID(3, subX, subY);
                REPORTER_ASSERT(reporter, packedID == correctID);
            }
        }
    }
}

DEF_TEST(SkSourceGlyphBufferBasic, reporter) {
    SkSourceGlyphBuffer rejects;
    // Positions are picked to avoid precision problems.
    const SkPoint positions[] = {{10.25,10.25}, {20.5,10.25}, {30.75,10.25}, {40,10.25}};
    const SkGlyphID glyphIDs[] = {1, 2, 3, 4};
    auto source = SkMakeZip(glyphIDs, positions);

    rejects.setSource(source);
    for (auto [i, glyphID, pos] : SkMakeEnumerate(rejects.source())) {
        REPORTER_ASSERT(reporter, glyphID == std::get<0>(source[i]));
        REPORTER_ASSERT(reporter, pos == std::get<1>(source[i]));
    }
    // Reject a couple of glyphs.
    rejects.reject(1);
    rejects.reject(2, 100);
    rejects.flipRejectsToSource();
    REPORTER_ASSERT(reporter, std::get<1>(rejects.maxDimensionHint()) == 100);
    for (auto [i, glyphID, pos] : SkMakeEnumerate(rejects.source())) {
        // This will index 1 and 2 from the original source.
        size_t j = i + 1;
        REPORTER_ASSERT(reporter, glyphID == std::get<0>(source[j]));
        REPORTER_ASSERT(reporter, pos == std::get<1>(source[j]));
    }

    // Reject an additional glyph
    rejects.reject(0, 10);
    rejects.flipRejectsToSource();
    REPORTER_ASSERT(reporter, std::get<1>(rejects.maxDimensionHint()) == 10);
    for (auto [i, glyphID, pos] : SkMakeEnumerate(rejects.source())) {
        // This will index 1 from the original source.
        size_t j = i + 1;
        REPORTER_ASSERT(reporter, glyphID == std::get<0>(source[j]));
        REPORTER_ASSERT(reporter, pos == std::get<1>(source[j]));
    }

    // Start all over
    rejects.setSource(source);
    for (auto [i, glyphID, pos] : SkMakeEnumerate(rejects.source())) {
        REPORTER_ASSERT(reporter, glyphID == std::get<0>(source[i]));
        REPORTER_ASSERT(reporter, pos == std::get<1>(source[i]));
    }

    // Check that everything is working after calling setSource.
    rejects.reject(1);
    rejects.reject(2, 100);
    rejects.flipRejectsToSource();
    REPORTER_ASSERT(reporter, std::get<1>(rejects.maxDimensionHint()) == 100);
    for (auto [i, glyphID, pos] : SkMakeEnumerate(rejects.source())) {
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
        drawable.startSource(source);
        for (auto [i, packedID, pos] : SkMakeEnumerate(drawable.input())) {
            REPORTER_ASSERT(reporter, packedID.packedID().glyphID() == glyphIDs[i]);
            REPORTER_ASSERT(reporter, pos == positions[i]);
        }
    }

    {
        SkDrawableGlyphBuffer drawable;
        drawable.ensureSize(100);
        SkMatrix matrix = SkMatrix::Scale(0.5, 0.5);
        SkGlyphPositionRoundingSpec rounding{true, kX_SkAxisAlignment};
        drawable.startBitmapDevice(source, {100, 100}, matrix, rounding);
        for (auto [i, packedID, pos] : SkMakeEnumerate(drawable.input())) {
            REPORTER_ASSERT(reporter, glyphIDs[i] == packedID.packedID().glyphID());
            REPORTER_ASSERT(reporter,
                    pos.x() == positions[i].x() * 0.5 + 50 + SkPackedGlyphID::kSubpixelRound);
            REPORTER_ASSERT(reporter, pos.y() == positions[i].y() * 0.5 + 50 + 0.5);
        }
    }

    {
        SkDrawableGlyphBuffer drawable;
        drawable.ensureSize(100);
        drawable.startSource(source);
        for (auto [i, packedID, pos] : SkMakeEnumerate(drawable.input())) {
            drawable.push_back(&glyphs[i], i);
        }
        for (auto [i, glyph, pos] : SkMakeEnumerate(drawable.drawable())) {
            REPORTER_ASSERT(reporter, glyph.glyph() == &glyphs[i]);
        }
    }
}
