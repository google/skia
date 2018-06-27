/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorMatrixFilter.h"
#include "SkHighContrastFilter.h"
#include "SkLumaColorFilter.h"
#include "SkTableColorFilter.h"

#include "sk_colorfilter.h"

#include "sk_types_priv.h"

void sk_colorfilter_unref(sk_colorfilter_t* filter) {
    SkSafeUnref(AsColorFilter(filter));
}

sk_colorfilter_t* sk_colorfilter_new_mode(sk_color_t c, sk_blendmode_t cmode) {

    sk_sp<SkColorFilter> filter = SkColorFilter::MakeModeFilter(c, (SkBlendMode)cmode);
    return ToColorFilter(filter.release());
}

sk_colorfilter_t* sk_colorfilter_new_lighting(sk_color_t mul, sk_color_t add) {

    sk_sp<SkColorFilter> filter = SkColorMatrixFilter::MakeLightingFilter(
        mul,
        add);
    return ToColorFilter(filter.release());
}

sk_colorfilter_t* sk_colorfilter_new_compose(sk_colorfilter_t* outer, sk_colorfilter_t* inner) {

    sk_sp<SkColorFilter> filter = SkColorFilter::MakeComposeFilter(
        sk_ref_sp(AsColorFilter(outer)),
        sk_ref_sp(AsColorFilter(inner)));
    return ToColorFilter(filter.release());
}

sk_colorfilter_t* sk_colorfilter_new_color_matrix(const float array[20]) {

    sk_sp<SkColorFilter> filter = SkColorFilter::MakeMatrixFilterRowMajor255(
        array);
    return ToColorFilter(filter.release());
}

sk_colorfilter_t* sk_colorfilter_new_luma_color() {

    sk_sp<SkColorFilter> filter = SkLumaColorFilter::Make();
    return ToColorFilter(filter.release());
}

sk_colorfilter_t* sk_colorfilter_new_high_contrast(const sk_highcontrastconfig_t* config) {

    sk_sp<SkColorFilter> filter = SkHighContrastFilter::Make(AsHighContrastConfig(*config));
    return ToColorFilter(filter.release());
}

sk_colorfilter_t* sk_colorfilter_new_table(const uint8_t table[256]) {

    sk_sp<SkColorFilter> filter = SkTableColorFilter::Make(table);
    return ToColorFilter(filter.release());
}

sk_colorfilter_t* sk_colorfilter_new_table_argb(const uint8_t tableA[256], const uint8_t tableR[256], const uint8_t tableG[256], const uint8_t tableB[256]) {
    sk_sp<SkColorFilter> filter = SkTableColorFilter::MakeARGB(tableA, tableR, tableG, tableB);
    return ToColorFilter(filter.release());
}
