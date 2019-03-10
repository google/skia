/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorFilter.h"
#include "SkMaskFilter.h"
#include "SkPaint.h"
#include "SkPathEffect.h"
#include "SkShader.h"
#include "SkTypeface.h"

#include "sk_paint.h"

#include "sk_types_priv.h"

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

bool sk_paint_is_verticaltext(const sk_paint_t* cpaint) {
    return AsPaint(cpaint)->isVerticalText();
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
    AsPaint(cpaint)->setImageFilter(sk_ref_sp(AsImageFilter(cfilter)));
}

sk_imagefilter_t* sk_paint_get_imagefilter(sk_paint_t* cpaint) {
    return ToImageFilter(AsPaint(cpaint)->getImageFilter());
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

sk_typeface_t* sk_paint_get_typeface(sk_paint_t* paint) {
    return ToTypeface(AsPaint(paint)->getTypeface());
}

void sk_paint_set_typeface(sk_paint_t* paint, sk_typeface_t* typeface) {
    AsPaint(paint)->setTypeface(sk_ref_sp(AsTypeface(typeface)));
}

float sk_paint_get_textsize(sk_paint_t* paint) {
    return AsPaint(paint)->getTextSize();
}

void sk_paint_set_textsize(sk_paint_t* paint, float size) {
    AsPaint(paint)->setTextSize(size);
}

sk_text_align_t sk_paint_get_text_align(const sk_paint_t* cpaint) {
    return (sk_text_align_t)AsPaint(cpaint)->getTextAlign();
}

void sk_paint_set_text_align(sk_paint_t* cpaint, sk_text_align_t calign) {
    AsPaint(cpaint)->setTextAlign((SkPaint::Align)calign);
}

sk_text_encoding_t sk_paint_get_text_encoding(const sk_paint_t* cpaint) {
    return (sk_text_encoding_t)AsPaint(cpaint)->getTextEncoding();
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

float sk_paint_get_fontmetrics(sk_paint_t* cpaint, sk_fontmetrics_t* cfontmetrics, float scale) {
    return AsPaint(cpaint)->getFontMetrics(AsFontMetrics(cfontmetrics), scale);
}

sk_path_effect_t* sk_paint_get_path_effect(sk_paint_t* cpaint) {
    return ToPathEffect(AsPaint(cpaint)->getPathEffect());
}

void sk_paint_set_path_effect(sk_paint_t* cpaint, sk_path_effect_t* effect) {
    AsPaint(cpaint)->setPathEffect(sk_ref_sp(AsPathEffect(effect)));
}

bool sk_paint_is_linear_text(const sk_paint_t* cpaint) {
    return AsPaint(cpaint)->isLinearText();
}

void sk_paint_set_linear_text(sk_paint_t* cpaint, bool linearText) {
    AsPaint(cpaint)->setLinearText(linearText);
}

bool sk_paint_is_subpixel_text(const sk_paint_t* cpaint) {
    return AsPaint(cpaint)->isSubpixelText();
}

void sk_paint_set_subpixel_text(sk_paint_t* cpaint, bool subpixelText) {
    AsPaint(cpaint)->setSubpixelText(subpixelText);
}

bool sk_paint_is_lcd_render_text(const sk_paint_t* cpaint) {
    return AsPaint(cpaint)->isLCDRenderText();
}

void sk_paint_set_lcd_render_text(sk_paint_t* cpaint, bool lcdText) {
    AsPaint(cpaint)->setLCDRenderText(lcdText);
}

bool sk_paint_is_embedded_bitmap_text(const sk_paint_t* cpaint) {
    return AsPaint(cpaint)->isEmbeddedBitmapText();
}

void sk_paint_set_embedded_bitmap_text(sk_paint_t* cpaint, bool useEmbeddedBitmapText) {
    AsPaint(cpaint)->setEmbeddedBitmapText(useEmbeddedBitmapText);
}

bool sk_paint_is_autohinted(const sk_paint_t* cpaint) {
    return AsPaint(cpaint)->isAutohinted();
}

void sk_paint_set_autohinted(sk_paint_t* cpaint, bool useAutohinter) {
    AsPaint(cpaint)->setAutohinted(useAutohinter);
}

sk_paint_hinting_t sk_paint_get_hinting(const sk_paint_t* cpaint) {
    return (sk_paint_hinting_t)AsPaint(cpaint)->getHinting();
}

void sk_paint_set_hinting(sk_paint_t* cpaint, sk_paint_hinting_t hintingLevel) {
    AsPaint(cpaint)->setHinting((SkPaint::Hinting)hintingLevel);
}

bool sk_paint_is_fake_bold_text(const sk_paint_t* cpaint) {
    return AsPaint(cpaint)->isFakeBoldText();
}

void sk_paint_set_fake_bold_text(sk_paint_t* cpaint, bool fakeBoldText) {
    AsPaint(cpaint)->setFakeBoldText(fakeBoldText);
}

bool sk_paint_is_dev_kern_text(const sk_paint_t* cpaint) {
    return AsPaint(cpaint)->isDevKernText();
}

void sk_paint_set_dev_kern_text(sk_paint_t* cpaint, bool devKernText) {
    AsPaint(cpaint)->setDevKernText(devKernText);
}

bool sk_paint_get_fill_path(const sk_paint_t* cpaint, const sk_path_t* src, sk_path_t* dst, const sk_rect_t* cullRect, float resScale) {
    return AsPaint(cpaint)->getFillPath(*AsPath(src), AsPath(dst), AsRect(cullRect), resScale);
}

int sk_paint_text_to_glyphs(const sk_paint_t* cpaint, const void* text, size_t byteLength, uint16_t* glyphs) {
    return AsPaint(cpaint)->textToGlyphs(text, byteLength, glyphs);
}

bool sk_paint_contains_text(const sk_paint_t* cpaint, const void* text, size_t byteLength) {
    return AsPaint(cpaint)->containsText(text, byteLength);
}

int sk_paint_count_text(const sk_paint_t* cpaint, const void* text, size_t byteLength) {
    return AsPaint(cpaint)->countText(text, byteLength);
}

int sk_paint_get_text_widths(const sk_paint_t* cpaint, const void* text, size_t byteLength, float* widths, sk_rect_t* bounds) {
    return AsPaint(cpaint)->getTextWidths(text, byteLength, widths, AsRect(bounds));
}

int sk_paint_get_text_intercepts(const sk_paint_t* cpaint, const void* text, size_t byteLength, float x, float y, const float bounds[2], float* intervals) {
    return AsPaint(cpaint)->getTextIntercepts(text, byteLength, x, y, bounds, intervals);
}

int sk_paint_get_pos_text_intercepts(const sk_paint_t* cpaint, const void* text, size_t byteLength, sk_point_t* pos, const float bounds[2], float* intervals) {
    return AsPaint(cpaint)->getPosTextIntercepts(text, byteLength, AsPoint(pos), bounds, intervals);
}

int sk_paint_get_pos_text_h_intercepts(const sk_paint_t* cpaint, const void* text, size_t byteLength, float* xpos, float y, const float bounds[2], float* intervals) {
    return AsPaint(cpaint)->getPosTextHIntercepts(text, byteLength, xpos, y, bounds, intervals);
}

int sk_paint_get_pos_text_blob_intercepts(const sk_paint_t* cpaint, sk_textblob_t* blob, const float bounds[2], float* intervals) {
    return AsPaint(cpaint)->getTextBlobIntercepts(AsTextBlob(blob), bounds, intervals);
}
