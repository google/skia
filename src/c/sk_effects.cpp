/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "sk_types_priv.h"
#include "SkMatrix.h"

static void from_c_matrix(const sk_matrix_t* cmatrix, SkMatrix* matrix) {
    matrix->setAll(cmatrix->mat[0], cmatrix->mat[1], cmatrix->mat[2],
                   cmatrix->mat[3], cmatrix->mat[4], cmatrix->mat[5],
                   cmatrix->mat[6], cmatrix->mat[7], cmatrix->mat[8]);
}

#include "../../include/effects/SkGradientShader.h"
#include "sk_shader.h"

const struct {
    sk_shader_tilemode_t    fC;
    SkShader::TileMode      fSK;
} gTileModeMap[] = {
    { CLAMP_SK_SHADER_TILEMODE,     SkShader::kClamp_TileMode },
    { REPEAT_SK_SHADER_TILEMODE,    SkShader::kRepeat_TileMode },
    { MIRROR_SK_SHADER_TILEMODE,    SkShader::kMirror_TileMode  },
};

static bool from_c_tilemode(sk_shader_tilemode_t cMode, SkShader::TileMode* skMode) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(gTileModeMap); ++i) {
        if (cMode == gTileModeMap[i].fC) {
            if (skMode) {
                *skMode = gTileModeMap[i].fSK;
            }
            return true;
        }
    }
    return false;
}

void sk_shader_ref(sk_shader_t* cshader) {
    SkSafeRef(AsShader(cshader));
}

void sk_shader_unref(sk_shader_t* cshader) {
    SkSafeUnref(AsShader(cshader));
}

sk_shader_t* sk_shader_new_linear_gradient(const sk_point_t pts[2],
                                           const sk_color_t colors[],
                                           const float colorPos[],
                                           int colorCount,
                                           sk_shader_tilemode_t cmode,
                                           const sk_matrix_t* cmatrix) {
    SkShader::TileMode mode;
    if (!from_c_tilemode(cmode, &mode)) {
        return NULL;
    }
    SkMatrix matrix;
    if (cmatrix) {
        from_c_matrix(cmatrix, &matrix);
    } else {
        matrix.setIdentity();
    }
    return (sk_shader_t*)SkGradientShader::MakeLinear(reinterpret_cast<const SkPoint*>(pts),
                                                      reinterpret_cast<const SkColor*>(colors),
                                                      colorPos, colorCount,
                                                      mode, 0, &matrix).release();
}

static const SkPoint& to_skpoint(const sk_point_t& p) {
    return reinterpret_cast<const SkPoint&>(p);
}

sk_shader_t* sk_shader_new_radial_gradient(const sk_point_t* ccenter,
                                           float radius,
                                           const sk_color_t colors[],
                                           const float colorPos[],
                                           int colorCount,
                                           sk_shader_tilemode_t cmode,
                                           const sk_matrix_t* cmatrix) {
    SkShader::TileMode mode;
    if (!from_c_tilemode(cmode, &mode)) {
        return NULL;
    }
    SkMatrix matrix;
    if (cmatrix) {
        from_c_matrix(cmatrix, &matrix);
    } else {
        matrix.setIdentity();
    }
    SkPoint center = to_skpoint(*ccenter);
    return (sk_shader_t*)SkGradientShader::MakeRadial(center, (SkScalar)radius,
                                                      reinterpret_cast<const SkColor*>(colors),
                                                      reinterpret_cast<const SkScalar*>(colorPos),
                                                      colorCount, mode, 0, &matrix).release();
}

sk_shader_t* sk_shader_new_sweep_gradient(const sk_point_t* ccenter,
                                          const sk_color_t colors[],
                                          const float colorPos[],
                                          int colorCount,
                                          const sk_matrix_t* cmatrix) {
    SkMatrix matrix;
    if (cmatrix) {
        from_c_matrix(cmatrix, &matrix);
    } else {
        matrix.setIdentity();
    }
    return (sk_shader_t*)SkGradientShader::MakeSweep((SkScalar)(ccenter->x),
                                                     (SkScalar)(ccenter->y),
                                                     reinterpret_cast<const SkColor*>(colors),
                                                     reinterpret_cast<const SkScalar*>(colorPos),
                                                     colorCount, 0, &matrix).release();
}

sk_shader_t* sk_shader_new_two_point_conical_gradient(const sk_point_t* start,
                                                      float startRadius,
                                                      const sk_point_t* end,
                                                      float endRadius,
                                                      const sk_color_t colors[],
                                                      const float colorPos[],
                                                      int colorCount,
                                                      sk_shader_tilemode_t cmode,
                                                      const sk_matrix_t* cmatrix) {
    SkShader::TileMode mode;
    if (!from_c_tilemode(cmode, &mode)) {
        return NULL;
    }
    SkMatrix matrix;
    if (cmatrix) {
        from_c_matrix(cmatrix, &matrix);
    } else {
        matrix.setIdentity();
    }
    SkPoint skstart = to_skpoint(*start);
    SkPoint skend = to_skpoint(*end);
    return (sk_shader_t*)SkGradientShader::MakeTwoPointConical(skstart, (SkScalar)startRadius,
                                                        skend, (SkScalar)endRadius,
                                                        reinterpret_cast<const SkColor*>(colors),
                                                        reinterpret_cast<const SkScalar*>(colorPos),
                                                        colorCount, mode, 0, &matrix).release();
}

///////////////////////////////////////////////////////////////////////////////////////////

#include "../../include/effects/SkBlurMaskFilter.h"
#include "sk_maskfilter.h"

const struct {
    sk_blurstyle_t  fC;
    SkBlurStyle     fSk;
} gBlurStylePairs[] = {
    { NORMAL_SK_BLUR_STYLE, kNormal_SkBlurStyle },
    { SOLID_SK_BLUR_STYLE,  kSolid_SkBlurStyle },
    { OUTER_SK_BLUR_STYLE,  kOuter_SkBlurStyle },
    { INNER_SK_BLUR_STYLE,  kInner_SkBlurStyle },
};

static bool find_blurstyle(sk_blurstyle_t csrc, SkBlurStyle* dst) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(gBlurStylePairs); ++i) {
        if (gBlurStylePairs[i].fC == csrc) {
            if (dst) {
                *dst = gBlurStylePairs[i].fSk;
            }
            return true;
        }
    }
    return false;
}

void sk_maskfilter_ref(sk_maskfilter_t* cfilter) {
    SkSafeRef(AsMaskFilter(cfilter));
}

void sk_maskfilter_unref(sk_maskfilter_t* cfilter) {
    SkSafeUnref(AsMaskFilter(cfilter));
}

sk_maskfilter_t* sk_maskfilter_new_blur(sk_blurstyle_t cstyle, float sigma) {
    SkBlurStyle style;
    if (!find_blurstyle(cstyle, &style)) {
        return NULL;
    }
    return ToMaskFilter(SkBlurMaskFilter::Make(style, sigma).release());
}
