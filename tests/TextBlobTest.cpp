/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPaint.h"
#include "SkPoint.h"
#include "SkTextBlobRunIterator.h"
#include "SkTypeface.h"

#include "Test.h"

class TextBlobTester {
public:
    // This unit test feeds an SkTextBlobBuilder various runs then checks to see if
    // the result contains the provided data and merges runs when appropriate.
    static void TestBuilder(skiatest::Reporter* reporter) {
        SkTextBlobBuilder builder;

        // empty run set
        RunBuilderTest(reporter, builder, nullptr, 0, nullptr, 0);

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
            sk_sp<SkTextBlob> blob(builder.make());
            REPORTER_ASSERT(reporter, !blob);
        }

        {
            SkRect r1 = SkRect::MakeXYWH(10, 10, 20, 20);
            builder.allocRun(font, 16, 0, 0, &r1);
            sk_sp<SkTextBlob> blob(builder.make());
            REPORTER_ASSERT(reporter, blob->bounds() == r1);
        }

        {
            SkRect r1 = SkRect::MakeXYWH(10, 10, 20, 20);
            builder.allocRunPosH(font, 16, 0, &r1);
            sk_sp<SkTextBlob> blob(builder.make());
            REPORTER_ASSERT(reporter, blob->bounds() == r1);
        }

        {
            SkRect r1 = SkRect::MakeXYWH(10, 10, 20, 20);
            builder.allocRunPos(font, 16, &r1);
            sk_sp<SkTextBlob> blob(builder.make());
            REPORTER_ASSERT(reporter, blob->bounds() == r1);
        }

        {
            SkRect r1 = SkRect::MakeXYWH(10, 10, 20, 20);
            SkRect r2 = SkRect::MakeXYWH(15, 20, 50, 50);
            SkRect r3 = SkRect::MakeXYWH(0, 5, 10, 5);

            builder.allocRun(font, 16, 0, 0, &r1);
            builder.allocRunPosH(font, 16, 0, &r2);
            builder.allocRunPos(font, 16, &r3);

            sk_sp<SkTextBlob> blob(builder.make());
            REPORTER_ASSERT(reporter, blob->bounds() == SkRect::MakeXYWH(0, 5, 65, 65));
        }

        {
            sk_sp<SkTextBlob> blob(builder.make());
            REPORTER_ASSERT(reporter, !blob);
        }

        // Implicit bounds

