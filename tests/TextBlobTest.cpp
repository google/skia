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
    // This unit test feeds an SkTextBlobBuilder various runs then checks to see if
    // the result contains the provided data and merges runs when appropriate.
    static void TestBuilder(skiatest::Reporter* reporter) {
        SkTextBlobBuilder builder;

        // empty run set
        RunBuilderTest(reporter, builder, NULL, 0, NULL, 0);

        RunDef set1[] = {
            { 128, SkTextBlob::kDefault_Positioning, 100, 100 },
        };
        RunBuilderTest(reporter, builder, set1, SK_ARRAY_COUNT(set1), set1, SK_ARRAY_COUNT(set1));

        RunDef set2[] = {
            { 128, SkTextBlob::kHorizontal_Positioning, 100, 100 },
        };
        RunBuilderTest(reporter, builder, set2, SK_ARRAY_COUNT(set2), set2, SK_ARRAY_COUNT(set2));

        RunDef set3[] = {
            { 128, SkTextBlob::kFull_Positioning, 100, 100 },
        };
        RunBuilderTest(reporter, builder, set3, SK_ARRAY_COUNT(set3), set3, SK_ARRAY_COUNT(set3));

        RunDef set4[] = {
            { 128, SkTextBlob::kDefault_Positioning, 100, 150 },
            { 128, SkTextBlob::kDefault_Positioning, 100, 150 },
            { 128, SkTextBlob::kDefault_Positioning, 100, 150 },
        };
        RunBuilderTest(reporter, builder, set4, SK_ARRAY_COUNT(set4), set4, SK_ARRAY_COUNT(set4));

        RunDef set5[] = {
            { 128, SkTextBlob::kHorizontal_Positioning, 100, 150 },
            { 128, SkTextBlob::kHorizontal_Positioning, 200, 150 },
            { 128, SkTextBlob::kHorizontal_Positioning, 300, 250 },
        };
        RunDef mergedSet5[] = {
            { 256, SkTextBlob::kHorizontal_Positioning, 0, 150 },
            { 128, SkTextBlob::kHorizontal_Positioning, 0, 250 },
        };
        RunBuilderTest(reporter, builder, set5, SK_ARRAY_COUNT(set5), mergedSet5,
                       SK_ARRAY_COUNT(mergedSet5));

        RunDef set6[] = {
            { 128, SkTextBlob::kFull_Positioning, 100, 100 },
            { 128, SkTextBlob::kFull_Positioning, 200, 200 },
            { 128, SkTextBlob::kFull_Positioning, 300, 300 },
        };
        RunDef mergedSet6[] = {
            { 384, SkTextBlob::kFull_Positioning, 0, 0 },
        };
        RunBuilderTest(reporter, builder, set6, SK_ARRAY_COUNT(set6), mergedSet6,
                       SK_ARRAY_COUNT(mergedSet6));

        RunDef set7[] = {
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
        RunDef mergedSet7[] = {
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
        RunBuilderTest(reporter, builder, set7, SK_ARRAY_COUNT(set7), mergedSet7,
                       SK_ARRAY_COUNT(mergedSet7));
    }

    // This unit test verifies blob bounds computation.
    static void TestBounds(skiatest::Reporter* reporter) {
        SkTextBlobBuilder builder;
        SkPaint font;
        font.setTextEncoding(SkPaint::kGlyphID_TextEncoding);

        // Explicit bounds.
        {
            SkAutoTUnref<const SkTextBlob> blob(builder.build());
            REPORTER_ASSERT(reporter, blob->bounds().isEmpty());
        }

        {
            SkRect r1 = SkRect::MakeXYWH(10, 10, 20, 20);
            builder.allocRun(font, 16, 0, 0, &r1);
            SkAutoTUnref<const SkTextBlob> blob(builder.build());
            REPORTER_ASSERT(reporter, blob->bounds() == r1);
        }

        {
            SkRect r1 = SkRect::MakeXYWH(10, 10, 20, 20);
            builder.allocRunPosH(font, 16, 0, &r1);
            SkAutoTUnref<const SkTextBlob> blob(builder.build());
            REPORTER_ASSERT(reporter, blob->bounds() == r1);
        }

        {
            SkRect r1 = SkRect::MakeXYWH(10, 10, 20, 20);
            builder.allocRunPos(font, 16, &r1);
            SkAutoTUnref<const SkTextBlob> blob(builder.build());
            REPORTER_ASSERT(reporter, blob->bounds() == r1);
        }

        {
            SkRect r1 = SkRect::MakeXYWH(10, 10, 20, 20);
            SkRect r2 = SkRect::MakeXYWH(15, 20, 50, 50);
            SkRect r3 = SkRect::MakeXYWH(0, 5, 10, 5);

            builder.allocRun(font, 16, 0, 0, &r1);
            builder.allocRunPosH(font, 16, 0, &r2);
            builder.allocRunPos(font, 16, &r3);

            SkAutoTUnref<const SkTextBlob> blob(builder.build());
            REPORTER_ASSERT(reporter, blob->bounds() == SkRect::MakeXYWH(0, 5, 65, 65));
        }

        {
            // Verify empty blob bounds after building some non-empty blobs.
            SkAutoTUnref<const SkTextBlob> blob(builder.build());
            REPORTER_ASSERT(reporter, blob->bounds().isEmpty());
        }

        // Implicit bounds
        // FIXME: not supported yet.
    }

private:
    struct RunDef {
        unsigned                     count;
        SkTextBlob::GlyphPositioning pos;
        SkScalar                     x, y;
    };

    static void RunBuilderTest(skiatest::Reporter* reporter, SkTextBlobBuilder& builder,
                               const RunDef in[], unsigned inCount,
                               const RunDef out[], unsigned outCount) {
        SkPaint font;
        font.setTextEncoding(SkPaint::kGlyphID_TextEncoding);

        unsigned glyphCount = 0;
        unsigned posCount = 0;

        for (unsigned i = 0; i < inCount; ++i) {
            AddRun(font, in[i].count, in[i].pos, SkPoint::Make(in[i].x, in[i].y), builder);
            glyphCount += in[i].count;
            posCount += in[i].count * in[i].pos;
        }

        SkAutoTUnref<const SkTextBlob> blob(builder.build());

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

    static void AddRun(const SkPaint& font, int count, SkTextBlob::GlyphPositioning pos,
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
    TextBlobTester::TestBuilder(reporter);
    TextBlobTester::TestBounds(reporter);
}
