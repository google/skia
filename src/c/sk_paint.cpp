/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPaint.h"

#include "sk_paint.h"
#include "sk_types_priv.h"

#define MAKE_FROM_TO_NAME(FROM)     g_ ## FROM ## _map

const struct {
    sk_stroke_cap_t fC;
    SkPaint::Cap    fSK;
} MAKE_FROM_TO_NAME(sk_stroke_cap_t)[] = {
    { BUTT_SK_STROKE_CAP,   SkPaint::kButt_Cap   },
    { ROUND_SK_STROKE_CAP,  SkPaint::kRound_Cap  },
    { SQUARE_SK_STROKE_CAP, SkPaint::kSquare_Cap },
};

const struct {
    sk_stroke_join_t fC;
    SkPaint::Join    fSK;
} MAKE_FROM_TO_NAME(sk_stroke_join_t)[] = {
    { MITER_SK_STROKE_JOIN, SkPaint::kMiter_Join },
    { ROUND_SK_STROKE_JOIN, SkPaint::kRound_Join },
    { BEVEL_SK_STROKE_JOIN, SkPaint::kBevel_Join },
};

const struct {
    sk_text_align_t fC;
    SkPaint::Align  fSK;
} MAKE_FROM_TO_NAME(sk_text_align_t)[] = {
    { LEFT_SK_TEXT_ALIGN, SkPaint::kLeft_Align },
    { CENTER_SK_TEXT_ALIGN, SkPaint::kCenter_Align },
    { RIGHT_SK_TEXT_ALIGN, SkPaint::kRight_Align },
};

const struct {
    sk_text_encoding_t    fC;
    SkPaint::TextEncoding fSK;
} MAKE_FROM_TO_NAME(sk_text_encoding_t)[] = {
    { UTF8_SK_TEXT_ENCODING, SkPaint::kUTF8_TextEncoding },
    { UTF16_SK_TEXT_ENCODING, SkPaint::kUTF16_TextEncoding },
    { UTF32_SK_TEXT_ENCODING, SkPaint::kUTF32_TextEncoding },
    { GLYPH_ID_SK_TEXT_ENCODING, SkPaint::kGlyphID_TextEncoding },
};

#define CType           sk_stroke_cap_t
#define SKType          SkPaint::Cap
#define CTypeSkTypeMap  MAKE_FROM_TO_NAME(sk_stroke_cap_t)
#include "sk_c_from_to.h"

#define CType           sk_stroke_join_t
#define SKType          SkPaint::Join
#define CTypeSkTypeMap  MAKE_FROM_TO_NAME(sk_stroke_join_t)
#include "sk_c_from_to.h"

#define CType           sk_text_align_t
#define SKType          SkPaint::Align
#define CTypeSkTypeMap  MAKE_FROM_TO_NAME(sk_text_align_t)
#include "sk_c_from_to.h"

#define CType           sk_text_encoding_t
#define SKType          SkPaint::TextEncoding
#define CTypeSkTypeMap  MAKE_FROM_TO_NAME(sk_text_encoding_t)
#include "sk_c_from_to.h"

//////////////////////////////////////////////////////////////////////////////////////////////////

sk_paint_t* sk_paint_new() { return (sk_paint_t*)new SkPaint; }

void sk_paint_delete(sk_paint_t* cpaint) { delete AsPaint(cpaint); }

bool sk_paint_is_antialias(const sk_paint_t* cpaint) {
    return AsPaint(*cpaint).isAntiAlias();
}

void sk_paint_set_antialias(sk_paint_t* cpaint, bool aa) {
    AsPaint(cpaint)->setAntiAlias(aa);
}

sk_color_t sk_paint_get_color(const sk_paint_t* cpaint) {
    return AsPaint(*cpaint).getColor();
}

void sk_paint_set_color(sk_paint_t* cpaint, sk_color_t c) {
    AsPaint(cpaint)->setColor(c);
}

void sk_paint_set_shader(sk_paint_t* cpaint, sk_shader_t* cshader) {
    AsPaint(cpaint)->setShader(AsShader(cshader));
}

void sk_paint_set_maskfilter(sk_paint_t* cpaint, sk_maskfilter_t* cfilter) {
    AsPaint(cpaint)->setMaskFilter(AsMaskFilter(cfilter));
}

bool sk_paint_is_stroke(const sk_paint_t* cpaint) {
    return AsPaint(*cpaint).getStyle() != SkPaint::kFill_Style;
}

void sk_paint_set_stroke(sk_paint_t* cpaint, bool doStroke) {
    AsPaint(cpaint)->setStyle(doStroke ? SkPaint::kStroke_Style : SkPaint::kFill_Style);
}

