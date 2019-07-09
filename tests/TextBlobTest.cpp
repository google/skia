/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkSerialProcs.h"
#include "include/core/SkTypeface.h"
#include "include/private/SkTo.h"
#include "src/core/SkTextBlobPriv.h"

#include "tests/Test.h"
#include "tools/ToolUtils.h"

class TextBlobTester {
public:
    // This unit test feeds an SkTextBlobBuilder various runs then checks to see if
    // the result contains the provided data and merges runs when appropriate.
    static void TestBuilder(skiatest::Reporter* reporter) {
        SkTextBlobBuilder builder;

        // empty run set
        RunBuilderTest(reporter, builder, nullptr, 0, nullptr, 0);

        RunDef set1[] = {
            { 128, SkTextBlobRunIterator::kDefault_Positioning, 100, 100 },
        };
        RunBuilderTest(reporter, builder, set1, SK_ARRAY_COUNT(set1), set1, SK_ARRAY_COUNT(set1));

        RunDef set2[] = {
            { 128, SkTextBlobRunIterator::kHorizontal_Positioning, 100, 100 },
        };
        RunBuilderTest(reporter, builder, set2, SK_ARRAY_COUNT(set2), set2, SK_ARRAY_COUNT(set2));

        RunDef set3[] = {
            { 128, SkTextBlobRunIterator::kFull_Positioning, 100, 100 },
        };
        RunBuilderTest(reporter, builder, set3, SK_ARRAY_COUNT(set3), set3, SK_ARRAY_COUNT(set3));

        RunDef set4[] = {
            { 128, SkTextBlobRunIterator::kDefault_Positioning, 100, 150 },
            { 128, SkTextBlobRunIterator::kDefault_Positioning, 100, 150 },
            { 128, SkTextBlobRunIterator::kDefault_Positioning, 100, 150 },
        };
        RunBuilderTest(reporter, builder, set4, SK_ARRAY_COUNT(set4), set4, SK_ARRAY_COUNT(set4));

        RunDef set5[] = {
            { 128, SkTextBlobRunIterator::kHorizontal_Positioning, 100, 150 },
            { 128, SkTextBlobRunIterator::kHorizontal_Positioning, 200, 150 },
            { 128, SkTextBlobRunIterator::kHorizontal_Positioning, 300, 250 },
        };
        RunDef mergedSet5[] = {
            { 256, SkTextBlobRunIterator::kHorizontal_Positioning, 0, 150 },
            { 128, SkTextBlobRunIterator::kHorizontal_Positioning, 0, 250 },
        };
        RunBuilderTest(reporter, builder, set5, SK_ARRAY_COUNT(set5), mergedSet5,
                       SK_ARRAY_COUNT(mergedSet5));

        RunDef set6[] = {
            { 128, SkTextBlobRunIterator::kFull_Positioning, 100, 100 },
            { 128, SkTextBlobRunIterator::kFull_Positioning, 200, 200 },
            { 128, SkTextBlobRunIterator::kFull_Positioning, 300, 300 },
        };
        RunDef mergedSet6[] = {
            { 384, SkTextBlobRunIterator::kFull_Positioning, 0, 0 },
        };
        RunBuilderTest(reporter, builder, set6, SK_ARRAY_COUNT(set6), mergedSet6,
                       SK_ARRAY_COUNT(mergedSet6));

        RunDef set7[] = {
            { 128, SkTextBlobRunIterator::kDefault_Positioning, 100, 150 },
            { 128, SkTextBlobRunIterator::kDefault_Positioning, 100, 150 },
            { 128, SkTextBlobRunIterator::kHorizontal_Positioning, 100, 150 },
            { 128, SkTextBlobRunIterator::kHorizontal_Positioning, 200, 150 },
            { 128, SkTextBlobRunIterator::kFull_Positioning, 400, 350 },
            { 128, SkTextBlobRunIterator::kFull_Positioning, 400, 350 },
            { 128, SkTextBlobRunIterator::kDefault_Positioning, 100, 450 },
            { 128, SkTextBlobRunIterator::kDefault_Positioning, 100, 450 },
            { 128, SkTextBlobRunIterator::kHorizontal_Positioning, 100, 550 },
            { 128, SkTextBlobRunIterator::kHorizontal_Positioning, 200, 650 },
            { 128, SkTextBlobRunIterator::kFull_Positioning, 400, 750 },
            { 128, SkTextBlobRunIterator::kFull_Positioning, 400, 850 },
        };
        RunDef mergedSet7[] = {
            { 128, SkTextBlobRunIterator::kDefault_Positioning, 100, 150 },
            { 128, SkTextBlobRunIterator::kDefault_Positioning, 100, 150 },
            { 256, SkTextBlobRunIterator::kHorizontal_Positioning, 0, 150 },
            { 256, SkTextBlobRunIterator::kFull_Positioning, 0, 0 },
            { 128, SkTextBlobRunIterator::kDefault_Positioning, 100, 450 },
            { 128, SkTextBlobRunIterator::kDefault_Positioning, 100, 450 },
            { 128, SkTextBlobRunIterator::kHorizontal_Positioning, 0, 550 },
            { 128, SkTextBlobRunIterator::kHorizontal_Positioning, 0, 650 },
            { 256, SkTextBlobRunIterator::kFull_Positioning, 0, 0 },
        };
        RunBuilderTest(reporter, builder, set7, SK_ARRAY_COUNT(set7), mergedSet7,
                       SK_ARRAY_COUNT(mergedSet7));
    }

