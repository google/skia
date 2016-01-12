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

struct SkRect;
class SkData;
class SkMaskFilter;
class SkPaint;
class SkImage;
class SkShader;
class SkCanvas;
class SkStream;
class SkFILEStream;
class SkMemoryStream;
class SkPictureRecorder;

static inline SkData* AsData(const sk_data_t* cdata) {
    return reinterpret_cast<SkData*>(const_cast<sk_data_t*>(cdata));
}

static inline sk_data_t* ToData(SkData* data) {
    return reinterpret_cast<sk_data_t*>(data);
}

static inline sk_rect_t ToRect(const SkRect& rect) {
    return reinterpret_cast<const sk_rect_t&>(rect);
}

static inline const SkRect* AsRect(const sk_rect_t* crect) {
    return reinterpret_cast<const SkRect*>(crect);
}

static inline const SkRect& AsRect(const sk_rect_t& crect) {
    return reinterpret_cast<const SkRect&>(crect);
}

static inline const SkPath& AsPath(const sk_path_t& cpath) {
    return reinterpret_cast<const SkPath&>(cpath);
}

static inline SkPath* as_path(sk_path_t* cpath) {
    return reinterpret_cast<SkPath*>(cpath);
}

static inline const SkImage* AsImage(const sk_image_t* cimage) {
    return reinterpret_cast<const SkImage*>(cimage);
}

static inline sk_image_t* ToImage(SkImage* cimage) {
    return reinterpret_cast<sk_image_t*>(cimage);
}

static inline sk_canvas_t* ToCanvas(SkCanvas* canvas) {
    return reinterpret_cast<sk_canvas_t*>(canvas);
}

static inline SkCanvas* AsCanvas(sk_canvas_t* ccanvas) {
    return reinterpret_cast<SkCanvas*>(ccanvas);
}

static inline SkPictureRecorder* AsPictureRecorder(sk_picture_recorder_t* crec) {
    return reinterpret_cast<SkPictureRecorder*>(crec);
}

static inline sk_picture_recorder_t* ToPictureRecorder(SkPictureRecorder* rec) {
    return reinterpret_cast<sk_picture_recorder_t*>(rec);
}

static inline const SkPicture* AsPicture(const sk_picture_t* cpic) {
    return reinterpret_cast<const SkPicture*>(cpic);
}

static inline SkPicture* AsPicture(sk_picture_t* cpic) {
    return reinterpret_cast<SkPicture*>(cpic);
}

static inline sk_picture_t* ToPicture(SkPicture* pic) {
    return reinterpret_cast<sk_picture_t*>(pic);
}

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

static inline const SkFILEStream* AsFileStream(const sk_stream_filestream_t* cfilestream) {
    return reinterpret_cast<const SkFILEStream*>(cfilestream);
}

static inline SkFILEStream* AsFileStream(sk_stream_filestream_t* cfilestream) {
    return reinterpret_cast<SkFILEStream*>(cfilestream);
}

static inline const SkMemoryStream* AsMemoryStream(const sk_stream_memorystream_t* cmemorystream) {
    return reinterpret_cast<const SkMemoryStream*>(cmemorystream);
}

static inline SkMemoryStream* AsMemoryStream(sk_stream_memorystream_t* cmemorystream) {
    return reinterpret_cast<SkMemoryStream*>(cmemorystream);
}

static inline const SkStream* AsStream(const sk_stream_t* cstream) {
    return reinterpret_cast<const SkStream*>(cstream);
}

static inline SkStream* AsStream(sk_stream_t* cstream) {
    return reinterpret_cast<SkStream*>(cstream);
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
