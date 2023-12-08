/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBlendMode.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorType.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathUtils.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/effects/SkColorMatrix.h"
#include "include/private/base/SkTemplates.h"
#include "src/base/SkAutoMalloc.h"
#include "src/core/SkBlurMask.h"
#include "src/core/SkPaintPriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "tests/Test.h"
#include "tools/fonts/FontToolUtils.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <optional>

#undef ASSERT

using namespace skia_private;

DEF_TEST(Paint_copy, reporter) {
    SkPaint paint;
    // set a few member variables
    paint.setStyle(SkPaint::kStrokeAndFill_Style);
    paint.setStrokeWidth(SkIntToScalar(2));
    // set a few pointers
    paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle,
                                               SkBlurMask::ConvertRadiusToSigma(1)));

    // copy the paint using the copy constructor and check they are the same
    SkPaint copiedPaint = paint;
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
    skpathutils::FillPathWithPaint(path, paint, &stroke);
    strokeR = stroke.getBounds();

    SkRect maxR = fillR;
    SkScalar miter = std::max(SK_Scalar1, paint.getStrokeMiter());
    SkScalar inset = paint.getStrokeJoin() == SkPaint::kMiter_Join ?
                            paint.getStrokeWidth() * miter :
                            paint.getStrokeWidth();
    maxR.inset(-inset, -inset);

    // test that our stroke didn't explode
    REPORTER_ASSERT(reporter, maxR.contains(strokeR));
}

DEF_TEST(Paint_flattening, reporter) {
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
    for (size_t index = 0; index < std::size(array); ++index) {         \
        paint.setter(array[index]);

    SkPaint paint;
    paint.setAntiAlias(true);

    // we don't serialize hinting or encoding -- soon to be removed from paint

    FOR_SETUP(l, caps, setStrokeCap)
    FOR_SETUP(m, joins, setStrokeJoin)
    FOR_SETUP(p, styles, setStyle)

    SkBinaryWriteBuffer writer({});
    SkPaintPriv::Flatten(paint, writer);

    SkAutoMalloc buf(writer.bytesWritten());
    writer.writeToMemory(buf.get());
    SkReadBuffer reader(buf.get(), writer.bytesWritten());

    SkPaint paint2 = reader.readPaint();
    REPORTER_ASSERT(reporter, paint2 == paint);

    }}}
#undef FOR_SETUP

}

// found and fixed for android: not initializing rect for string's of length 0
DEF_TEST(Paint_regression_measureText, reporter) {

    SkFont font = ToolUtils::DefaultFont();
    font.setSize(12.0f);

    SkRect r;
    r.setLTRB(SK_ScalarNaN, SK_ScalarNaN, SK_ScalarNaN, SK_ScalarNaN);

    // test that the rect was reset
    font.measureText("", 0, SkTextEncoding::kUTF8, &r);
    REPORTER_ASSERT(reporter, r.isEmpty());
}

#define ASSERT(expr) REPORTER_ASSERT(r, expr)

DEF_TEST(Paint_MoreFlattening, r) {
    SkPaint paint;
    paint.setColor(0x00AABBCC);
    paint.setBlendMode(SkBlendMode::kModulate);

    SkBinaryWriteBuffer writer({});
    SkPaintPriv::Flatten(paint, writer);

    SkAutoMalloc buf(writer.bytesWritten());
    writer.writeToMemory(buf.get());
    SkReadBuffer reader(buf.get(), writer.bytesWritten());

    SkPaint other = reader.readPaint();
    ASSERT(reader.offset() == writer.bytesWritten());

    // No matter the encoding, these must always hold.
    ASSERT(other.getColor()    == paint.getColor());
    ASSERT(other.asBlendMode() == paint.asBlendMode());
}

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
    paint.setColorFilter(SkColorFilters::Matrix(cm));
    REPORTER_ASSERT(r, paint.nothingToDraw());

    cm.postTranslate(0, 0, 0, 1.0f/255);    // wacks alpha
    paint.setColorFilter(SkColorFilters::Matrix(cm));
    REPORTER_ASSERT(r, !paint.nothingToDraw());
}

DEF_TEST(Font_getpos, r) {
    SkFont font = ToolUtils::DefaultFont();
    const char text[] = "Hamburgefons!@#!#23425,./;'[]";
    int count = font.countText(text, strlen(text), SkTextEncoding::kUTF8);
    AutoTArray<uint16_t> glyphStorage(count);
    uint16_t* glyphs = glyphStorage.get();
    (void)font.textToGlyphs(text, strlen(text), SkTextEncoding::kUTF8, glyphs, count);

    AutoTArray<SkScalar> widthStorage(count);
    AutoTArray<SkScalar> xposStorage(count);
    AutoTArray<SkPoint> posStorage(count);

    SkScalar* widths = widthStorage.get();
    SkScalar* xpos = xposStorage.get();
    SkPoint* pos = posStorage.get();

    for (bool subpix : { false, true }) {
        font.setSubpixel(subpix);
        for (auto hint : { SkFontHinting::kNone, SkFontHinting::kSlight, SkFontHinting::kNormal, SkFontHinting::kFull}) {
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

DEF_TEST(Paint_dither, reporter) {
    SkPaint p;
    p.setDither(true);

    bool shouldDither = SkPaintPriv::ShouldDither(p, kBGRA_8888_SkColorType);

    REPORTER_ASSERT(reporter, !shouldDither);
}
