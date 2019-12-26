/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorFilter.h"
#include "include/core/SkFont.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkShader.h"
#include "include/core/SkTypeface.h"

#include "include/c/sk_paint.h"

#include "src/c/sk_types_priv.h"

//////////////////////////////////////////////////////////////////////////////////////////////////

sk_paint_t* sk_paint_new() {
    return ToPaint(new SkPaint());
}

sk_paint_t* sk_paint_clone(sk_paint_t* paint) {
    return ToPaint(new SkPaint(*AsPaint(paint)));
}

void sk_paint_delete(sk_paint_t* cpaint) { delete AsPaint(cpaint); }

void sk_paint_reset(sk_paint_t* cpaint) {
    AsPaint(cpaint)->reset();
}

bool sk_paint_is_antialias(const sk_paint_t* cpaint) {
    return AsPaint(cpaint)->isAntiAlias();
}

void sk_paint_set_antialias(sk_paint_t* cpaint, bool aa) {
    AsPaint(cpaint)->setAntiAlias(aa);
}

sk_color_t sk_paint_get_color(const sk_paint_t* cpaint) {
    return AsPaint(cpaint)->getColor();
}

void sk_paint_set_color(sk_paint_t* cpaint, sk_color_t c) {
    AsPaint(cpaint)->setColor(c);
}

void sk_paint_set_shader(sk_paint_t* cpaint, sk_shader_t* cshader) {
    AsPaint(cpaint)->setShader(sk_ref_sp(AsShader(cshader)));
}

void sk_paint_set_maskfilter(sk_paint_t* cpaint, sk_maskfilter_t* cfilter) {
    AsPaint(cpaint)->setMaskFilter(sk_ref_sp(AsMaskFilter(cfilter)));
}

sk_paint_style_t sk_paint_get_style(const sk_paint_t* cpaint) {
    return (sk_paint_style_t)AsPaint(cpaint)->getStyle();
}

void sk_paint_set_style(sk_paint_t* cpaint, sk_paint_style_t style) {
    AsPaint(cpaint)->setStyle((SkPaint::Style)style);
}

float sk_paint_get_stroke_width(const sk_paint_t* cpaint) {
    return AsPaint(cpaint)->getStrokeWidth();
}

void sk_paint_set_stroke_width(sk_paint_t* cpaint, float width) {
    AsPaint(cpaint)->setStrokeWidth(width);
}

float sk_paint_get_stroke_miter(const sk_paint_t* cpaint) {
    return AsPaint(cpaint)->getStrokeMiter();
}

void sk_paint_set_stroke_miter(sk_paint_t* cpaint, float miter) {
    AsPaint(cpaint)->setStrokeMiter(miter);
}

sk_stroke_cap_t sk_paint_get_stroke_cap(const sk_paint_t* cpaint) {
    return (sk_stroke_cap_t)AsPaint(cpaint)->getStrokeCap();
}

void sk_paint_set_stroke_cap(sk_paint_t* cpaint, sk_stroke_cap_t ccap) {
    AsPaint(cpaint)->setStrokeCap((SkPaint::Cap)ccap);
}

sk_stroke_join_t sk_paint_get_stroke_join(const sk_paint_t* cpaint) {
    return (sk_stroke_join_t)AsPaint(cpaint)->getStrokeJoin();
}

void sk_paint_set_stroke_join(sk_paint_t* cpaint, sk_stroke_join_t cjoin) {
    AsPaint(cpaint)->setStrokeJoin((SkPaint::Join)cjoin);
}

void sk_paint_set_blendmode(sk_paint_t* paint, sk_blendmode_t mode) {
    AsPaint(paint)->setBlendMode((SkBlendMode)mode);
}

bool sk_paint_is_dither(const sk_paint_t* cpaint) {
    return AsPaint(cpaint)->isDither();
}

void sk_paint_set_dither(sk_paint_t* cpaint, bool isdither) {
    AsPaint(cpaint)->setDither(isdither);
}

sk_shader_t* sk_paint_get_shader(sk_paint_t* cpaint) {
    return ToShader(AsPaint(cpaint)->refShader().release());
}

sk_maskfilter_t* sk_paint_get_maskfilter(sk_paint_t* cpaint) {
    return ToMaskFilter(AsPaint(cpaint)->refMaskFilter().release());
}

void sk_paint_set_colorfilter(sk_paint_t* cpaint, sk_colorfilter_t* cfilter) {
    AsPaint(cpaint)->setColorFilter(sk_ref_sp(AsColorFilter(cfilter)));
}

sk_colorfilter_t* sk_paint_get_colorfilter(sk_paint_t* cpaint) {
    return ToColorFilter(AsPaint(cpaint)->refColorFilter().release());
}

void sk_paint_set_imagefilter(sk_paint_t* cpaint, sk_imagefilter_t* cfilter) {
    AsPaint(cpaint)->setImageFilter(sk_ref_sp(AsImageFilter(cfilter)));
}

