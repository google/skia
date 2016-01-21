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
#include "SkImageDecoder.h"
#include "SkPath.h"
#include "SkPaint.h"
#include "SkData.h"
#include "SkImage.h"
#include "SkMaskFilter.h"
#include "SkMatrix.h"
#include "SkPictureRecorder.h"
#include "SkSurface.h"
#include "../../include/effects/SkGradientShader.h"
#include "../../include/effects/SkBlurMaskFilter.h"

class SkFILEStream;
class SkMemoryStream;

#define MAKE_FROM_TO_NAME(FROM)     g_ ## FROM ## _map

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

static inline const SkIRect* AsIRect(const sk_irect_t* crect) {
    return reinterpret_cast<const SkIRect*>(crect);
}

static inline const SkIRect& AsIRect(const sk_irect_t& crect) {
    return reinterpret_cast<const SkIRect&>(crect);
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

static inline const SkBitmap* AsBitmap(const sk_bitmap_t* cbitmap) {
    return reinterpret_cast<const SkBitmap*>(cbitmap);
}

static inline const SkBitmap& AsBitmap(const sk_bitmap_t& cbitmap) {
    return reinterpret_cast<const SkBitmap&>(cbitmap);
}

static inline SkBitmap* AsBitmap(sk_bitmap_t* cbitmap) {
    return reinterpret_cast<SkBitmap*>(cbitmap);
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

static inline SkStreamRewindable* AsStreamRewindable(sk_stream_streamrewindable_t* cstreamrewindable) {
    return reinterpret_cast<SkStreamRewindable*>(cstreamrewindable);
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

const struct {
    sk_imagedecoder_mode_t  fC;
    SkImageDecoder::Mode    fSK;
} MAKE_FROM_TO_NAME(sk_imagedecoder_mode_t)[] = {
    { DECODEBOUNDS_SK_IMAGEDECODER_MODE, SkImageDecoder::kDecodeBounds_Mode },
    { DECODEPIXELS_SK_IMAGEDECODER_MODE, SkImageDecoder::kDecodePixels_Mode },
};
#define CType           sk_imagedecoder_mode_t
#define SKType          SkImageDecoder::Mode
#define CTypeSkTypeMap  MAKE_FROM_TO_NAME(sk_imagedecoder_mode_t)
#include "sk_c_from_to.h"

const struct {
    sk_imagedecoder_result_t  fC;
    SkImageDecoder::Result    fSK;
} MAKE_FROM_TO_NAME(sk_imagedecoder_result_t)[] = {
    { FAILURE_SK_IMAGEDECODER_RESULT,        SkImageDecoder::kFailure        },
    { PARTIALSUCCESS_SK_IMAGEDECODER_RESULT, SkImageDecoder::kPartialSuccess },
    { SUCCESS_SK_IMAGEDECODER_RESULT,        SkImageDecoder::kSuccess        },
};
#define CType           sk_imagedecoder_result_t
#define SKType          SkImageDecoder::Result
#define CTypeSkTypeMap  MAKE_FROM_TO_NAME(sk_imagedecoder_result_t)
#include "sk_c_from_to.h"

const struct {
    sk_imagedecoder_format_t  fC;
    SkImageDecoder::Format    fSK;
} MAKE_FROM_TO_NAME(sk_imagedecoder_format_t)[] = {
    { UNKNOWN_SK_IMAGEDECODER_FORMAT, SkImageDecoder::kUnknown_Format },
    { BMP_SK_IMAGEDECODER_FORMAT,     SkImageDecoder::kBMP_Format     },
    { GIF_SK_IMAGEDECODER_FORMAT,     SkImageDecoder::kGIF_Format     },
    { ICO_SK_IMAGEDECODER_FORMAT,     SkImageDecoder::kICO_Format     },
    { JPEG_SK_IMAGEDECODER_FORMAT,    SkImageDecoder::kJPEG_Format    },
    { PNG_SK_IMAGEDECODER_FORMAT,     SkImageDecoder::kPNG_Format     },
    { WBMP_SK_IMAGEDECODER_FORMAT,    SkImageDecoder::kWBMP_Format    },
    { WEBP_SK_IMAGEDECODER_FORMAT,    SkImageDecoder::kWEBP_Format    },
    { PKM_SK_IMAGEDECODER_FORMAT,     SkImageDecoder::kPKM_Format     },
    { KTX_SK_IMAGEDECODER_FORMAT,     SkImageDecoder::kKTX_Format     },
    { ASTC_SK_IMAGEDECODER_FORMAT,    SkImageDecoder::kASTC_Format    },
};
#define CType           sk_imagedecoder_format_t
#define SKType          SkImageDecoder::Format
#define CTypeSkTypeMap  MAKE_FROM_TO_NAME(sk_imagedecoder_format_t)
#include "sk_c_from_to.h"

const struct {
    sk_colortype_t  fC;
    SkColorType     fSK;
} MAKE_FROM_TO_NAME(sk_colortype_t)[] = {
    { UNKNOWN_SK_COLORTYPE,     kUnknown_SkColorType    },
    { RGBA_8888_SK_COLORTYPE,   kRGBA_8888_SkColorType  },
    { BGRA_8888_SK_COLORTYPE,   kBGRA_8888_SkColorType  },
    { ALPHA_8_SK_COLORTYPE,     kAlpha_8_SkColorType    },
    { RGB_565_SK_COLORTYPE,     kRGB_565_SkColorType    },
    { N_32_SK_COLORTYPE,        kN32_SkColorType        },
};
#define CType           sk_colortype_t
#define SKType          SkColorType
#define CTypeSkTypeMap  MAKE_FROM_TO_NAME(sk_colortype_t)
#include "sk_c_from_to.h"

const struct {
    sk_alphatype_t  fC;
    SkAlphaType     fSK;
} MAKE_FROM_TO_NAME(sk_alphatype_t)[] = {
    { OPAQUE_SK_ALPHATYPE,      kOpaque_SkAlphaType     },
    { PREMUL_SK_ALPHATYPE,      kPremul_SkAlphaType     },
    { UNPREMUL_SK_ALPHATYPE,    kUnpremul_SkAlphaType   },
};
#define CType           sk_alphatype_t
#define SKType          SkAlphaType
#define CTypeSkTypeMap  MAKE_FROM_TO_NAME(sk_alphatype_t)
#include "sk_c_from_to.h"

const struct {
    sk_pixelgeometry_t fC;
    SkPixelGeometry    fSK;
} MAKE_FROM_TO_NAME(sk_pixelgeometry_t)[] = {
    { UNKNOWN_SK_PIXELGEOMETRY, kUnknown_SkPixelGeometry },
    { RGB_H_SK_PIXELGEOMETRY,   kRGB_H_SkPixelGeometry   },
    { BGR_H_SK_PIXELGEOMETRY,   kBGR_H_SkPixelGeometry   },
    { RGB_V_SK_PIXELGEOMETRY,   kRGB_V_SkPixelGeometry   },
    { BGR_V_SK_PIXELGEOMETRY,   kBGR_V_SkPixelGeometry   },
};
#define CType           sk_pixelgeometry_t
#define SKType          SkPixelGeometry
#define CTypeSkTypeMap  MAKE_FROM_TO_NAME(sk_pixelgeometry_t)
#include "sk_c_from_to.h"

const struct {
    sk_path_direction_t fC;
    SkPath::Direction   fSK;
} MAKE_FROM_TO_NAME(sk_path_direction_t)[] = {
    { CW_SK_PATH_DIRECTION,  SkPath::kCW_Direction },
    { CCW_SK_PATH_DIRECTION, SkPath::kCCW_Direction },
};
#define CType           sk_path_direction_t
#define SKType          SkPath::Direction
#define CTypeSkTypeMap  MAKE_FROM_TO_NAME(sk_path_direction_t)
#include "sk_c_from_to.h"

const struct {
    sk_blurstyle_t  fC;
    SkBlurStyle     fSK;
} MAKE_FROM_TO_NAME(sk_blurstyle_t)[] = {
    { NORMAL_SK_BLUR_STYLE, kNormal_SkBlurStyle },
    { SOLID_SK_BLUR_STYLE,  kSolid_SkBlurStyle },
    { OUTER_SK_BLUR_STYLE,  kOuter_SkBlurStyle },
    { INNER_SK_BLUR_STYLE,  kInner_SkBlurStyle },
};
#define CType           sk_blurstyle_t
#define SKType          SkBlurStyle
#define CTypeSkTypeMap  MAKE_FROM_TO_NAME(sk_blurstyle_t)
#include "sk_c_from_to.h"

const struct {
    sk_shader_tilemode_t    fC;
    SkShader::TileMode      fSK;
} MAKE_FROM_TO_NAME(sk_shader_tilemode_t)[] = {
    { CLAMP_SK_SHADER_TILEMODE,     SkShader::kClamp_TileMode },
    { REPEAT_SK_SHADER_TILEMODE,    SkShader::kRepeat_TileMode },
    { MIRROR_SK_SHADER_TILEMODE,    SkShader::kMirror_TileMode  },
};
#define CType           sk_shader_tilemode_t
#define SKType          SkShader::TileMode
#define CTypeSkTypeMap  MAKE_FROM_TO_NAME(sk_shader_tilemode_t)
#include "sk_c_from_to.h"

const struct {
    sk_stroke_cap_t fC;
    SkPaint::Cap    fSK;
} MAKE_FROM_TO_NAME(sk_stroke_cap_t)[] = {
    { BUTT_SK_STROKE_CAP,   SkPaint::kButt_Cap   },
    { ROUND_SK_STROKE_CAP,  SkPaint::kRound_Cap  },
    { SQUARE_SK_STROKE_CAP, SkPaint::kSquare_Cap },
};
#define CType           sk_stroke_cap_t
#define SKType          SkPaint::Cap
#define CTypeSkTypeMap  MAKE_FROM_TO_NAME(sk_stroke_cap_t)
#include "sk_c_from_to.h"

const struct {
    sk_stroke_join_t fC;
    SkPaint::Join    fSK;
} MAKE_FROM_TO_NAME(sk_stroke_join_t)[] = {
    { MITER_SK_STROKE_JOIN, SkPaint::kMiter_Join },
    { ROUND_SK_STROKE_JOIN, SkPaint::kRound_Join },
    { BEVEL_SK_STROKE_JOIN, SkPaint::kBevel_Join },
};
#define CType           sk_stroke_join_t
#define SKType          SkPaint::Join
#define CTypeSkTypeMap  MAKE_FROM_TO_NAME(sk_stroke_join_t)
#include "sk_c_from_to.h"

const struct {
    sk_text_align_t fC;
    SkPaint::Align  fSK;
} MAKE_FROM_TO_NAME(sk_text_align_t)[] = {
    { LEFT_SK_TEXT_ALIGN, SkPaint::kLeft_Align },
    { CENTER_SK_TEXT_ALIGN, SkPaint::kCenter_Align },
    { RIGHT_SK_TEXT_ALIGN, SkPaint::kRight_Align },
};
#define CType           sk_text_align_t
#define SKType          SkPaint::Align
#define CTypeSkTypeMap  MAKE_FROM_TO_NAME(sk_text_align_t)
#include "sk_c_from_to.h"

const struct {
    sk_text_encoding_t    fC;
    SkPaint::TextEncoding fSK;
} MAKE_FROM_TO_NAME(sk_text_encoding_t)[] = {
    { UTF8_SK_TEXT_ENCODING, SkPaint::kUTF8_TextEncoding },
    { UTF16_SK_TEXT_ENCODING, SkPaint::kUTF16_TextEncoding },
    { UTF32_SK_TEXT_ENCODING, SkPaint::kUTF32_TextEncoding },
    { GLYPH_ID_SK_TEXT_ENCODING, SkPaint::kGlyphID_TextEncoding },
};
#define CType           sk_text_encoding_t
#define SKType          SkPaint::TextEncoding
#define CTypeSkTypeMap  MAKE_FROM_TO_NAME(sk_text_encoding_t)
#include "sk_c_from_to.h"

static inline bool find_sk(const sk_imageinfo_t& cinfo, SkImageInfo* info) {
    SkColorType ct;
    SkAlphaType at;

    if (!find_sk(cinfo.colorType, &ct)) {
        // optionally report error to client?
        return false;
    }
    if (!find_sk(cinfo.alphaType, &at)) {
        // optionally report error to client?
        return false;
    }
    if (info) {
        *info = SkImageInfo::Make(cinfo.width, cinfo.height, ct, at);
    }
    return true;
}

static inline bool find_c(const SkImageInfo& info, sk_imageinfo_t* cinfo)
{
    sk_imageinfo_t tempInfo;
    tempInfo.width = info.width();
    tempInfo.height = info.height();
    if (!find_c(info.colorType(), &tempInfo.colorType)) {
        return false;
    }
    if (find_c(info.alphaType(), &tempInfo.alphaType)) {
        return false;
    }
    if (cinfo) {
        *cinfo = tempInfo;
    }
    return true;
}

#endif
