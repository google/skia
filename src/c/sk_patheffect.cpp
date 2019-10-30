/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPathEffect.h"
#include "include/core/SkPath.h"
#include "include/effects/SkDiscretePathEffect.h"
#include "include/effects/SkCornerPathEffect.h"
#include "include/effects/Sk1DPathEffect.h"
#include "include/effects/Sk2DPathEffect.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/effects/SkTrimPathEffect.h"

#include "include/c/sk_patheffect.h"

#include "src/c/sk_types_priv.h"

void sk_path_effect_unref(sk_path_effect_t* effect) {
    SkSafeUnref(AsPathEffect(effect));
} 

sk_path_effect_t* sk_path_effect_create_compose(sk_path_effect_t* outer, sk_path_effect_t* inner) {
    return ToPathEffect(SkPathEffect::MakeCompose(sk_ref_sp(AsPathEffect(outer)), sk_ref_sp(AsPathEffect(inner))).release());
}

sk_path_effect_t* sk_path_effect_create_sum(sk_path_effect_t* first, sk_path_effect_t* second) {
    return ToPathEffect(SkPathEffect::MakeSum(sk_ref_sp(AsPathEffect(first)), sk_ref_sp(AsPathEffect(second))).release());
}

sk_path_effect_t* sk_path_effect_create_discrete(float segLength, float deviation, uint32_t seedAssist) {
    return ToPathEffect(SkDiscretePathEffect::Make(segLength, deviation, seedAssist).release());
}

sk_path_effect_t* sk_path_effect_create_corner(float radius) {
    return ToPathEffect(SkCornerPathEffect::Make(radius).release());
}

sk_path_effect_t* sk_path_effect_create_1d_path(const sk_path_t* path, float advance, float phase, sk_path_effect_1d_style_t style) {
    return ToPathEffect(SkPath1DPathEffect::Make(*AsPath(path), advance, phase, (SkPath1DPathEffect::Style)style).release());
}

sk_path_effect_t* sk_path_effect_create_2d_line(float width, const sk_matrix_t* cmatrix) {
    return ToPathEffect(SkLine2DPathEffect::Make(width, AsMatrix(cmatrix)).release());
}

sk_path_effect_t* sk_path_effect_create_2d_path(const sk_matrix_t* cmatrix, const sk_path_t* path) {
    return ToPathEffect(SkPath2DPathEffect::Make(AsMatrix(cmatrix), *AsPath(path)).release());
}

sk_path_effect_t* sk_path_effect_create_dash(const float intervals[], int count, float phase) {
    return ToPathEffect(SkDashPathEffect::Make(intervals, count, phase).release());
}

sk_path_effect_t* sk_path_effect_create_trim(float start, float stop, sk_path_effect_trim_mode_t mode) {
    return ToPathEffect(SkTrimPathEffect::Make(start, stop, (SkTrimPathEffect::Mode)mode).release());
}