        {
            // Exercise the empty bounds path, and ensure that RunRecord-aligned pos buffers
            // don't trigger asserts (http://crbug.com/542643).
            SkPaint p;
            p.setTextSize(0);
            p.setTextEncoding(SkPaint::kUTF8_TextEncoding);

            const char* txt = "BOOO";
            const size_t txtLen = strlen(txt);
            const int glyphCount = p.textToGlyphs(txt, txtLen, nullptr);

            p.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
            const SkTextBlobBuilder::RunBuffer& buffer = builder.allocRunPos(p, glyphCount);

            p.setTextEncoding(SkPaint::kUTF8_TextEncoding);
            p.textToGlyphs(txt, txtLen, buffer.glyphs);

            memset(buffer.pos, 0, sizeof(SkScalar) * glyphCount * 2);
            sk_sp<SkTextBlob> blob(builder.make());
            REPORTER_ASSERT(reporter, blob->bounds().isEmpty());
        }
    }

    // Verify that text-related properties are captured in run paints.
    static void TestPaintProps(skiatest::Reporter* reporter) {
        SkPaint font;
        font.setTextEncoding(SkPaint::kGlyphID_TextEncoding);

        // Kitchen sink font.
        font.setTextSize(42);
        font.setTextScaleX(4.2f);
        font.setTypeface(SkTypeface::MakeDefault());
        font.setTextSkewX(0.42f);
        font.setTextAlign(SkPaint::kCenter_Align);
        font.setHinting(SkPaint::kFull_Hinting);
        font.setAntiAlias(true);
        font.setFakeBoldText(true);
        font.setLinearText(true);
        font.setSubpixelText(true);
        font.setDevKernText(true);
        font.setLCDRenderText(true);
        font.setEmbeddedBitmapText(true);
        font.setAutohinted(true);
        font.setVerticalText(true);
        font.setFlags(font.getFlags() | SkPaint::kGenA8FromLCD_Flag);

        // Ensure we didn't pick default values by mistake.
        SkPaint defaultPaint;
        REPORTER_ASSERT(reporter, defaultPaint.getTextSize() != font.getTextSize());
        REPORTER_ASSERT(reporter, defaultPaint.getTextScaleX() != font.getTextScaleX());
        REPORTER_ASSERT(reporter, defaultPaint.getTypeface() != font.getTypeface());
        REPORTER_ASSERT(reporter, defaultPaint.getTextSkewX() != font.getTextSkewX());
        REPORTER_ASSERT(reporter, defaultPaint.getTextAlign() != font.getTextAlign());
        REPORTER_ASSERT(reporter, defaultPaint.getHinting() != font.getHinting());
        REPORTER_ASSERT(reporter, defaultPaint.isAntiAlias() != font.isAntiAlias());
        REPORTER_ASSERT(reporter, defaultPaint.isFakeBoldText() != font.isFakeBoldText());
        REPORTER_ASSERT(reporter, defaultPaint.isLinearText() != font.isLinearText());
        REPORTER_ASSERT(reporter, defaultPaint.isSubpixelText() != font.isSubpixelText());
        REPORTER_ASSERT(reporter, defaultPaint.isDevKernText() != font.isDevKernText());
        REPORTER_ASSERT(reporter, defaultPaint.isLCDRenderText() != font.isLCDRenderText());
        REPORTER_ASSERT(reporter, defaultPaint.isEmbeddedBitmapText() != font.isEmbeddedBitmapText());
        REPORTER_ASSERT(reporter, defaultPaint.isAutohinted() != font.isAutohinted());
        REPORTER_ASSERT(reporter, defaultPaint.isVerticalText() != font.isVerticalText());
        REPORTER_ASSERT(reporter, (defaultPaint.getFlags() & SkPaint::kGenA8FromLCD_Flag) !=
                                  (font.getFlags() & SkPaint::kGenA8FromLCD_Flag));

        SkTextBlobBuilder builder;
        AddRun(font, 1, SkTextBlob::kDefault_Positioning, SkPoint::Make(0, 0), builder);
        AddRun(font, 1, SkTextBlob::kHorizontal_Positioning, SkPoint::Make(0, 0), builder);
        AddRun(font, 1, SkTextBlob::kFull_Positioning, SkPoint::Make(0, 0), builder);
        sk_sp<SkTextBlob> blob(builder.make());

        SkTextBlobRunIterator it(blob.get());
        while (!it.done()) {
            SkPaint paint;
            it.applyFontToPaint(&paint);

            REPORTER_ASSERT(reporter, paint.getTextSize() == font.getTextSize());
            REPORTER_ASSERT(reporter, paint.getTextScaleX() == font.getTextScaleX());
            REPORTER_ASSERT(reporter, paint.getTypeface() == font.getTypeface());
            REPORTER_ASSERT(reporter, paint.getTextSkewX() == font.getTextSkewX());
            REPORTER_ASSERT(reporter, paint.getTextAlign() == font.getTextAlign());
            REPORTER_ASSERT(reporter, paint.getHinting() == font.getHinting());
            REPORTER_ASSERT(reporter, paint.isAntiAlias() == font.isAntiAlias());
            REPORTER_ASSERT(reporter, paint.isFakeBoldText() == font.isFakeBoldText());
            REPORTER_ASSERT(reporter, paint.isLinearText() == font.isLinearText());
            REPORTER_ASSERT(reporter, paint.isSubpixelText() == font.isSubpixelText());
            REPORTER_ASSERT(reporter, paint.isDevKernText() == font.isDevKernText());
            REPORTER_ASSERT(reporter, paint.isLCDRenderText() == font.isLCDRenderText());
            REPORTER_ASSERT(reporter, paint.isEmbeddedBitmapText() == font.isEmbeddedBitmapText());
            REPORTER_ASSERT(reporter, paint.isAutohinted() == font.isAutohinted());
            REPORTER_ASSERT(reporter, paint.isVerticalText() == font.isVerticalText());
            REPORTER_ASSERT(reporter, (paint.getFlags() & SkPaint::kGenA8FromLCD_Flag) ==
                                      (font.getFlags() & SkPaint::kGenA8FromLCD_Flag));

            it.next();
        }

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

        sk_sp<SkTextBlob> blob(builder.make());
        REPORTER_ASSERT(reporter, (inCount > 0) == SkToBool(blob));
        if (!blob) {
            return;
        }

        SkTextBlobRunIterator it(blob.get());
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
                       const SkRect* bounds = nullptr) {
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

DEF_TEST(TextBlob_paint, reporter) {
    TextBlobTester::TestPaintProps(reporter);
}

DEF_TEST(TextBlob_extended, reporter) {
    SkTextBlobBuilder textBlobBuilder;
    SkPaint paint;
    const char text1[] = "Foo";
    const char text2[] = "Bar";

    int glyphCount = paint.textToGlyphs(text1, strlen(text1), nullptr);
    SkAutoTMalloc<uint16_t> glyphs(glyphCount);
    (void)paint.textToGlyphs(text1, strlen(text1), glyphs.get());
    paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);

    auto run = textBlobBuilder.allocRunText(
            paint, glyphCount, 0, 0, SkToInt(strlen(text2)), SkString(), nullptr);
    memcpy(run.glyphs, glyphs.get(), sizeof(uint16_t) * glyphCount);
    memcpy(run.utf8text, text2, strlen(text2));
    for (int i = 0; i < glyphCount; ++i) {
        run.clusters[i] = SkTMin(SkToU32(i), SkToU32(strlen(text2)));
    }
    sk_sp<SkTextBlob> blob(textBlobBuilder.make());
    REPORTER_ASSERT(reporter, blob);

    for (SkTextBlobRunIterator it(blob.get()); !it.done(); it.next()) {
        REPORTER_ASSERT(reporter, it.glyphCount() == (uint32_t)glyphCount);
        for (uint32_t i = 0; i < it.glyphCount(); ++i) {
            REPORTER_ASSERT(reporter, it.glyphs()[i] == glyphs[i]);
        }
        REPORTER_ASSERT(reporter, SkTextBlob::kDefault_Positioning == it.positioning());
        REPORTER_ASSERT(reporter, (SkPoint{0.0f, 0.0f}) == it.offset());
        REPORTER_ASSERT(reporter, it.textSize() > 0);
        REPORTER_ASSERT(reporter, it.clusters());
        for (uint32_t i = 0; i < it.glyphCount(); ++i) {
            REPORTER_ASSERT(reporter, i == it.clusters()[i]);
        }
        REPORTER_ASSERT(reporter, 0 == strncmp(text2, it.text(), it.textSize()));
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "SkCanvas.h"
#include "SkSurface.h"
#include "SkTDArray.h"

static void add_run(SkTextBlobBuilder* builder, const char text[], SkScalar x, SkScalar y,
                    sk_sp<SkTypeface> tf) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setSubpixelText(true);
    paint.setTextSize(16);
    paint.setTypeface(tf);

    int glyphCount = paint.textToGlyphs(text, strlen(text), nullptr);

    paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
    SkTextBlobBuilder::RunBuffer buffer = builder->allocRun(paint, glyphCount, x, y);

    paint.setTextEncoding(SkPaint::kUTF8_TextEncoding);
    (void)paint.textToGlyphs(text, strlen(text), buffer.glyphs);
}

static sk_sp<SkImage> render(const SkTextBlob* blob) {
    auto surf = SkSurface::MakeRasterN32Premul(SkScalarRoundToInt(blob->bounds().width()),
                                               SkScalarRoundToInt(blob->bounds().height()));
    if (!surf) {
        return nullptr; // bounds are empty?
    }
    surf->getCanvas()->clear(SK_ColorWHITE);
    surf->getCanvas()->drawTextBlob(blob, -blob->bounds().left(), -blob->bounds().top(), SkPaint());
    return surf->makeImageSnapshot();
}

/*
 *  Build a blob with more than one typeface.
 *  Draw it into an offscreen,
 *  then serialize and deserialize,
 *  Then draw the new instance and assert it draws the same as the original.
 */
DEF_TEST(TextBlob_serialize, reporter) {
    SkTextBlobBuilder builder;

    sk_sp<SkTypeface> tf0;
    sk_sp<SkTypeface> tf1 = SkTypeface::MakeFromName("Times", SkFontStyle());

    add_run(&builder, "Hello", 10, 20, tf0);
    add_run(&builder, "World", 10, 40, tf1);
    sk_sp<SkTextBlob> blob0 = builder.make();

    SkTDArray<SkTypeface*> array;
    sk_sp<SkData> data = blob0->serialize([&array](SkTypeface* tf) {
        if (array.find(tf) < 0) {
            *array.append() = tf;
        }
    });
    REPORTER_ASSERT(reporter, array.count() > 0);

    sk_sp<SkTextBlob> blob1 = SkTextBlob::Deserialize(data->data(), data->size(),
                                                      [&array, reporter](uint32_t uniqueID) {
        for (int i = 0; i < array.count(); ++i) {
            if (array[i]->uniqueID() == uniqueID) {
                return sk_ref_sp(array[i]);
            }
        }
        REPORTER_ASSERT(reporter, false);
        return sk_sp<SkTypeface>(nullptr);
    });

    sk_sp<SkImage> img0 = render(blob0.get());
    sk_sp<SkImage> img1 = render(blob1.get());
    if (img0 && img1) {
        REPORTER_ASSERT(reporter, img0->width() == img1->width());
        REPORTER_ASSERT(reporter, img0->height() == img1->height());

        sk_sp<SkData> enc0 = img0->encodeToData();
        sk_sp<SkData> enc1 = img1->encodeToData();
        REPORTER_ASSERT(reporter, enc0->equals(enc1.get()));
        if (false) {    // in case you want to actually see the images...
            SkFILEWStream("textblob_serialize_img0.png").write(enc0->data(), enc0->size());
            SkFILEWStream("textblob_serialize_img1.png").write(enc1->data(), enc1->size());
        }
    }
}
