/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPaint.h"

#include "sk_paint.h"
#include "sk_types_priv.h"

#define MAKE_FROM_TO_NAME(FROM)     g_ ## FROM ## _map

const struct {
    sk_stroke_cap_t fC;
    SkPaint::Cap    fSK;
} MAKE_FROM_TO_NAME(sk_stroke_cap_t)[] = {
    { BUTT_SK_STROKE_CAP,   SkPaint::kButt_Cap   },
    { ROUND_SK_STROKE_CAP,  SkPaint::kRound_Cap  },
    { SQUARE_SK_STROKE_CAP, SkPaint::kSquare_Cap },
};

const struct {
    sk_stroke_join_t fC;
    SkPaint::Join    fSK;
} MAKE_FROM_TO_NAME(sk_stroke_join_t)[] = {
    { MITER_SK_STROKE_JOIN, SkPaint::kMiter_Join },
    { ROUND_SK_STROKE_JOIN, SkPaint::kRound_Join },
    { BEVEL_SK_STROKE_JOIN, SkPaint::kBevel_Join },
};

#define CType           sk_stroke_cap_t
#define SKType          SkPaint::Cap
#define CTypeSkTypeMap  MAKE_FROM_TO_NAME(sk_stroke_cap_t)
#include "sk_c_from_to.h"

#define CType           sk_stroke_join_t
#define SKType          SkPaint::Join
#define CTypeSkTypeMap  MAKE_FROM_TO_NAME(sk_stroke_join_t)
#include "sk_c_from_to.h"

//////////////////////////////////////////////////////////////////////////////////////////////////

sk_paint_t* sk_paint_new() { return (sk_paint_t*)new SkPaint; }

void sk_paint_delete(sk_paint_t* cpaint) { delete AsPaint(cpaint); }

bool sk_paint_is_antialias(const sk_paint_t* cpaint) {
    return AsPaint(*cpaint).isAntiAlias();
}

void sk_paint_set_antialias(sk_paint_t* cpaint, bool aa) {
    AsPaint(cpaint)->setAntiAlias(aa);
}

sk_color_t sk_paint_get_color(const sk_paint_t* cpaint) {
    return AsPaint(*cpaint).getColor();
}

void sk_paint_set_color(sk_paint_t* cpaint, sk_color_t c) {
    AsPaint(cpaint)->setColor(c);
}

void sk_paint_set_shader(sk_paint_t* cpaint, sk_shader_t* cshader) {
    AsPaint(cpaint)->setShader(AsShader(cshader));
}

void sk_paint_set_maskfilter(sk_paint_t* cpaint, sk_maskfilter_t* cfilter) {
    AsPaint(cpaint)->setMaskFilter(AsMaskFilter(cfilter));
}

bool sk_paint_is_stroke(const sk_paint_t* cpaint) {
    return AsPaint(*cpaint).getStyle() != SkPaint::kFill_Style;
}

void sk_paint_set_stroke(sk_paint_t* cpaint, bool doStroke) {
    AsPaint(cpaint)->setStyle(doStroke ? SkPaint::kStroke_Style : SkPaint::kFill_Style);
}

float sk_paint_get_stroke_width(const sk_paint_t* cpaint) {
    return AsPaint(*cpaint).getStrokeWidth();
}

void sk_paint_set_stroke_width(sk_paint_t* cpaint, float width) {
    AsPaint(cpaint)->setStrokeWidth(width);
}

float sk_paint_get_stroke_miter(const sk_paint_t* cpaint) {
    return AsPaint(*cpaint).getStrokeMiter();
}

void sk_paint_set_stroke_miter(sk_paint_t* cpaint, float miter) {
    AsPaint(cpaint)->setStrokeMiter(miter);
}

sk_stroke_cap_t sk_paint_get_stroke_cap(const sk_paint_t* cpaint) {
    sk_stroke_cap_t ccap;
    if (find_c(AsPaint(*cpaint).getStrokeCap(), &ccap)) {
        ccap = BUTT_SK_STROKE_CAP;
    }
    return ccap;
}

void sk_paint_set_stroke_cap(sk_paint_t* cpaint, sk_stroke_cap_t ccap) {
    SkPaint::Cap skcap;
    if (find_sk(ccap, &skcap)) {
        AsPaint(cpaint)->setStrokeCap(skcap);
    } else {
        // unknown ccap
    }
}

