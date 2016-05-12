/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPaint.h"

#include "xamarin/sk_x_paint.h"

#include "../sk_types_priv.h"
#include "sk_x_types_priv.h"

bool sk_paint_is_dither(const sk_paint_t* cpaint) {
    return AsPaint(*cpaint).isDither();
}

void sk_paint_set_dither(sk_paint_t* cpaint, bool isdither) {
    AsPaint(cpaint)->setDither(isdither);
}

bool sk_paint_is_verticaltext(const sk_paint_t* cpaint) {
    return AsPaint(*cpaint).isVerticalText();
}

void sk_paint_set_verticaltext(sk_paint_t* cpaint, bool vt) {
    AsPaint(cpaint)->setVerticalText(vt);
}

sk_shader_t* sk_paint_get_shader(sk_paint_t* cpaint) {
    return ToShader(AsPaint(cpaint)->getShader());
}

sk_maskfilter_t* sk_paint_get_maskfilter(sk_paint_t* cpaint) {
    return ToMaskFilter(AsPaint(cpaint)->getMaskFilter());
}

void sk_paint_set_colorfilter(sk_paint_t* cpaint, sk_colorfilter_t* cfilter) {
    AsPaint(cpaint)->setColorFilter(AsColorFilter(cfilter));
}

sk_colorfilter_t* sk_paint_get_colorfilter(sk_paint_t* cpaint) {
    return ToColorFilter(AsPaint(cpaint)->getColorFilter());
}

void sk_paint_set_imagefilter(sk_paint_t* cpaint, sk_imagefilter_t* cfilter) {
    AsPaint(cpaint)->setImageFilter(AsImageFilter(cfilter));
}

sk_imagefilter_t* sk_paint_get_imagefilter(sk_paint_t* cpaint) {
    return ToImageFilter(AsPaint(cpaint)->getImageFilter());
}

sk_xfermode_mode_t sk_paint_get_xfermode_mode(sk_paint_t* paint) {
    SkASSERT(paint);

    SkXfermode::Mode mode;
    sk_xfermode_mode_t cmode;
    if (SkXfermode::AsMode(AsPaint(paint)->getXfermode(), &mode) && find_c(mode, &cmode)) {
        return cmode;
    }
    return SRCOVER_SK_XFERMODE_MODE;
}

void sk_paint_set_filter_quality(sk_paint_t* cpaint, sk_filter_quality_t filterQuality)
{
    SkASSERT(cpaint);
    SkFilterQuality fq;
    if (find_sk(filterQuality, &fq)) {
        AsPaint(cpaint)->setFilterQuality(fq);
    }
}

sk_filter_quality_t sk_paint_get_filter_quality(sk_paint_t* cpaint)
{
    SkASSERT(cpaint);
    SkFilterQuality fq;
    sk_filter_quality_t cfq;
    fq = AsPaint(cpaint)->getFilterQuality();
    if (find_c(fq, &cfq)) {
        return cfq;
    }
    else {
        return NONE_SK_FILTER_QUALITY;
    }
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

size_t sk_paint_break_text(const sk_paint_t* cpaint, const void* text, size_t length, float maxWidth, float* measuredWidth) {
    return AsPaint(cpaint)->breakText(text, length, maxWidth, measuredWidth);
}

float sk_paint_measure_text(const sk_paint_t* cpaint, const void* text, size_t length, sk_rect_t* bounds) {
    return AsPaint(cpaint)->measureText(text, length, AsRect(bounds));
}

sk_path_t* sk_paint_get_text_path(sk_paint_t* cpaint, const void* text, size_t length, float x, float y) {
    SkPath* path = new SkPath();
    AsPaint(cpaint)->getTextPath(text, length, x, y, path);
    return ToPath(path);
}

sk_path_t* sk_paint_get_pos_text_path(sk_paint_t* cpaint, const void* text, size_t length, const sk_point_t pos[]) {
    SkPath* path = new SkPath();
    AsPaint(cpaint)->getPosTextPath(text, length, reinterpret_cast<const SkPoint*>(pos), path);
    return ToPath(path);
}

float sk_paint_get_fontmetrics(sk_paint_t* cpaint, sk_fontmetrics_t* cfontmetrics, float scale)
{
    SkPaint *paint = AsPaint(cpaint);
    return paint->getFontMetrics(AsFontMetrics(cfontmetrics), scale);
}