float sk_paint_get_stroke_width(const sk_paint_t* cpaint) {
    return AsPaint(*cpaint).getStrokeWidth();
}

void sk_paint_set_stroke_width(sk_paint_t* cpaint, float width) {
    AsPaint(cpaint)->setStrokeWidth(width);
}

float sk_paint_get_stroke_miter(const sk_paint_t* cpaint) {
    return AsPaint(*cpaint).getStrokeMiter();
}

void sk_paint_set_stroke_miter(sk_paint_t* cpaint, float miter) {
    AsPaint(cpaint)->setStrokeMiter(miter);
}

sk_stroke_cap_t sk_paint_get_stroke_cap(const sk_paint_t* cpaint) {
    sk_stroke_cap_t ccap;
    if (!find_c(AsPaint(*cpaint).getStrokeCap(), &ccap)) {
        ccap = BUTT_SK_STROKE_CAP;
    }
    return ccap;
}

void sk_paint_set_stroke_cap(sk_paint_t* cpaint, sk_stroke_cap_t ccap) {
    SkPaint::Cap skcap;
    if (find_sk(ccap, &skcap)) {
        AsPaint(cpaint)->setStrokeCap(skcap);
    } else {
        // unknown ccap
    }
}

sk_stroke_join_t sk_paint_get_stroke_join(const sk_paint_t* cpaint) {
    sk_stroke_join_t cjoin;
    if (!find_c(AsPaint(*cpaint).getStrokeJoin(), &cjoin)) {
        cjoin = MITER_SK_STROKE_JOIN;
    }
    return cjoin;
}

void sk_paint_set_stroke_join(sk_paint_t* cpaint, sk_stroke_join_t cjoin) {
    SkPaint::Join skjoin;
    if (find_sk(cjoin, &skjoin)) {
        AsPaint(cpaint)->setStrokeJoin(skjoin);
    } else {
        // unknown cjoin
    }
}

void sk_paint_set_xfermode_mode(sk_paint_t* paint, sk_xfermode_mode_t mode) {
    SkASSERT(paint);

    AsPaint(paint)->setXfermodeMode(MapXferMode (mode));
}

sk_typeface_t* sk_paint_get_typeface(sk_paint_t* paint)
{
    return (sk_typeface_t*) AsPaint(paint)->getTypeface();
}

void sk_paint_set_typeface(sk_paint_t* paint, sk_typeface_t* typeface)
{
    AsPaint(paint)->setTypeface((SkTypeface*) typeface);
}

float sk_paint_get_textsize(sk_paint_t* paint)
{
    return AsPaint(paint)->getTextSize();
}

void sk_paint_set_textsize(sk_paint_t* paint, float size)
{
    AsPaint(paint)->setTextSize(size);
}

sk_text_align_t sk_paint_get_text_align(const sk_paint_t* cpaint) {
    sk_text_align_t calign;
    if (!find_c(AsPaint(*cpaint).getTextAlign(), &calign)) {
        calign = LEFT_SK_TEXT_ALIGN;
    }
    return calign;
}

void sk_paint_set_text_align(sk_paint_t* cpaint, sk_text_align_t calign) {
    SkPaint::Align skalign;
    if (find_sk(calign, &skalign)) {
        AsPaint(cpaint)->setTextAlign(skalign);
    } else {
        // unknown calign
    }
}

sk_text_encoding_t sk_paint_get_text_encoding(const sk_paint_t* cpaint) {
    sk_text_encoding_t cencoding;
    if (!find_c(AsPaint(*cpaint).getTextEncoding(), &cencoding)) {
        cencoding = UTF8_SK_TEXT_ENCODING;
    }
    return cencoding;
}

void sk_paint_set_text_encoding(sk_paint_t* cpaint, sk_text_encoding_t cencoding) {
    SkPaint::TextEncoding skencoding;
    if (find_sk(cencoding, &skencoding)) {
        AsPaint(cpaint)->setTextEncoding(skencoding);
    } else {
        // unknown cencoding
    }
}

float sk_paint_get_text_scale_x(const sk_paint_t* cpaint) {
    return AsPaint(cpaint)->getTextScaleX();
}

void sk_paint_set_text_scale_x(sk_paint_t* cpaint, float scale) {
    AsPaint(cpaint)->setTextScaleX(scale);
}

float sk_paint_get_text_skew_x(const sk_paint_t* cpaint) {
    return AsPaint(cpaint)->getTextSkewX();
}

void sk_paint_set_text_skew_x(sk_paint_t* cpaint, float skew) {
    AsPaint(cpaint)->setTextSkewX(skew);
}

