/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../../include/effects/SkColorCubeFilter.h"
#include "../../include/effects/SkColorMatrixFilter.h"
#include "../../include/effects/SkLumaColorFilter.h"
#include "../../include/effects/SkTableColorFilter.h"

#include "xamarin/sk_x_colorfilter.h"

#include "../sk_types_priv.h"
#include "sk_x_types_priv.h"

void sk_colorfilter_unref(sk_colorfilter_t* filter) {
    SkSafeUnref(AsColorFilter(filter));
}

sk_colorfilter_t* sk_colorfilter_new_mode(sk_color_t c, sk_xfermode_mode_t cmode) {

    SkXfermode::Mode mode;
    if (!find_sk(cmode, &mode)) {
        return NULL;
    }

    SkColorFilter* filter = SkColorFilter::CreateModeFilter(
        c,
        mode);
    return ToColorFilter(filter);
}

sk_colorfilter_t* sk_colorfilter_new_lighting(sk_color_t mul, sk_color_t add) {

    SkColorFilter* filter = SkColorMatrixFilter::CreateLightingFilter(
        mul,
        add);
    return ToColorFilter(filter);
}

sk_colorfilter_t* sk_colorfilter_new_compose(sk_colorfilter_t* outer, sk_colorfilter_t* inner) {

    SkColorFilter* filter = SkColorFilter::CreateComposeFilter(
        AsColorFilter(outer),
        AsColorFilter(inner));
    return ToColorFilter(filter);
}

sk_colorfilter_t* sk_colorfilter_new_color_cube(sk_data_t* cubeData, int cubeDimension) {

    SkColorFilter* filter = SkColorCubeFilter::Create(
        AsData(cubeData),
        cubeDimension);
    return ToColorFilter(filter);
}

sk_colorfilter_t* sk_colorfilter_new_color_matrix(const float array[20]) {

    SkColorFilter* filter = SkColorMatrixFilter::Create(
        array);
    return ToColorFilter(filter);
}

sk_colorfilter_t* sk_colorfilter_new_luma_color() {

    SkColorFilter* filter = SkLumaColorFilter::Create();
    return ToColorFilter(filter);
}

sk_colorfilter_t* sk_colorfilter_new_table(const uint8_t table[256]) {

    SkColorFilter* filter = SkTableColorFilter::Create(table);
    return ToColorFilter(filter);
}

sk_colorfilter_t* sk_colorfilter_new_table_argb(const uint8_t tableA[256], const uint8_t tableR[256], const uint8_t tableG[256], const uint8_t tableB[256]) {
    SkColorFilter* filter = SkTableColorFilter::CreateARGB(tableA, tableR, tableG, tableB);
    return ToColorFilter(filter);
}
