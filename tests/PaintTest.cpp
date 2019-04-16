/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAutoMalloc.h"
#include "SkBlurMask.h"
#include "SkFont.h"
#include "SkLayerDrawLooper.h"
#include "SkMaskFilter.h"
#include "SkPaintPriv.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkReadBuffer.h"
#include "SkTo.h"
#include "SkTypeface.h"
#include "SkUTF.h"
#include "SkWriteBuffer.h"
#include "Test.h"
#undef ASSERT

static size_t uni_to_utf8(const SkUnichar src[], void* dst, int count) {
    char* u8 = (char*)dst;
    for (int i = 0; i < count; ++i) {
        int n = SkToInt(SkUTF::ToUTF8(src[i], u8));
        u8 += n;
    }
    return u8 - (char*)dst;
}

static size_t uni_to_utf16(const SkUnichar src[], void* dst, int count) {
    uint16_t* u16 = (uint16_t*)dst;
    for (int i = 0; i < count; ++i) {
        int n = SkToInt(SkUTF::ToUTF16(src[i], u16));
        u16 += n;
    }
    return (char*)u16 - (char*)dst;
}

static size_t uni_to_utf32(const SkUnichar src[], void* dst, int count) {
    SkUnichar* u32 = (SkUnichar*)dst;
    if (src != u32) {
        memcpy(u32, src, count * sizeof(SkUnichar));
    }
    return count * sizeof(SkUnichar);
}

static int find_first_zero(const uint16_t glyphs[], int count) {
    for (int i = 0; i < count; ++i) {
        if (0 == glyphs[i]) {
            return i;
        }
    }
    return count;
}

DEF_TEST(Paint_cmap, reporter) {
    // need to implement charsToGlyphs on other backends (e.g. linux, win)
    // before we can run this tests everywhere
    return;

    static const int NGLYPHS = 64;

    SkUnichar src[NGLYPHS];
    SkUnichar dst[NGLYPHS]; // used for utf8, utf16, utf32 storage

    static const struct {
        size_t (*fSeedTextProc)(const SkUnichar[], void* dst, int count);
        SkTextEncoding   fEncoding;
    } gRec[] = {
        { uni_to_utf8,  kUTF8_SkTextEncoding },
        { uni_to_utf16, kUTF16_SkTextEncoding },
        { uni_to_utf32, kUTF32_SkTextEncoding },
    };

    SkRandom rand;
    SkFont font;
    font.setTypeface(SkTypeface::MakeDefault());
    SkTypeface* face = font.getTypefaceOrDefault();

    for (int i = 0; i < 1000; ++i) {
        // generate some random text
        for (int j = 0; j < NGLYPHS; ++j) {
            src[j] = ' ' + j;
        }
        // inject some random chars, to sometimes abort early
        src[rand.nextU() & 63] = rand.nextU() & 0xFFF;

        for (size_t k = 0; k < SK_ARRAY_COUNT(gRec); ++k) {
            size_t len = gRec[k].fSeedTextProc(src, dst, NGLYPHS);

            uint16_t    glyphs0[NGLYPHS], glyphs1[NGLYPHS];

            int nglyphs = font.textToGlyphs(dst, len, gRec[k].fEncoding, glyphs0, NGLYPHS);
            int first = face->charsToGlyphs(dst, (SkTypeface::Encoding)gRec[k].fEncoding,
                                            glyphs1, NGLYPHS);
            int index = find_first_zero(glyphs1, NGLYPHS);

            REPORTER_ASSERT(reporter, NGLYPHS == nglyphs);
            REPORTER_ASSERT(reporter, index == first);
            REPORTER_ASSERT(reporter, 0 == memcmp(glyphs0, glyphs1, NGLYPHS * sizeof(uint16_t)));
        }
    }
}

// temparary api for bicubic, just be sure we can set/clear it
DEF_TEST(Paint_filterQuality, reporter) {
    SkPaint p0, p1;

    REPORTER_ASSERT(reporter, kNone_SkFilterQuality == p0.getFilterQuality());

    static const SkFilterQuality gQualitys[] = {
        kNone_SkFilterQuality,
        kLow_SkFilterQuality,
        kMedium_SkFilterQuality,
        kHigh_SkFilterQuality
    };
    for (size_t i = 0; i < SK_ARRAY_COUNT(gQualitys); ++i) {
        p0.setFilterQuality(gQualitys[i]);
        REPORTER_ASSERT(reporter, gQualitys[i] == p0.getFilterQuality());
        p1 = p0;
        REPORTER_ASSERT(reporter, gQualitys[i] == p1.getFilterQuality());

        p0.reset();
        REPORTER_ASSERT(reporter, kNone_SkFilterQuality == p0.getFilterQuality());
    }
}