sk_imagefilter_t* sk_paint_get_imagefilter(sk_paint_t* cpaint) {
    return ToImageFilter(AsPaint(cpaint)->refImageFilter().release());
}

sk_blendmode_t sk_paint_get_blendmode(sk_paint_t* paint) {
    return (sk_blendmode_t)AsPaint(paint)->getBlendMode();
}

void sk_paint_set_filter_quality(sk_paint_t* cpaint, sk_filter_quality_t filterQuality) {
    AsPaint(cpaint)->setFilterQuality((SkFilterQuality)filterQuality);
}

sk_filter_quality_t sk_paint_get_filter_quality(sk_paint_t* cpaint) {
    return (sk_filter_quality_t)AsPaint(cpaint)->getFilterQuality();
}

sk_path_effect_t* sk_paint_get_path_effect(sk_paint_t* cpaint) {
    return ToPathEffect(AsPaint(cpaint)->refPathEffect().release());
}

void sk_paint_set_path_effect(sk_paint_t* cpaint, sk_path_effect_t* effect) {
    AsPaint(cpaint)->setPathEffect(sk_ref_sp(AsPathEffect(effect)));
}

bool sk_paint_get_fill_path(const sk_paint_t* cpaint, const sk_path_t* src, sk_path_t* dst, const sk_rect_t* cullRect, float resScale) {
    return AsPaint(cpaint)->getFillPath(*AsPath(src), AsPath(dst), AsRect(cullRect), resScale);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

sk_font_t* sk_font_new() {
    return ToFont(new SkFont());
}

sk_font_t* sk_font_new_with_values(sk_typeface_t* typeface, float size, float scaleX, float skewX) {
    return ToFont(new SkFont(sk_ref_sp(AsTypeface(typeface)), size, scaleX, skewX));
}

void sk_font_delete(sk_font_t* font) {
    delete AsFont(font);
}

bool sk_font_is_force_auto_hinting(const sk_font_t* font) {
    return AsFont(font)->isForceAutoHinting();
}

void sk_font_set_force_auto_hinting(sk_font_t* font, bool value) {
    AsFont(font)->setForceAutoHinting(value);
}

bool sk_font_is_embedded_bitmaps(const sk_font_t* font) {
    return AsFont(font)->isEmbeddedBitmaps();
}

void sk_font_set_embedded_bitmaps(sk_font_t* font, bool value) {
    AsFont(font)->setEmbeddedBitmaps(value);
}

bool sk_font_is_subpixel(const sk_font_t* font) {
    return AsFont(font)->isSubpixel();
}

void sk_font_set_subpixel(sk_font_t* font, bool value) {
    AsFont(font)->setSubpixel(value);
}

bool sk_font_is_linear_metrics(const sk_font_t* font) {
    return AsFont(font)->isLinearMetrics();
}

void sk_font_set_linear_metrics(sk_font_t* font, bool value) {
    AsFont(font)->setLinearMetrics(value);
}

bool sk_font_is_embolden(const sk_font_t* font) {
    return AsFont(font)->isEmbolden();
}

void sk_font_set_embolden(sk_font_t* font, bool value) {
    AsFont(font)->setEmbolden(value);
}

bool sk_font_is_baseline_snap(const sk_font_t* font) {
    return AsFont(font)->isBaselineSnap();
}

void sk_font_set_baseline_snap(sk_font_t* font, bool value) {
    AsFont(font)->setBaselineSnap(value);
}

sk_font_edging_t sk_font_get_edging(const sk_font_t* font) {
    return (sk_font_edging_t)AsFont(font)->getEdging();
}

void sk_font_set_edging(sk_font_t* font, sk_font_edging_t value) {
    AsFont(font)->setEdging((SkFont::Edging)value);
}

sk_font_hinting_t sk_font_get_hinting(const sk_font_t* font) {
    return (sk_font_hinting_t)AsFont(font)->getHinting();
}

void sk_font_set_hinting(sk_font_t* font, sk_font_hinting_t value) {
    AsFont(font)->setHinting((SkFontHinting)value);
}

sk_typeface_t* sk_font_get_typeface(const sk_font_t* font) {
    return ToTypeface(AsFont(font)->refTypeface().release());
}

void sk_font_set_typeface(sk_font_t* font, sk_typeface_t* value) {
    AsFont(font)->setTypeface(sk_ref_sp(AsTypeface(value)));
}

float sk_font_get_size(const sk_font_t* font) {
    return AsFont(font)->getSize();
}

void sk_font_set_size(sk_font_t* font, float value) {
    AsFont(font)->setSize(value);
}

float sk_font_get_scale_x(const sk_font_t* font) {
    return AsFont(font)->getScaleX();
}

void sk_font_set_scale_x(sk_font_t* font, float value) {
    AsFont(font)->setScaleX(value);
}

float sk_font_get_skew_x(const sk_font_t* font) {
    return AsFont(font)->getSkewX();
}

void sk_font_set_skew_x(sk_font_t* font, float value) {
    AsFont(font)->setSkewX(value);
}
