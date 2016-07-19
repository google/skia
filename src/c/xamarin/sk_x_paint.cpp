/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPaint.h"
#include "SkTypeface.h"
#include "SkColorFilter.h"

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
    AsPaint(cpaint)->setColorFilter(sk_ref_sp(AsColorFilter(cfilter)));
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
    if (SkXfermode::AsMode(AsPaint(paint)->getXfermode(), &mode)) {
        return (sk_xfermode_mode_t)mode;
    }
    return SRCOVER_SK_XFERMODE_MODE;
}

void sk_paint_set_filter_quality(sk_paint_t* cpaint, sk_filter_quality_t filterQuality)
{
    SkASSERT(cpaint);
    AsPaint(cpaint)->setFilterQuality((SkFilterQuality)filterQuality);
}

sk_filter_quality_t sk_paint_get_filter_quality(sk_paint_t* cpaint)
{
    SkASSERT(cpaint);
    return (sk_filter_quality_t)AsPaint(cpaint)->getFilterQuality();
}

sk_typeface_t* sk_paint_get_typeface(sk_paint_t* paint)
{
    return ToTypeface(AsPaint(paint)->getTypeface());
}

void sk_paint_set_typeface(sk_paint_t* paint, sk_typeface_t* typeface)
{
    AsPaint(paint)->setTypeface(sk_ref_sp(AsTypeface(typeface)));
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
    return (sk_text_align_t)AsPaint(*cpaint).getTextAlign();
}

void sk_paint_set_text_align(sk_paint_t* cpaint, sk_text_align_t calign) {
    AsPaint(cpaint)->setTextAlign((SkPaint::Align)calign);
}

sk_text_encoding_t sk_paint_get_text_encoding(const sk_paint_t* cpaint) {
    return (sk_text_encoding_t)AsPaint(*cpaint).getTextEncoding();
}

void sk_paint_set_text_encoding(sk_paint_t* cpaint, sk_text_encoding_t cencoding) {
    AsPaint(cpaint)->setTextEncoding((SkPaint::TextEncoding)cencoding);
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

sk_path_effect_t* sk_paint_get_path_effect(sk_paint_t* cpaint) {
    return ToPathEffect(AsPaint(cpaint)->getPathEffect());
}

void sk_paint_set_path_effect(sk_paint_t* cpaint, sk_path_effect_t* effect) {
    AsPaint(cpaint)->setPathEffect(sk_ref_sp(AsPathEffect(effect)));
}
