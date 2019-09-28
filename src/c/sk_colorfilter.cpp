/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkColorMatrixFilter.h"
#include "include/effects/SkHighContrastFilter.h"
#include "include/effects/SkLumaColorFilter.h"
#include "include/effects/SkTableColorFilter.h"

#include "include/c/sk_colorfilter.h"

#include "src/c/sk_types_priv.h"

void sk_colorfilter_unref(sk_colorfilter_t* filter) {
    SkSafeUnref(AsColorFilter(filter));
}

sk_colorfilter_t* sk_colorfilter_new_mode(sk_color_t c, sk_blendmode_t cmode) {
    return ToColorFilter(SkColorFilters::Blend(c, (SkBlendMode)cmode).release());
}

sk_colorfilter_t* sk_colorfilter_new_lighting(sk_color_t mul, sk_color_t add) {
    return ToColorFilter(SkColorMatrixFilter::MakeLightingFilter(mul, add).release());
}

sk_colorfilter_t* sk_colorfilter_new_compose(sk_colorfilter_t* outer, sk_colorfilter_t* inner) {
    return ToColorFilter(SkColorFilters::Compose(sk_ref_sp(AsColorFilter(outer)), sk_ref_sp(AsColorFilter(inner))).release());
}

sk_colorfilter_t* sk_colorfilter_new_color_matrix(const float array[20]) {
    return ToColorFilter(SkColorFilters::Matrix(array).release());
}

sk_colorfilter_t* sk_colorfilter_new_luma_color() {
    return ToColorFilter(SkLumaColorFilter::Make().release());
}

sk_colorfilter_t* sk_colorfilter_new_high_contrast(const sk_highcontrastconfig_t* config) {
    return ToColorFilter(SkHighContrastFilter::Make(*AsHighContrastConfig(config)).release());
}

sk_colorfilter_t* sk_colorfilter_new_table(const uint8_t table[256]) {
    return ToColorFilter(SkTableColorFilter::Make(table).release());
}

sk_colorfilter_t* sk_colorfilter_new_table_argb(const uint8_t tableA[256], const uint8_t tableR[256], const uint8_t tableG[256], const uint8_t tableB[256]) {
    return ToColorFilter(SkTableColorFilter::MakeARGB(tableA, tableR, tableG, tableB).release());
}
