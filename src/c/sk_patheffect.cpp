/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPathEffect.h"
#include "SkDiscretePathEffect.h"
#include "SkCornerPathEffect.h"
#include "Sk1DPathEffect.h"
#include "Sk2DPathEffect.h"
#include "SkDashPathEffect.h"
#include "SkArcToPathEffect.h"
#include "SkPath.h"

#include "sk_patheffect.h"

#include "sk_types_priv.h"

void sk_path_effect_unref(sk_path_effect_t* effect)
{
    SkSafeUnref(AsPathEffect(effect));
} 

sk_path_effect_t* sk_path_effect_create_compose(sk_path_effect_t* outer, sk_path_effect_t* inner)
{
    return ToPathEffect(SkPathEffect::MakeCompose(sk_ref_sp(AsPathEffect(outer)), sk_ref_sp(AsPathEffect(inner))).release());
}

sk_path_effect_t* sk_path_effect_create_sum(sk_path_effect_t* first, sk_path_effect_t* second)
{
    return ToPathEffect(SkPathEffect::MakeSum(sk_ref_sp(AsPathEffect(first)), sk_ref_sp(AsPathEffect(second))).release());
}

sk_path_effect_t* sk_path_effect_create_discrete(float segLength, float deviation, uint32_t seedAssist /*0*/)
{
    return ToPathEffect(SkDiscretePathEffect::Make(segLength, deviation, seedAssist).release());
}

sk_path_effect_t* sk_path_effect_create_corner(float radius)
{
    return ToPathEffect(SkCornerPathEffect::Make(radius).release());
}

sk_path_effect_t* sk_path_effect_create_arc_to(float radius)
{
    return ToPathEffect(SkArcToPathEffect::Make(radius).release());
}

sk_path_effect_t* sk_path_effect_create_1d_path(const sk_path_t* path, float advance, float phase, sk_path_effect_1d_style_t style)
{
    return ToPathEffect(SkPath1DPathEffect::Make(AsPath(*path), advance, phase, (SkPath1DPathEffect::Style)style).release());
}

sk_path_effect_t* sk_path_effect_create_2d_line(float width, const sk_matrix_t* cmatrix)
{
    SkMatrix matrix;
    if (cmatrix) {
        from_c(cmatrix, &matrix);
    }
    return ToPathEffect(SkLine2DPathEffect::Make(width, matrix).release());
}

sk_path_effect_t* sk_path_effect_create_2d_path(const sk_matrix_t* cmatrix, const sk_path_t* path)
{
    SkMatrix matrix;
    if (cmatrix) {
        from_c(cmatrix, &matrix);
    }
    return ToPathEffect(SkPath2DPathEffect::Make(matrix, AsPath(*path)).release());
}

sk_path_effect_t* sk_path_effect_create_dash(const float intervals[], int count, float phase)
{
    return ToPathEffect(SkDashPathEffect::Make(intervals, count, phase).release());
}