    // This unit test verifies blob bounds computation.
    static void TestBounds(skiatest::Reporter* reporter) {
        SkTextBlobBuilder builder;
        SkFont font;

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
            SkFont font;
            font.setSize(0);

            const char* txt = "BOOO";
            const size_t txtLen = strlen(txt);
            const int glyphCount = font.countText(txt, txtLen, SkTextEncoding::kUTF8);
            const SkTextBlobBuilder::RunBuffer& buffer = builder.allocRunPos(font, glyphCount);

            font.textToGlyphs(txt, txtLen, SkTextEncoding::kUTF8, buffer.glyphs, glyphCount);

            memset(buffer.pos, 0, sizeof(SkScalar) * glyphCount * 2);
            sk_sp<SkTextBlob> blob(builder.make());
            REPORTER_ASSERT(reporter, blob->bounds().isEmpty());
        }
    }

    // Verify that text-related properties are captured in run paints.
    static void TestPaintProps(skiatest::Reporter* reporter) {
        SkFont font;
        // Kitchen sink font.
        font.setSize(42);
        font.setScaleX(4.2f);
        font.setTypeface(ToolUtils::create_portable_typeface());
        font.setSkewX(0.42f);
        font.setHinting(SkFontHinting::kFull);
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
        font.setEmbolden(true);
        font.setLinearMetrics(true);
        font.setSubpixel(true);
        font.setEmbeddedBitmaps(true);
        font.setForceAutoHinting(true);

        // Ensure we didn't pick default values by mistake.
        SkFont defaultFont;
        REPORTER_ASSERT(reporter, defaultFont.getSize() != font.getSize());
        REPORTER_ASSERT(reporter, defaultFont.getScaleX() != font.getScaleX());
        REPORTER_ASSERT(reporter, defaultFont.getTypefaceOrDefault() != font.getTypefaceOrDefault());
        REPORTER_ASSERT(reporter, defaultFont.getSkewX() != font.getSkewX());
        REPORTER_ASSERT(reporter, defaultFont.getHinting() != font.getHinting());
        REPORTER_ASSERT(reporter, defaultFont.getEdging() != font.getEdging());
        REPORTER_ASSERT(reporter, defaultFont.isEmbolden() != font.isEmbolden());
        REPORTER_ASSERT(reporter, defaultFont.isLinearMetrics() != font.isLinearMetrics());
        REPORTER_ASSERT(reporter, defaultFont.isSubpixel() != font.isSubpixel());
        REPORTER_ASSERT(reporter,
                        defaultFont.isEmbeddedBitmaps() != font.isEmbeddedBitmaps());
        REPORTER_ASSERT(reporter, defaultFont.isForceAutoHinting() != font.isForceAutoHinting());

        SkTextBlobBuilder builder;
        AddRun(font, 1, SkTextBlobRunIterator::kDefault_Positioning, SkPoint::Make(0, 0), builder);
        AddRun(font, 1, SkTextBlobRunIterator::kHorizontal_Positioning, SkPoint::Make(0, 0),
               builder);
        AddRun(font, 1, SkTextBlobRunIterator::kFull_Positioning, SkPoint::Make(0, 0), builder);
        sk_sp<SkTextBlob> blob(builder.make());

        SkTextBlobRunIterator it(blob.get());
        while (!it.done()) {
            REPORTER_ASSERT(reporter, it.font() == font);
            it.next();
        }

    }