DEF_TEST(Paint_copy, reporter) {
    SkPaint paint;
    // set a few member variables
    paint.setStyle(SkPaint::kStrokeAndFill_Style);
    paint.setStrokeWidth(SkIntToScalar(2));
    // set a few pointers
    SkLayerDrawLooper::Builder looperBuilder;
    paint.setLooper(looperBuilder.detach());
    paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle,
                                               SkBlurMask::ConvertRadiusToSigma(1)));

    // copy the paint using the copy constructor and check they are the same
    SkPaint copiedPaint = paint;
    REPORTER_ASSERT(reporter, paint.getHash() == copiedPaint.getHash());
    REPORTER_ASSERT(reporter, paint == copiedPaint);

    // copy the paint using the equal operator and check they are the same
    copiedPaint = paint;
    REPORTER_ASSERT(reporter, paint == copiedPaint);

    // clean the paint and check they are back to their initial states
    SkPaint cleanPaint;
    paint.reset();
    copiedPaint.reset();
    REPORTER_ASSERT(reporter, cleanPaint == paint);
    REPORTER_ASSERT(reporter, cleanPaint == copiedPaint);
}

// found and fixed for webkit: mishandling when we hit recursion limit on
// mostly degenerate cubic flatness test
DEF_TEST(Paint_regression_cubic, reporter) {
    SkPath path, stroke;
    SkPaint paint;

    path.moveTo(460.2881309415525f,
                303.250847066498f);
    path.cubicTo(463.36378422175284f,
                 302.1169735073363f,
                 456.32239330810046f,
                 304.720354932878f,
                 453.15255460013304f,
                 305.788586869862f);

    SkRect fillR, strokeR;
    fillR = path.getBounds();

    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(SkIntToScalar(2));
    paint.getFillPath(path, &stroke);
    strokeR = stroke.getBounds();

    SkRect maxR = fillR;
    SkScalar miter = SkMaxScalar(SK_Scalar1, paint.getStrokeMiter());
    SkScalar inset = paint.getStrokeJoin() == SkPaint::kMiter_Join ?
                            paint.getStrokeWidth() * miter :
                            paint.getStrokeWidth();
    maxR.inset(-inset, -inset);

    // test that our stroke didn't explode
    REPORTER_ASSERT(reporter, maxR.contains(strokeR));
}

DEF_TEST(Paint_flattening, reporter) {
    const SkFilterQuality levels[] = {
        kNone_SkFilterQuality,
        kLow_SkFilterQuality,
        kMedium_SkFilterQuality,
        kHigh_SkFilterQuality,
    };
    const SkPaint::Cap caps[] = {
        SkPaint::kButt_Cap,
        SkPaint::kRound_Cap,
        SkPaint::kSquare_Cap,
    };
    const SkPaint::Join joins[] = {
        SkPaint::kMiter_Join,
        SkPaint::kRound_Join,
        SkPaint::kBevel_Join,
    };
    const SkPaint::Style styles[] = {
        SkPaint::kFill_Style,
        SkPaint::kStroke_Style,
        SkPaint::kStrokeAndFill_Style,
    };

#define FOR_SETUP(index, array, setter)                                 \
    for (size_t index = 0; index < SK_ARRAY_COUNT(array); ++index) {    \
        paint.setter(array[index]);

    SkPaint paint;
    paint.setAntiAlias(true);

    // we don't serialize hinting or encoding -- soon to be removed from paint

    FOR_SETUP(i, levels, setFilterQuality)
    FOR_SETUP(l, caps, setStrokeCap)
    FOR_SETUP(m, joins, setStrokeJoin)
    FOR_SETUP(p, styles, setStyle)

    SkBinaryWriteBuffer writer;
    SkPaintPriv::Flatten(paint, writer);

    SkAutoMalloc buf(writer.bytesWritten());
    writer.writeToMemory(buf.get());
    SkReadBuffer reader(buf.get(), writer.bytesWritten());

    SkPaint paint2;
    SkPaintPriv::Unflatten(&paint2, reader, nullptr);
    REPORTER_ASSERT(reporter, paint2 == paint);

    }}}}
#undef FOR_SETUP

}

// found and fixed for android: not initializing rect for string's of length 0
DEF_TEST(Paint_regression_measureText, reporter) {

    SkFont font;
    font.setSize(12.0f);

    SkRect r;
    r.setLTRB(SK_ScalarNaN, SK_ScalarNaN, SK_ScalarNaN, SK_ScalarNaN);

    // test that the rect was reset
    font.measureText("", 0, kUTF8_SkTextEncoding, &r);
    REPORTER_ASSERT(reporter, r.isEmpty());
}

#define ASSERT(expr) REPORTER_ASSERT(r, expr)

