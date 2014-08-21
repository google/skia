/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPaint.h"
#include "SkPoint.h"
#include "SkTextBlob.h"

#include "Test.h"


class TextBlobTester {
public:
    static void test_builder(skiatest::Reporter* reporter) {
        SkPaint font;
        font.setTextEncoding(SkPaint::kGlyphID_TextEncoding);

        SkTextBlobBuilder builder;

        // empty run set
        runBuilderTest(reporter, font, builder, NULL, 0, NULL, 0);

        RunDef SET1[] = {
            { 128, SkTextBlob::kDefault_Positioning, 100, 100 },
        };
        runBuilderTest(reporter, font, builder, SET1, SK_ARRAY_COUNT(SET1), SET1,
                       SK_ARRAY_COUNT(SET1));

        RunDef SET2[] = {
            { 128, SkTextBlob::kHorizontal_Positioning, 100, 100 },
        };
        runBuilderTest(reporter, font, builder, SET2, SK_ARRAY_COUNT(SET2), SET2,
                       SK_ARRAY_COUNT(SET2));

        RunDef SET3[] = {
            { 128, SkTextBlob::kFull_Positioning, 100, 100 },
        };
        runBuilderTest(reporter, font, builder, SET3, SK_ARRAY_COUNT(SET3), SET3,
                       SK_ARRAY_COUNT(SET3));

        RunDef SET4[] = {
            { 128, SkTextBlob::kDefault_Positioning, 100, 150 },
            { 128, SkTextBlob::kDefault_Positioning, 100, 150 },
            { 128, SkTextBlob::kDefault_Positioning, 100, 150 },
        };
        runBuilderTest(reporter, font, builder, SET4, SK_ARRAY_COUNT(SET4), SET4,
                       SK_ARRAY_COUNT(SET4));

        RunDef SET5[] = {
            { 128, SkTextBlob::kHorizontal_Positioning, 100, 150 },
            { 128, SkTextBlob::kHorizontal_Positioning, 200, 150 },
            { 128, SkTextBlob::kHorizontal_Positioning, 300, 250 },
        };
        RunDef SET5_MERGED[] = {
            { 256, SkTextBlob::kHorizontal_Positioning, 0, 150 },
            { 128, SkTextBlob::kHorizontal_Positioning, 0, 250 },
        };
        runBuilderTest(reporter, font, builder, SET5, SK_ARRAY_COUNT(SET5), SET5_MERGED,
                       SK_ARRAY_COUNT(SET5_MERGED));

        RunDef SET6[] = {
            { 128, SkTextBlob::kFull_Positioning, 100, 100 },
            { 128, SkTextBlob::kFull_Positioning, 200, 200 },
            { 128, SkTextBlob::kFull_Positioning, 300, 300 },
        };
        RunDef SET6_MERGED[] = {
            { 384, SkTextBlob::kFull_Positioning, 0, 0 },
        };
        runBuilderTest(reporter, font, builder, SET6, SK_ARRAY_COUNT(SET6), SET6_MERGED,
                       SK_ARRAY_COUNT(SET6_MERGED));

        RunDef SET7[] = {
            { 128, SkTextBlob::kDefault_Positioning, 100, 150 },
            { 128, SkTextBlob::kDefault_Positioning, 100, 150 },
            { 128, SkTextBlob::kHorizontal_Positioning, 100, 150 },
            { 128, SkTextBlob::kHorizontal_Positioning, 200, 150 },
            { 128, SkTextBlob::kFull_Positioning, 400, 350 },
            { 128, SkTextBlob::kFull_Positioning, 400, 350 },
            { 128, SkTextBlob::kDefault_Positioning, 100, 450 },
            { 128, SkTextBlob::kDefault_Positioning, 100, 450 },
            { 128, SkTextBlob::kHorizontal_Positioning, 100, 550 },
            { 128, SkTextBlob::kHorizontal_Positioning, 200, 650 },
            { 128, SkTextBlob::kFull_Positioning, 400, 750 },
            { 128, SkTextBlob::kFull_Positioning, 400, 850 },
        };
        RunDef SET7_MERGED[] = {
            { 128, SkTextBlob::kDefault_Positioning, 100, 150 },
            { 128, SkTextBlob::kDefault_Positioning, 100, 150 },
            { 256, SkTextBlob::kHorizontal_Positioning, 0, 150 },
            { 256, SkTextBlob::kFull_Positioning, 0, 0 },
            { 128, SkTextBlob::kDefault_Positioning, 100, 450 },
            { 128, SkTextBlob::kDefault_Positioning, 100, 450 },
            { 128, SkTextBlob::kHorizontal_Positioning, 0, 550 },
            { 128, SkTextBlob::kHorizontal_Positioning, 0, 650 },
            { 256, SkTextBlob::kFull_Positioning, 0, 0 },
        };
        runBuilderTest(reporter, font, builder, SET7, SK_ARRAY_COUNT(SET7), SET7_MERGED,
                       SK_ARRAY_COUNT(SET7_MERGED));
    }

private:
    struct RunDef {
        unsigned                     count;
        SkTextBlob::GlyphPositioning pos;
        SkScalar                     x, y;
    };

