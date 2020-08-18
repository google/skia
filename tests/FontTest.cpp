/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/utils/SkCustomTypeface.h"
#include "src/core/SkAutoMalloc.h"
#include "src/core/SkFontPriv.h"
#include "src/core/SkPtrRecorder.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "tests/Test.h"

static SkFont serialize_deserialize(const SkFont& font, skiatest::Reporter* reporter) {
    sk_sp<SkRefCntSet> typefaces = sk_make_sp<SkRefCntSet>();
    SkBinaryWriteBuffer wb;
    wb.setTypefaceRecorder(typefaces);

    SkFontPriv::Flatten(font, wb);
    size_t size = wb.bytesWritten();
    SkAutoMalloc storage(size);
    wb.writeToMemory(storage.get());

    int count = typefaces->count();
    SkASSERT((!font.getTypeface() && count == 0) ||
             ( font.getTypeface() && count == 1));
    SkDynamicMemoryWStream typefaceWStream;
    if (count) {
        SkTypeface* typeface;
        typefaces->copyToArray((SkRefCnt**)&typeface);
        SkASSERT(typeface == font.getTypeface());
        typeface->serialize(&typefaceWStream);
    }

    SkReadBuffer rb(storage.get(), size);
    sk_sp<SkTypeface> cloneTypeface;
    if (count) {
        std::unique_ptr<SkStream> typefaceStream = typefaceWStream.detachAsStream();
        cloneTypeface = SkTypeface::MakeDeserialize(typefaceStream.get());
        SkASSERT(cloneTypeface);
        rb.setTypefaceArray(&cloneTypeface, 1);
    }
    SkFont clone;
    REPORTER_ASSERT(reporter, SkFontPriv::Unflatten(&clone, rb));
    return clone;
}

enum {
    kForceAutoHinting      = 1 << 0,
    kEmbeddedBitmaps       = 1 << 1,
    kSubpixel              = 1 << 2,
    kLinearMetrics         = 1 << 3,
    kEmbolden              = 1 << 4,
    kBaselineSnap          = 1 << 5,

    kAllBits = 0x3F,
};

static void apply_flags(SkFont* font, unsigned flags) {
    font->setForceAutoHinting(SkToBool(flags & kForceAutoHinting));
    font->setEmbeddedBitmaps( SkToBool(flags & kEmbeddedBitmaps));
    font->setSubpixel(        SkToBool(flags & kSubpixel));
    font->setLinearMetrics(   SkToBool(flags & kLinearMetrics));
    font->setEmbolden(        SkToBool(flags & kEmbolden));
    font->setBaselineSnap(    SkToBool(flags & kBaselineSnap));
}
static bool test_flags(const SkFont& font, unsigned flags) {
    if (font.isForceAutoHinting() != SkToBool(flags & kForceAutoHinting)) { return false; }
    if (font.isEmbeddedBitmaps()  != SkToBool(flags & kEmbeddedBitmaps) ) { return false; }
    if (font.isSubpixel()         != SkToBool(flags & kSubpixel)        ) { return false; }
    if (font.isLinearMetrics()    != SkToBool(flags & kLinearMetrics)   ) { return false; }
    if (font.isEmbolden()         != SkToBool(flags & kEmbolden)        ) { return false; }
    if (font.isBaselineSnap()     != SkToBool(flags & kBaselineSnap)    ) { return false; }
    return true;
}

static sk_sp<SkTypeface> make_custom_tf() {
    SkCustomTypefaceBuilder builder;
    SkFont font;
    const float upem = 200;

    {
        SkFontMetrics metrics;
        metrics.fFlags = 0;
        metrics.fTop = -200;
        metrics.fAscent = -150;
        metrics.fDescent = 50;
        metrics.fBottom = -75;
        metrics.fLeading = 10;
        metrics.fAvgCharWidth = 150;
        metrics.fMaxCharWidth = 300;
        metrics.fXMin = -20;
        metrics.fXMax = 290;
        metrics.fXHeight = -100;
        metrics.fCapHeight = 0;
        metrics.fUnderlineThickness = 5;
        metrics.fUnderlinePosition = 2;
        metrics.fStrikeoutThickness = 5;
        metrics.fStrikeoutPosition = -50;
        builder.setMetrics(metrics, 1.0f/upem);
    }
    builder.setFontStyle(SkFontStyle(367, 3, SkFontStyle::kOblique_Slant));

    const SkMatrix scale = SkMatrix::Scale(1.0f/upem, 1.0f/upem);
    for (SkGlyphID index = 0; index <= 67; ++index) {
        SkScalar width;
        width = 100;
        SkPath path;
        path.addCircle(50, -50, 75);

        builder.setGlyph(index, width/upem, path.makeTransform(scale));
    }

    return builder.detach();
}

DEF_TEST(Font_flatten, reporter) {
    const float sizes[] = {0, 0.001f, 1, 10, 10.001f, 100000.01f};
    const float scales[] = {-5, 0, 1, 5};
    const float skews[] = {-5, 0, 5};
    const SkFont::Edging edges[] = {
        SkFont::Edging::kAlias, SkFont::Edging::kSubpixelAntiAlias
    };
    const SkFontHinting hints[] = {
        SkFontHinting::kNone, SkFontHinting::kFull
    };
    const unsigned int flags[] = {
        kForceAutoHinting, kEmbeddedBitmaps, kSubpixel, kLinearMetrics, kEmbolden, kBaselineSnap,
        kAllBits,
    };
    const sk_sp<SkTypeface> typefaces[] = {
        make_custom_tf(), nullptr
    };

    SkFont font;
    for (float size : sizes) {
        font.setSize(size);
        for (float scale : scales) {
            font.setScaleX(scale);
            for (float skew : skews) {
                font.setSkewX(skew);
                for (auto edge : edges) {
                    font.setEdging(edge);
                    for (auto hint : hints) {
                        font.setHinting(hint);
                        for (auto flag : flags) {
                            apply_flags(&font, flag);
                            for (const sk_sp<SkTypeface>& typeface : typefaces) {
                                font.setTypeface(typeface);
                                SkFont clone = serialize_deserialize(font, reporter);
                                // Cannot use SkFont::operator== since it compares SkTypeface*
                                //REPORTER_ASSERT(reporter, font == clone);
                                REPORTER_ASSERT(reporter, font.getSize() == clone.getSize());
                                REPORTER_ASSERT(reporter, font.getScaleX() == clone.getScaleX());
                                REPORTER_ASSERT(reporter, font.getSkewX() == clone.getSkewX());
                                REPORTER_ASSERT(reporter, test_flags(clone, flag));
                                REPORTER_ASSERT(reporter, font.getEdging() == clone.getEdging());
                                REPORTER_ASSERT(reporter, font.getHinting() == clone.getHinting());

                                SkFontMetrics fontMetrics, cloneMetrics;
                                font.getMetrics(&fontMetrics);
                                clone.getMetrics(&cloneMetrics);
                                REPORTER_ASSERT(reporter, fontMetrics == cloneMetrics);
                                SkTypeface* fontTypeface = font.getTypefaceOrDefault();
                                SkTypeface* cloneTypeface = clone.getTypefaceOrDefault();
                                REPORTER_ASSERT(reporter, fontTypeface->countGlyphs() ==
                                                          cloneTypeface->countGlyphs());
                                REPORTER_ASSERT(reporter, fontTypeface->fontStyle() ==
                                                          cloneTypeface->fontStyle());
                            }
                        }
                    }
                }
            }
        }
    }
}
