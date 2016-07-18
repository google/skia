/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPathEffect.h"
#include "../../include/effects/SkDiscretePathEffect.h"
#include "../../include/effects/SkCornerPathEffect.h"
#include "../../include/effects/Sk1DPathEffect.h"
#include "../../include/effects/Sk2DPathEffect.h"
#include "../../include/effects/SkDashPathEffect.h"
#include "SkPath.h"

#include "xamarin/sk_x_patheffect.h"

#include "../sk_types_priv.h"
#include "sk_x_types_priv.h"

void sk_path_effect_unref(sk_path_effect_t* effect)
{
    SkSafeUnref(AsPathEffect(effect));
} 

sk_path_effect_t* sk_path_effect_create_compose(sk_path_effect_t* outer, sk_path_effect_t* inner)
{
    return ToPathEffect(SkComposePathEffect::Create(AsPathEffect(outer), AsPathEffect(inner)));
}

sk_path_effect_t* sk_path_effect_create_sum(sk_path_effect_t* first, sk_path_effect_t* second)
{
    return ToPathEffect(SkSumPathEffect::Create(AsPathEffect(first), AsPathEffect(second)));
}

sk_path_effect_t* sk_path_effect_create_discrete(float segLength, float deviation, uint32_t seedAssist /*0*/)
{
    return ToPathEffect(SkDiscretePathEffect::Create(segLength, deviation, seedAssist));
}

sk_path_effect_t* sk_path_effect_create_corner(float radius)
{
    return ToPathEffect(SkCornerPathEffect::Create(radius));
}

sk_path_effect_t* sk_path_effect_create_1d_path(const sk_path_t* path, float advance, float phase, sk_path_effect_1d_style_t style)
{
    return ToPathEffect(SkPath1DPathEffect::Create(AsPath(*path), advance, phase, find_sk_default(style, SkPath1DPathEffect::kTranslate_Style)));
}

sk_path_effect_t* sk_path_effect_create_2d_line(float width, const sk_matrix_t* cmatrix)
{
    SkMatrix matrix;
    if (cmatrix) {
        from_c(cmatrix, &matrix);
    }
    return ToPathEffect(SkLine2DPathEffect::Create(width, matrix));
}

sk_path_effect_t* sk_path_effect_create_2d_path(const sk_matrix_t* cmatrix, const sk_path_t* path)
{
    SkMatrix matrix;
    if (cmatrix) {
        from_c(cmatrix, &matrix);
    }
    return ToPathEffect(SkPath2DPathEffect::Create(matrix, AsPath(*path)));
}

sk_path_effect_t* sk_path_effect_create_dash(const float intervals[], int count, float phase)
{
    return ToPathEffect(SkDashPathEffect::Create(intervals, count, phase));
}