    static void runBuilderTest(skiatest::Reporter* reporter, const SkPaint& font,
                               SkTextBlobBuilder& builder,
                               const RunDef in[], unsigned inCount,
                               const RunDef out[], unsigned outCount) {
        unsigned glyphCount = 0;
        unsigned posCount = 0;

        for (unsigned i = 0; i < inCount; ++i) {
            addRun(font, in[i].count, in[i].pos, SkPoint::Make(in[i].x, in[i].y), builder);
            glyphCount += in[i].count;
            posCount += in[i].count * in[i].pos;
        }

        SkAutoTUnref<const SkTextBlob> blob(builder.build());
        REPORTER_ASSERT(reporter, (NULL != blob->fGlyphBuffer) == (glyphCount > 0));
        REPORTER_ASSERT(reporter, (NULL != blob->fPosBuffer) == (posCount > 0));
        REPORTER_ASSERT(reporter, (NULL != blob->fRuns.get()) == (inCount > 0));

        SkTextBlob::RunIterator it(blob);
        for (unsigned i = 0; i < outCount; ++i) {
            REPORTER_ASSERT(reporter, !it.done());
            REPORTER_ASSERT(reporter, out[i].pos == it.positioning());
            REPORTER_ASSERT(reporter, out[i].count == it.glyphCount());
            if (SkTextBlob::kDefault_Positioning == out[i].pos) {
                REPORTER_ASSERT(reporter, out[i].x == it.offset().x());
                REPORTER_ASSERT(reporter, out[i].y == it.offset().y());
            } else if (SkTextBlob::kHorizontal_Positioning == out[i].pos) {
                REPORTER_ASSERT(reporter, out[i].y == it.offset().y());
            }

            for (unsigned k = 0; k < it.glyphCount(); ++k) {
                REPORTER_ASSERT(reporter, k % 128 == it.glyphs()[k]);
                if (SkTextBlob::kHorizontal_Positioning == it.positioning()) {
                    REPORTER_ASSERT(reporter, SkIntToScalar(k % 128) == it.pos()[k]);
                } else if (SkTextBlob::kFull_Positioning == it.positioning()) {
                    REPORTER_ASSERT(reporter, SkIntToScalar(k % 128) == it.pos()[k * 2]);
                    REPORTER_ASSERT(reporter, -SkIntToScalar(k % 128) == it.pos()[k * 2 + 1]);
                }
            }

            it.next();
        }

        REPORTER_ASSERT(reporter, it.done());
    }

    static void addRun(const SkPaint& font, int count, SkTextBlob::GlyphPositioning pos,
                       const SkPoint& offset, SkTextBlobBuilder& builder,
                       const SkRect* bounds = NULL) {
        switch (pos) {
        case SkTextBlob::kDefault_Positioning: {
            const SkTextBlobBuilder::RunBuffer& rb = builder.allocRun(font, count, offset.x(),
                                                                      offset.y(), bounds);
            for (int i = 0; i < count; ++i) {
                rb.glyphs[i] = i;
            }
        } break;
        case SkTextBlob::kHorizontal_Positioning: {
            const SkTextBlobBuilder::RunBuffer& rb = builder.allocRunPosH(font, count, offset.y(),
                                                                          bounds);
            for (int i = 0; i < count; ++i) {
                rb.glyphs[i] = i;
                rb.pos[i] = SkIntToScalar(i);
            }
        } break;
        case SkTextBlob::kFull_Positioning: {
            const SkTextBlobBuilder::RunBuffer& rb = builder.allocRunPos(font, count, bounds);
            for (int i = 0; i < count; ++i) {
                rb.glyphs[i] = i;
                rb.pos[i * 2] = SkIntToScalar(i);
                rb.pos[i * 2 + 1] = -SkIntToScalar(i);
            }
        } break;
        default:
            SkFAIL("unhandled positioning value");
        }
    }
};

DEF_TEST(TextBlob_builder, reporter) {
    TextBlobTester::test_builder(reporter);
}