DEF_TEST(Paint_MoreFlattening, r) {
    SkPaint paint;
    paint.setColor(0x00AABBCC);
    paint.setBlendMode(SkBlendMode::kModulate);
    paint.setLooper(nullptr);  // Default value, ignored.

    SkBinaryWriteBuffer writer;
    SkPaintPriv::Flatten(paint, writer);

    SkAutoMalloc buf(writer.bytesWritten());
    writer.writeToMemory(buf.get());
    SkReadBuffer reader(buf.get(), writer.bytesWritten());

    SkPaint other;
    SkPaintPriv::Unflatten(&other, reader, nullptr);
    ASSERT(reader.offset() == writer.bytesWritten());

    // No matter the encoding, these must always hold.
    ASSERT(other.getColor()      == paint.getColor());
    ASSERT(other.getLooper()     == paint.getLooper());
    ASSERT(other.getBlendMode()  == paint.getBlendMode());
}

DEF_TEST(Paint_getHash, r) {
    // Try not to inspect the actual hash values in here.
    // We might want to change the hash function.

    SkPaint paint;
    const uint32_t defaultHash = paint.getHash();

    // Check that some arbitrary field affects the hash.
    paint.setColor(0xFF00FF00);
    REPORTER_ASSERT(r, paint.getHash() != defaultHash);
    paint.setColor(SK_ColorBLACK);  // Reset to default value.
    REPORTER_ASSERT(r, paint.getHash() == defaultHash);

    // This is part of fBitfields, the last field we hash.
    paint.setBlendMode(SkBlendMode::kSrc);
    REPORTER_ASSERT(r, paint.getHash() != defaultHash);
    paint.setBlendMode(SkBlendMode::kSrcOver);
    REPORTER_ASSERT(r, paint.getHash() == defaultHash);
}

#include "SkColorMatrixFilter.h"

DEF_TEST(Paint_nothingToDraw, r) {
    SkPaint paint;

    REPORTER_ASSERT(r, !paint.nothingToDraw());
    paint.setAlpha(0);
    REPORTER_ASSERT(r, paint.nothingToDraw());

    paint.setAlpha(0xFF);
    paint.setBlendMode(SkBlendMode::kDst);
    REPORTER_ASSERT(r, paint.nothingToDraw());

    paint.setAlpha(0);
    paint.setBlendMode(SkBlendMode::kSrcOver);

    SkColorMatrix cm;
    cm.setIdentity();   // does not change alpha
    paint.setColorFilter(SkColorFilters::MatrixRowMajor255(cm.fMat));
    REPORTER_ASSERT(r, paint.nothingToDraw());

    cm.postTranslate(0, 0, 0, 1);    // wacks alpha
    paint.setColorFilter(SkColorFilters::MatrixRowMajor255(cm.fMat));
    REPORTER_ASSERT(r, !paint.nothingToDraw());
}

DEF_TEST(Font_getpos, r) {
    SkFont font;
    const char text[] = "Hamburgefons!@#!#23425,./;'[]";
    int count = font.countText(text, strlen(text), kUTF8_SkTextEncoding);
    SkAutoTArray<uint16_t> glyphStorage(count);
    uint16_t* glyphs = glyphStorage.get();
    (void)font.textToGlyphs(text, strlen(text), kUTF8_SkTextEncoding, glyphs, count);

    SkAutoTArray<SkScalar> widthStorage(count);
    SkAutoTArray<SkScalar> xposStorage(count);
    SkAutoTArray<SkPoint> posStorage(count);

    SkScalar* widths = widthStorage.get();
    SkScalar* xpos = xposStorage.get();
    SkPoint* pos = posStorage.get();

    for (bool subpix : { false, true }) {
        font.setSubpixel(subpix);
        for (auto hint : { kNo_SkFontHinting, kSlight_SkFontHinting, kNormal_SkFontHinting, kFull_SkFontHinting}) {
            font.setHinting(hint);
            for (auto size : { 1.0f, 12.0f, 100.0f }) {
                font.setSize(size);

                font.getWidths(glyphs, count, widths);
                font.getXPos(glyphs, count, xpos, 10);
                font.getPos(glyphs, count, pos, {10, 20});

                auto nearly_eq = [](SkScalar a, SkScalar b) {
                    return SkScalarAbs(a - b) < 0.000001f;
                };

                SkScalar x = 10;
                for (int i = 0; i < count; ++i) {
                    REPORTER_ASSERT(r, nearly_eq(x,  xpos[i]));
                    REPORTER_ASSERT(r, nearly_eq(x,   pos[i].fX));
                    REPORTER_ASSERT(r, nearly_eq(20,  pos[i].fY));
                    x += widths[i];
                }
            }
        }
    }
}
