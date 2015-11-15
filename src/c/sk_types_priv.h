/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_types_priv_DEFINED
#define sk_types_priv_DEFINED

#include "sk_types.h"
#include "SkCanvas.h"
class SkMaskFilter;
class SkPaint;
class SkShader;
class SkCanvas;

static inline const SkPaint& AsPaint(const sk_paint_t& cpaint) {
    return reinterpret_cast<const SkPaint&>(cpaint);
}

static inline const SkPaint* AsPaint(const sk_paint_t* cpaint) {
    return reinterpret_cast<const SkPaint*>(cpaint);
}

static inline SkPaint* AsPaint(sk_paint_t* cpaint) {
    return reinterpret_cast<SkPaint*>(cpaint);
}

static inline SkMaskFilter* AsMaskFilter(sk_maskfilter_t* cfilter) {
    return reinterpret_cast<SkMaskFilter*>(cfilter);
}

static inline sk_maskfilter_t* ToMaskFilter(SkMaskFilter* filter) {
    return reinterpret_cast<sk_maskfilter_t*>(filter);
}

static inline SkShader* AsShader(sk_shader_t* cshader) {
    return reinterpret_cast<SkShader*>(cshader);
}

static inline SkXfermode::Mode MapXferMode(sk_xfermode_mode_t mode)
{
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
		return SkXfermode::kSrc_Mode;
    }
    return skmode;
}

static inline SkCanvas::PointMode MapPointMode(sk_point_mode_t mode)
{
    SkCanvas::PointMode pointMode;
    switch (mode) {
        #define MAP(X, Y) case (X): pointMode = (Y); break
	    MAP( POINTS_SK_POINT_MODE,  SkCanvas::kPoints_PointMode );
	    MAP( LINES_SK_POINT_MODE,   SkCanvas::kLines_PointMode );
	    MAP( POLYGON_SK_POINT_MODE, SkCanvas::kPolygon_PointMode );
        #undef MAP
        default:
		return SkCanvas::kPoints_PointMode;
    }
    return pointMode;
}


#endif