private:
    struct RunDef {
        unsigned                                count;
        SkTextBlobRunIterator::GlyphPositioning pos;
        SkScalar                                x, y;
    };

    static void RunBuilderTest(skiatest::Reporter* reporter, SkTextBlobBuilder& builder,
                               const RunDef in[], unsigned inCount,
                               const RunDef out[], unsigned outCount) {
        SkFont font;

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
            if (SkTextBlobRunIterator::kDefault_Positioning == out[i].pos) {
                REPORTER_ASSERT(reporter, out[i].x == it.offset().x());
                REPORTER_ASSERT(reporter, out[i].y == it.offset().y());
            } else if (SkTextBlobRunIterator::kHorizontal_Positioning == out[i].pos) {
                REPORTER_ASSERT(reporter, out[i].y == it.offset().y());
            }

            for (unsigned k = 0; k < it.glyphCount(); ++k) {
                REPORTER_ASSERT(reporter, k % 128 == it.glyphs()[k]);
                if (SkTextBlobRunIterator::kHorizontal_Positioning == it.positioning()) {
                    REPORTER_ASSERT(reporter, SkIntToScalar(k % 128) == it.pos()[k]);
                } else if (SkTextBlobRunIterator::kFull_Positioning == it.positioning()) {
                    REPORTER_ASSERT(reporter, SkIntToScalar(k % 128) == it.pos()[k * 2]);
                    REPORTER_ASSERT(reporter, -SkIntToScalar(k % 128) == it.pos()[k * 2 + 1]);
                }
            }

            it.next();
        }

        REPORTER_ASSERT(reporter, it.done());
    }

    static void AddRun(const SkFont& font, int count, SkTextBlobRunIterator::GlyphPositioning pos,
                       const SkPoint& offset, SkTextBlobBuilder& builder,
                       const SkRect* bounds = nullptr) {
        switch (pos) {
        case SkTextBlobRunIterator::kDefault_Positioning: {
            const SkTextBlobBuilder::RunBuffer& rb = builder.allocRun(font, count, offset.x(),
                                                                      offset.y(), bounds);
            for (int i = 0; i < count; ++i) {
                rb.glyphs[i] = i;
            }
        } break;
        case SkTextBlobRunIterator::kHorizontal_Positioning: {
            const SkTextBlobBuilder::RunBuffer& rb = builder.allocRunPosH(font, count, offset.y(),
                                                                          bounds);
            for (int i = 0; i < count; ++i) {
                rb.glyphs[i] = i;
                rb.pos[i] = SkIntToScalar(i);
            }
        } break;
        case SkTextBlobRunIterator::kFull_Positioning: {
            const SkTextBlobBuilder::RunBuffer& rb = builder.allocRunPos(font, count, bounds);
            for (int i = 0; i < count; ++i) {
                rb.glyphs[i] = i;
                rb.pos[i * 2] = SkIntToScalar(i);
                rb.pos[i * 2 + 1] = -SkIntToScalar(i);
            }
        } break;
        default:
            SK_ABORT("unhandled positioning value");
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
    SkFont font;
    const char text1[] = "Foo";
    const char text2[] = "Bar";

    int glyphCount = font.countText(text1, strlen(text1), SkTextEncoding::kUTF8);
    SkAutoTMalloc<uint16_t> glyphs(glyphCount);
    (void)font.textToGlyphs(text1, strlen(text1), SkTextEncoding::kUTF8, glyphs.get(), glyphCount);

    auto run = SkTextBlobBuilderPriv::AllocRunText(&textBlobBuilder,
            font, glyphCount, 0, 0, SkToInt(strlen(text2)), SkString(), nullptr);
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
        REPORTER_ASSERT(reporter, SkTextBlobRunIterator::kDefault_Positioning == it.positioning());
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
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/private/SkTArray.h"

static void add_run(SkTextBlobBuilder* builder, const char text[], SkScalar x, SkScalar y,
                    sk_sp<SkTypeface> tf) {
    SkFont font;
    font.setEdging(SkFont::Edging::kAntiAlias);
    font.setSubpixel(true);
    font.setSize(16);
    font.setTypeface(tf);

    int glyphCount = font.countText(text, strlen(text), SkTextEncoding::kUTF8);

    SkTextBlobBuilder::RunBuffer buffer = builder->allocRun(font, glyphCount, x, y);

    (void)font.textToGlyphs(text, strlen(text), SkTextEncoding::kUTF8, buffer.glyphs, glyphCount);
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

static sk_sp<SkData> SerializeTypeface(SkTypeface* tf, void* ctx) {
    auto array = (SkTArray<sk_sp<SkTypeface>>*)ctx;
    const size_t idx = array->size();
    array->emplace_back(sk_ref_sp(tf));
    // In this test, we are deserializing on the same machine, so we don't worry about endianness.
    return SkData::MakeWithCopy(&idx, sizeof(idx));
}

static sk_sp<SkTypeface> DeserializeTypeface(const void* data, size_t length, void* ctx) {
    auto array = (SkTArray<sk_sp<SkTypeface>>*)ctx;
    if (length != sizeof(size_t)) {
        SkASSERT(false);
        return nullptr;
    }
    size_t idx = *reinterpret_cast<const size_t*>(data);
    if (idx >= array->size()) {
        SkASSERT(false);
        return nullptr;
    }
    return (*array)[idx];
}

/*
 *  Build a blob with more than one typeface.
 *  Draw it into an offscreen,
 *  then serialize and deserialize,
 *  Then draw the new instance and assert it draws the same as the original.
 */
DEF_TEST(TextBlob_serialize, reporter) {
    sk_sp<SkTextBlob> blob0 = []() {
        sk_sp<SkTypeface> tf = SkTypeface::MakeFromName(nullptr, SkFontStyle::BoldItalic());

        SkTextBlobBuilder builder;
        add_run(&builder, "Hello", 10, 20, nullptr);    // don't flatten a typeface
        add_run(&builder, "World", 10, 40, tf);         // do flatten this typeface
        return builder.make();
    }();

    SkTArray<sk_sp<SkTypeface>> array;
    SkSerialProcs serializeProcs;
    serializeProcs.fTypefaceProc = &SerializeTypeface;
    serializeProcs.fTypefaceCtx = (void*) &array;
    sk_sp<SkData> data = blob0->serialize(serializeProcs);
    REPORTER_ASSERT(reporter, array.count() == 1);
    SkDeserialProcs deserializeProcs;
    deserializeProcs.fTypefaceProc = &DeserializeTypeface;
    deserializeProcs.fTypefaceCtx = (void*) &array;
    sk_sp<SkTextBlob> blob1 = SkTextBlob::Deserialize(data->data(), data->size(), deserializeProcs);

    sk_sp<SkImage> img0 = render(blob0.get());
    sk_sp<SkImage> img1 = render(blob1.get());
    if (img0 && img1) {
        REPORTER_ASSERT(reporter, ToolUtils::equal_pixels(img0.get(), img1.get()));
    }
}

DEF_TEST(TextBlob_MakeAsDrawText, reporter) {
    const char text[] = "Hello";
    auto blob = SkTextBlob::MakeFromString(text, SkFont(), SkTextEncoding::kUTF8);

    int runs = 0;
    for(SkTextBlobRunIterator it(blob.get()); !it.done(); it.next()) {
        REPORTER_ASSERT(reporter, it.glyphCount() == strlen(text));
        REPORTER_ASSERT(reporter, it.positioning() == SkTextBlobRunIterator::kFull_Positioning);
        runs += 1;
    }
    REPORTER_ASSERT(reporter, runs == 1);

}
