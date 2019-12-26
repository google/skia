/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFont.h"
#include "include/core/SkTypeface.h"

#include "include/c/sk_font.h"

#include "src/c/sk_types_priv.h"

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