sk_stroke_join_t sk_paint_get_stroke_join(const sk_paint_t* cpaint) {
    sk_stroke_join_t cjoin;
    if (find_c(AsPaint(*cpaint).getStrokeJoin(), &cjoin)) {
        cjoin = MITER_SK_STROKE_JOIN;
    }
    return cjoin;
}

void sk_paint_set_stroke_join(sk_paint_t* cpaint, sk_stroke_join_t cjoin) {
    SkPaint::Join skjoin;
    if (find_sk(cjoin, &skjoin)) {
        AsPaint(cpaint)->setStrokeJoin(skjoin);
    } else {
        // unknown cjoin
    }
}

void sk_paint_set_xfermode_mode(sk_paint_t* paint, sk_xfermode_mode_t mode) {
    SkASSERT(paint);
    SkXfermode::Mode skmode;
    switch (mode) {
        #define MAP(X, Y) case (X): skmode = (Y); break
        MAP( CLEAR_SK_XFERMODE_MODE,      SkXfermode::kClear_Mode      );
        MAP( SRC_SK_XFERMODE_MODE,        SkXfermode::kSrc_Mode        );
        MAP( DST_SK_XFERMODE_MODE,        SkXfermode::kDst_Mode        );
        MAP( SRCOVER_SK_XFERMODE_MODE,    SkXfermode::kSrcOver_Mode    );
        MAP( DSTOVER_SK_XFERMODE_MODE,    SkXfermode::kDstOver_Mode    );
        MAP( SRCIN_SK_XFERMODE_MODE,      SkXfermode::kSrcIn_Mode      );
        MAP( DSTIN_SK_XFERMODE_MODE,      SkXfermode::kDstIn_Mode      );
        MAP( SRCOUT_SK_XFERMODE_MODE,     SkXfermode::kSrcOut_Mode     );
        MAP( DSTOUT_SK_XFERMODE_MODE,     SkXfermode::kDstOut_Mode     );
        MAP( SRCATOP_SK_XFERMODE_MODE,    SkXfermode::kSrcATop_Mode    );
        MAP( DSTATOP_SK_XFERMODE_MODE,    SkXfermode::kDstATop_Mode    );
        MAP( XOR_SK_XFERMODE_MODE,        SkXfermode::kXor_Mode        );
        MAP( PLUS_SK_XFERMODE_MODE,       SkXfermode::kPlus_Mode       );
        MAP( MODULATE_SK_XFERMODE_MODE,   SkXfermode::kModulate_Mode   );
        MAP( SCREEN_SK_XFERMODE_MODE,     SkXfermode::kScreen_Mode     );
        MAP( OVERLAY_SK_XFERMODE_MODE,    SkXfermode::kOverlay_Mode    );
        MAP( DARKEN_SK_XFERMODE_MODE,     SkXfermode::kDarken_Mode     );
        MAP( LIGHTEN_SK_XFERMODE_MODE,    SkXfermode::kLighten_Mode    );
        MAP( COLORDODGE_SK_XFERMODE_MODE, SkXfermode::kColorDodge_Mode );
        MAP( COLORBURN_SK_XFERMODE_MODE,  SkXfermode::kColorBurn_Mode  );
        MAP( HARDLIGHT_SK_XFERMODE_MODE,  SkXfermode::kHardLight_Mode  );
        MAP( SOFTLIGHT_SK_XFERMODE_MODE,  SkXfermode::kSoftLight_Mode  );
        MAP( DIFFERENCE_SK_XFERMODE_MODE, SkXfermode::kDifference_Mode );
        MAP( EXCLUSION_SK_XFERMODE_MODE,  SkXfermode::kExclusion_Mode  );
        MAP( MULTIPLY_SK_XFERMODE_MODE,   SkXfermode::kMultiply_Mode   );
        MAP( HUE_SK_XFERMODE_MODE,        SkXfermode::kHue_Mode        );
        MAP( SATURATION_SK_XFERMODE_MODE, SkXfermode::kSaturation_Mode );
        MAP( COLOR_SK_XFERMODE_MODE,      SkXfermode::kColor_Mode      );
        MAP( LUMINOSITY_SK_XFERMODE_MODE, SkXfermode::kLuminosity_Mode );
        #undef MAP
        default:
            return;
    }
    AsPaint(paint)->setXfermodeMode(skmode);
}
