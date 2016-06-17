/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_x_types_priv_DEFINED
#define sk_x_types_priv_DEFINED

#include "SkImageInfo.h"
#include "SkBlurTypes.h"
#include "SkDocument.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkCodec.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"
#include "SkPoint3.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkDisplacementMapEffect.h"
#include "SkDropShadowImageFilter.h"
#include "SkMatrixConvolutionImageFilter.h"

#include "sk_path.h"
#include "sk_paint.h"
#include "sk_shader.h"
#include "sk_maskfilter.h"

#include "sk_types.h"
#include "xamarin/sk_x_types.h"

#include "../sk_types_priv.h"

#define MAKE_FROM_TO_NAME(FROM)     g_ ## FROM ## _map

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
#include "../sk_c_from_to.h"

const struct {
    sk_path_filltype_t fC;
    SkPath::FillType   fSK;
} MAKE_FROM_TO_NAME(sk_path_filltype_t)[] = {
    { WINDING_SK_PATH_FILLTYPE,  SkPath::kWinding_FillType },
    { EVENODD_SK_PATH_FILLTYPE, SkPath::kEvenOdd_FillType },
    { INVERSE_WINDING_SK_PATH_FILLTYPE,  SkPath::kInverseWinding_FillType },
    { INVERSE_EVENODD_SK_PATH_FILLTYPE, SkPath::kInverseEvenOdd_FillType },
};
#define CType           sk_path_filltype_t
#define SKType          SkPath::FillType
#define CTypeSkTypeMap  MAKE_FROM_TO_NAME(sk_path_filltype_t)
#include "../sk_c_from_to.h"

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
#include "../sk_c_from_to.h"

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
#include "../sk_c_from_to.h"

const struct {
    sk_filter_quality_t fC;
    SkFilterQuality     fSK;
} MAKE_FROM_TO_NAME(sk_filter_quality_t)[] = {
    { NONE_SK_FILTER_QUALITY,   SkFilterQuality::kNone_SkFilterQuality   },
    { LOW_SK_FILTER_QUALITY,    SkFilterQuality::kLow_SkFilterQuality    },
    { MEDIUM_SK_FILTER_QUALITY, SkFilterQuality::kMedium_SkFilterQuality },
    { HIGH_SK_FILTER_QUALITY,   SkFilterQuality::kHigh_SkFilterQuality   },
};
#define CType           sk_filter_quality_t
#define SKType          SkFilterQuality
#define CTypeSkTypeMap  MAKE_FROM_TO_NAME(sk_filter_quality_t)
#include "../sk_c_from_to.h"

const struct {
    sk_crop_rect_flags_t              fC;
    SkImageFilter::CropRect::CropEdge fSK;
} MAKE_FROM_TO_NAME(sk_crop_rect_flags_t)[] = {
    { HAS_LEFT_SK_CROP_RECT_FLAG,   SkImageFilter::CropRect::kHasLeft_CropEdge   },
    { HAS_TOP_SK_CROP_RECT_FLAG,    SkImageFilter::CropRect::kHasTop_CropEdge    },
    { HAS_WIDTH_SK_CROP_RECT_FLAG,  SkImageFilter::CropRect::kHasWidth_CropEdge  },
    { HAS_HEIGHT_SK_CROP_RECT_FLAG, SkImageFilter::CropRect::kHasHeight_CropEdge },
    { HAS_ALL_SK_CROP_RECT_FLAG,    SkImageFilter::CropRect::kHasAll_CropEdge    },
};
#define CType           sk_crop_rect_flags_t
#define SKType          SkImageFilter::CropRect::CropEdge
#define CTypeSkTypeMap  MAKE_FROM_TO_NAME(sk_crop_rect_flags_t)
#include "../sk_c_from_to.h"

const struct {
    sk_drop_shadow_image_filter_shadow_mode_t fC;
    SkDropShadowImageFilter::ShadowMode       fSK;
} MAKE_FROM_TO_NAME(sk_drop_shadow_image_filter_shadow_mode_t)[] = {
    { DRAW_SHADOW_AND_FOREGROUND_SK_DROP_SHADOW_IMAGE_FILTER_SHADOW_MODE, SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode },
    { DRAW_SHADOW_ONLY_SK_DROP_SHADOW_IMAGE_FILTER_SHADOW_MODE,           SkDropShadowImageFilter::kDrawShadowOnly_ShadowMode          },
};
#define CType           sk_drop_shadow_image_filter_shadow_mode_t
#define SKType          SkDropShadowImageFilter::ShadowMode
#define CTypeSkTypeMap  MAKE_FROM_TO_NAME(sk_drop_shadow_image_filter_shadow_mode_t)
#include "../sk_c_from_to.h"

const struct {
    sk_displacement_map_effect_channel_selector_type_t fC;
    SkDisplacementMapEffect::ChannelSelectorType       fSK;
} MAKE_FROM_TO_NAME(sk_displacement_map_effect_channel_selector_type_t)[] = {
    { UNKNOWN_SK_DISPLACEMENT_MAP_EFFECT_CHANNEL_SELECTOR_TYPE, SkDisplacementMapEffect::kUnknown_ChannelSelectorType },
    { R_SK_DISPLACEMENT_MAP_EFFECT_CHANNEL_SELECTOR_TYPE,       SkDisplacementMapEffect::kR_ChannelSelectorType       },
    { G_SK_DISPLACEMENT_MAP_EFFECT_CHANNEL_SELECTOR_TYPE,       SkDisplacementMapEffect::kG_ChannelSelectorType       },
    { B_SK_DISPLACEMENT_MAP_EFFECT_CHANNEL_SELECTOR_TYPE,       SkDisplacementMapEffect::kB_ChannelSelectorType       },
    { A_SK_DISPLACEMENT_MAP_EFFECT_CHANNEL_SELECTOR_TYPE,       SkDisplacementMapEffect::kA_ChannelSelectorType       },
};
#define CType           sk_displacement_map_effect_channel_selector_type_t
#define SKType          SkDisplacementMapEffect::ChannelSelectorType
#define CTypeSkTypeMap  MAKE_FROM_TO_NAME(sk_displacement_map_effect_channel_selector_type_t)
#include "../sk_c_from_to.h"

const struct {
    sk_matrix_convolution_tilemode_t         fC;
    SkMatrixConvolutionImageFilter::TileMode fSK;
} MAKE_FROM_TO_NAME(sk_matrix_convolution_tilemode_t)[] = {
    { CLAMP_SK_MATRIX_CONVOLUTION_TILEMODE,          SkMatrixConvolutionImageFilter::kClamp_TileMode        },
    { REPEAT_SK_MATRIX_CONVOLUTION_TILEMODE,         SkMatrixConvolutionImageFilter::kRepeat_TileMode       },
    { CLAMP_TO_BLACK_SK_MATRIX_CONVOLUTION_TILEMODE, SkMatrixConvolutionImageFilter::kClampToBlack_TileMode },
};
#define CType           sk_matrix_convolution_tilemode_t
#define SKType          SkMatrixConvolutionImageFilter::TileMode
#define CTypeSkTypeMap  MAKE_FROM_TO_NAME(sk_matrix_convolution_tilemode_t)
#include "../sk_c_from_to.h"

const struct {
    sk_image_encoder_t fC;
    SkImageEncoder::Type fSK;
} MAKE_FROM_TO_NAME(sk_image_encoder_t)[] = {
    { UNKNOWN_SK_IMAGE_ENCODER_TYPE, SkImageEncoder::kUnknown_Type },
    { BMP_SK_IMAGE_ENCODER_TYPE, SkImageEncoder::kBMP_Type },
    { GIF_SK_IMAGE_ENCODER_TYPE, SkImageEncoder::kGIF_Type },
    { ICO_SK_IMAGE_ENCODER_TYPE, SkImageEncoder::kICO_Type },
    { JPEG_SK_IMAGE_ENCODER_TYPE, SkImageEncoder::kJPEG_Type },
    { PNG_SK_IMAGE_ENCODER_TYPE, SkImageEncoder::kPNG_Type },
    { WBMP_SK_IMAGE_ENCODER_TYPE, SkImageEncoder::kWBMP_Type },
    { WEBP_SK_IMAGE_ENCODER_TYPE, SkImageEncoder::kWEBP_Type },
    { KTX_SK_IMAGE_ENCODER_TYPE, SkImageEncoder::kKTX_Type },
};
#define CType           sk_image_encoder_t
#define SKType          SkImageEncoder::Type
#define CTypeSkTypeMap  MAKE_FROM_TO_NAME(sk_image_encoder_t)
#include "../sk_c_from_to.h"

const struct {
    sk_xfermode_mode_t  fC;
    SkXfermode::Mode    fSK;
} MAKE_FROM_TO_NAME(sk_xfermode_mode_t)[] = {
        { CLEAR_SK_XFERMODE_MODE,      SkXfermode::kClear_Mode      },
        { SRC_SK_XFERMODE_MODE,        SkXfermode::kSrc_Mode        },
        { DST_SK_XFERMODE_MODE,        SkXfermode::kDst_Mode        },
        { SRCOVER_SK_XFERMODE_MODE,    SkXfermode::kSrcOver_Mode    },
        { DSTOVER_SK_XFERMODE_MODE,    SkXfermode::kDstOver_Mode    },
        { SRCIN_SK_XFERMODE_MODE,      SkXfermode::kSrcIn_Mode      },
        { DSTIN_SK_XFERMODE_MODE,      SkXfermode::kDstIn_Mode      },
        { SRCOUT_SK_XFERMODE_MODE,     SkXfermode::kSrcOut_Mode     },
        { DSTOUT_SK_XFERMODE_MODE,     SkXfermode::kDstOut_Mode     },
        { SRCATOP_SK_XFERMODE_MODE,    SkXfermode::kSrcATop_Mode    },
        { DSTATOP_SK_XFERMODE_MODE,    SkXfermode::kDstATop_Mode    },
        { XOR_SK_XFERMODE_MODE,        SkXfermode::kXor_Mode        },
        { PLUS_SK_XFERMODE_MODE,       SkXfermode::kPlus_Mode       },
        { MODULATE_SK_XFERMODE_MODE,   SkXfermode::kModulate_Mode   },
        { SCREEN_SK_XFERMODE_MODE,     SkXfermode::kScreen_Mode     },
        { OVERLAY_SK_XFERMODE_MODE,    SkXfermode::kOverlay_Mode    },
        { DARKEN_SK_XFERMODE_MODE,     SkXfermode::kDarken_Mode     },
        { LIGHTEN_SK_XFERMODE_MODE,    SkXfermode::kLighten_Mode    },
        { COLORDODGE_SK_XFERMODE_MODE, SkXfermode::kColorDodge_Mode },
        { COLORBURN_SK_XFERMODE_MODE,  SkXfermode::kColorBurn_Mode  },
        { HARDLIGHT_SK_XFERMODE_MODE,  SkXfermode::kHardLight_Mode  },
        { SOFTLIGHT_SK_XFERMODE_MODE,  SkXfermode::kSoftLight_Mode  },
        { DIFFERENCE_SK_XFERMODE_MODE, SkXfermode::kDifference_Mode },
        { EXCLUSION_SK_XFERMODE_MODE,  SkXfermode::kExclusion_Mode  },
        { MULTIPLY_SK_XFERMODE_MODE,   SkXfermode::kMultiply_Mode   },
        { HUE_SK_XFERMODE_MODE,        SkXfermode::kHue_Mode        },
        { SATURATION_SK_XFERMODE_MODE, SkXfermode::kSaturation_Mode },
        { COLOR_SK_XFERMODE_MODE,      SkXfermode::kColor_Mode      },
        { LUMINOSITY_SK_XFERMODE_MODE, SkXfermode::kLuminosity_Mode }
};
#define CType           sk_xfermode_mode_t
#define SKType          SkXfermode::Mode
#define CTypeSkTypeMap  MAKE_FROM_TO_NAME(sk_xfermode_mode_t)
#include "../sk_c_from_to.h"

const struct {
    sk_colortype_t  fC;
    SkColorType     fSK;
} MAKE_FROM_TO_NAME(sk_colortype_t)[] = {
    { UNKNOWN_SK_COLORTYPE,     kUnknown_SkColorType    },
    { ALPHA_8_SK_COLORTYPE,     kAlpha_8_SkColorType    },
    { RGB_565_SK_COLORTYPE,     kRGB_565_SkColorType    },
    { ARGB_4444_SK_COLORTYPE,   kARGB_4444_SkColorType  },
    { RGBA_8888_SK_COLORTYPE,   kRGBA_8888_SkColorType  },
    { BGRA_8888_SK_COLORTYPE,   kBGRA_8888_SkColorType  },
    { Index_8_SK_COLORTYPE,     kIndex_8_SkColorType    },
    { Gray_8_SK_COLORTYPE,      kGray_8_SkColorType     },
    { RGBA_F16_SK_COLORTYPE,    kRGBA_F16_SkColorType   },
};
#define CType           sk_colortype_t
#define SKType          SkColorType
#define CTypeSkTypeMap  MAKE_FROM_TO_NAME(sk_colortype_t)
#include "../sk_c_from_to.h"

const struct {
    sk_alphatype_t  fC;
    SkAlphaType     fSK;
} MAKE_FROM_TO_NAME(sk_alphatype_t)[] = {
    { UNKNOWN_SK_ALPHATYPE,     kUnknown_SkAlphaType    },
    { OPAQUE_SK_ALPHATYPE,      kOpaque_SkAlphaType     },
    { PREMUL_SK_ALPHATYPE,      kPremul_SkAlphaType     },
    { UNPREMUL_SK_ALPHATYPE,    kUnpremul_SkAlphaType   },
};
#define CType           sk_alphatype_t
#define SKType          SkAlphaType
#define CTypeSkTypeMap  MAKE_FROM_TO_NAME(sk_alphatype_t)
#include "../sk_c_from_to.h"

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
#include "../sk_c_from_to.h"

const struct {
    sk_shader_tilemode_t    fC;
    SkShader::TileMode      fSK;
} MAKE_FROM_TO_NAME(sk_shader_tilemode_t)[] = {
    { CLAMP_SK_SHADER_TILEMODE,     SkShader::kClamp_TileMode  },
    { REPEAT_SK_SHADER_TILEMODE,    SkShader::kRepeat_TileMode },
    { MIRROR_SK_SHADER_TILEMODE,    SkShader::kMirror_TileMode },
};
#define CType           sk_shader_tilemode_t
#define SKType          SkShader::TileMode
#define CTypeSkTypeMap  MAKE_FROM_TO_NAME(sk_shader_tilemode_t)
#include "../sk_c_from_to.h"

const struct {
    sk_blurstyle_t  fC;
    SkBlurStyle     fSK;
} MAKE_FROM_TO_NAME(sk_blurstyle_t)[] = {
    { NORMAL_SK_BLUR_STYLE, kNormal_SkBlurStyle },
    { SOLID_SK_BLUR_STYLE,  kSolid_SkBlurStyle  },
    { OUTER_SK_BLUR_STYLE,  kOuter_SkBlurStyle  },
    { INNER_SK_BLUR_STYLE,  kInner_SkBlurStyle  },
};
#define CType           sk_blurstyle_t
#define SKType          SkBlurStyle
#define CTypeSkTypeMap  MAKE_FROM_TO_NAME(sk_blurstyle_t)
#include "../sk_c_from_to.h"

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
#include "../sk_c_from_to.h"

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
#include "../sk_c_from_to.h"

const struct {
    sk_region_op_t fC;
    SkRegion::Op   fSK;
} MAKE_FROM_TO_NAME(sk_region_op_t)[] = {
    { DIFFERENCE_SK_REGION_OP,         SkRegion::kDifference_Op },
    { INTERSECT_SK_REGION_OP,          SkRegion::kIntersect_Op },
    { UNION_SK_REGION_OP,              SkRegion::kUnion_Op },
    { XOR_SK_REGION_OP,                SkRegion::kXOR_Op },
    { REVERSE_DIFFERENCE_SK_REGION_OP, SkRegion::kReverseDifference_Op },
    { REPLACE_SK_REGION_OP,            SkRegion::kReplace_Op },
};
#define CType           sk_region_op_t
#define SKType          SkRegion::Op
#define CTypeSkTypeMap  MAKE_FROM_TO_NAME(sk_region_op_t)
#include "../sk_c_from_to.h"

const struct {
    sk_colorprofiletype_t  fC;
    SkColorProfileType     fSK;
} MAKE_FROM_TO_NAME(sk_colorprofiletype_t)[] = {
    { LINEAR_SK_COLORPROFILETYPE,  kLinear_SkColorProfileType },
    { SRGB_SK_COLORPROFILETYPE,    kSRGB_SkColorProfileType   },
};
#define CType           sk_colorprofiletype_t
#define SKType          SkColorProfileType
#define CTypeSkTypeMap  MAKE_FROM_TO_NAME(sk_colorprofiletype_t)
#include "../sk_c_from_to.h"

const struct {
    sk_encoded_format_t  fC;
    SkEncodedFormat      fSK;
} MAKE_FROM_TO_NAME(sk_encoded_format_t)[] = {
    { UNKNOWN_SK_ENCODED_FORMAT, SkEncodedFormat::kUnknown_SkEncodedFormat },
    { BMP_SK_ENCODED_FORMAT,     SkEncodedFormat::kBMP_SkEncodedFormat     },
    { GIF_SK_ENCODED_FORMAT,     SkEncodedFormat::kGIF_SkEncodedFormat     },
    { ICO_SK_ENCODED_FORMAT,     SkEncodedFormat::kICO_SkEncodedFormat     },
    { JPEG_SK_ENCODED_FORMAT,    SkEncodedFormat::kJPEG_SkEncodedFormat    },
    { PNG_SK_ENCODED_FORMAT,     SkEncodedFormat::kPNG_SkEncodedFormat     },
    { WBMP_SK_ENCODED_FORMAT,    SkEncodedFormat::kWBMP_SkEncodedFormat    },
    { WEBP_SK_ENCODED_FORMAT,    SkEncodedFormat::kWEBP_SkEncodedFormat    },
    { PKM_SK_ENCODED_FORMAT,     SkEncodedFormat::kPKM_SkEncodedFormat     },
    { KTX_SK_ENCODED_FORMAT,     SkEncodedFormat::kKTX_SkEncodedFormat     },
    { ASTC_SK_ENCODED_FORMAT,    SkEncodedFormat::kASTC_SkEncodedFormat    },
    { DNG_SK_ENCODED_FORMAT,     SkEncodedFormat::kDNG_SkEncodedFormat     },
};
#define CType           sk_encoded_format_t
#define SKType          SkEncodedFormat
#define CTypeSkTypeMap  MAKE_FROM_TO_NAME(sk_encoded_format_t)
#include "../sk_c_from_to.h"

const struct {
    sk_codec_origin_t  fC;
    SkCodec::Origin    fSK;
} MAKE_FROM_TO_NAME(sk_codec_origin_t)[] = {
    { TOP_LEFT_SK_CODEC_ORIGIN,      SkCodec::kTopLeft_Origin },
    { TOP_RIGHT_SK_CODEC_ORIGIN,     SkCodec::kTopRight_Origin },
    { BOTTOM_RIGHT_SK_CODEC_ORIGIN,  SkCodec::kBottomRight_Origin },
    { BOTTOM_LEFT_SK_CODEC_ORIGIN,   SkCodec::kBottomLeft_Origin },
    { LEFT_TOP_SK_CODEC_ORIGIN,      SkCodec::kLeftTop_Origin },
    { RIGHT_TOP_SK_CODEC_ORIGIN,     SkCodec::kRightTop_Origin },
    { RIGHT_BOTTOM_SK_CODEC_ORIGIN,  SkCodec::kRightBottom_Origin },
    { LEFT_BOTTOM_SK_CODEC_ORIGIN,   SkCodec::kLeftBottom_Origin },
};
#define CType           sk_codec_origin_t
#define SKType          SkCodec::Origin
#define CTypeSkTypeMap  MAKE_FROM_TO_NAME(sk_codec_origin_t)
#include "../sk_c_from_to.h"

const struct {
    sk_codec_result_t  fC;
    SkCodec::Result    fSK;
} MAKE_FROM_TO_NAME(sk_codec_result_t)[] = {
    { SUCCESS_SK_CODEC_RESULT,             SkCodec::kSuccess           },
    { INCOMPLETE_INPUT_SK_CODEC_RESULT,    SkCodec::kIncompleteInput   },
    { INVALID_CONVERSION_SK_CODEC_RESULT,  SkCodec::kInvalidConversion },
    { INVALID_SCALE_SK_CODEC_RESULT,       SkCodec::kInvalidScale      },
    { INVALID_PARAMETERS_SK_CODEC_RESULT,  SkCodec::kInvalidParameters },
    { INVALID_INPUT_SK_CODEC_RESULT,       SkCodec::kInvalidInput      },
    { COULD_NOT_REWIND_SK_CODEC_RESULT,    SkCodec::kCouldNotRewind    },
    { UNIMPLEMENTED_SK_CODEC_RESULT,       SkCodec::kUnimplemented     },
};
#define CType           sk_codec_result_t
#define SKType          SkCodec::Result
#define CTypeSkTypeMap  MAKE_FROM_TO_NAME(sk_codec_result_t)
#include "../sk_c_from_to.h"

const struct {
    sk_codec_zero_initialized_t  fC;
    SkCodec::ZeroInitialized     fSK;
} MAKE_FROM_TO_NAME(sk_codec_zero_initialized_t)[] = {
    { YES_SK_CODEC_ZERO_INITIALIZED,  SkCodec::kYes_ZeroInitialized  },
    { NO_SK_CODEC_ZERO_INITIALIZED,   SkCodec::kNo_ZeroInitialized   },
};
#define CType           sk_codec_zero_initialized_t
#define SKType          SkCodec::ZeroInitialized
#define CTypeSkTypeMap  MAKE_FROM_TO_NAME(sk_codec_zero_initialized_t)
#include "../sk_c_from_to.h"

static inline SkRect* AsRect(sk_rect_t* crect) {
    return reinterpret_cast<SkRect*>(crect);
}

static inline const SkRect* AsRect(const sk_rect_t* crect) {
    return reinterpret_cast<const SkRect*>(crect);
}

static inline const SkRect& AsRect(const sk_rect_t& crect) {
    return reinterpret_cast<const SkRect&>(crect);
}

static inline sk_rect_t ToRect(const SkRect& rect) {
    return reinterpret_cast<const sk_rect_t&>(rect);
}

static inline SkIRect* AsIRect(sk_irect_t* crect) {
    return reinterpret_cast<SkIRect*>(crect);
}

static inline const SkIRect* AsIRect(const sk_irect_t* crect) {
    return reinterpret_cast<const SkIRect*>(crect);
}

static inline const SkIRect& AsIRect(const sk_irect_t& crect) {
    return reinterpret_cast<const SkIRect&>(crect);
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

static inline SkData* AsData(const sk_data_t* cdata) {
    return reinterpret_cast<SkData*>(const_cast<sk_data_t*>(cdata));
}

static inline sk_data_t* ToData(SkData* data) {
    return reinterpret_cast<sk_data_t*>(data);
}

static inline sk_path_t* ToPath(SkPath* cpath) {
    return reinterpret_cast<sk_path_t*>(cpath);
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

static inline SkImageFilter* AsImageFilter(sk_imagefilter_t* cfilter) {
    return reinterpret_cast<SkImageFilter*>(cfilter);
}

static inline SkImageFilter** AsImageFilters(sk_imagefilter_t** cfilter) {
    return reinterpret_cast<SkImageFilter**>(cfilter);
}

static inline sk_imagefilter_t* ToImageFilter(SkImageFilter* filter) {
    return reinterpret_cast<sk_imagefilter_t*>(filter);
}

static inline SkColorFilter* AsColorFilter(sk_colorfilter_t* cfilter) {
    return reinterpret_cast<SkColorFilter*>(cfilter);
}

static inline sk_colorfilter_t* ToColorFilter(SkColorFilter* filter) {
    return reinterpret_cast<sk_colorfilter_t*>(filter);
}

static inline const SkCodec* AsCodec(const sk_codec_t* codec) {
    return reinterpret_cast<const SkCodec*>(codec);
}

static inline const SkCodec& AsCodec(const sk_codec_t& codec) {
    return reinterpret_cast<const SkCodec&>(codec);
}

static inline SkCodec* AsCodec(sk_codec_t* codec) {
    return reinterpret_cast<SkCodec*>(codec);
}

static inline sk_codec_t* ToCodec(SkCodec* codec) {
    return reinterpret_cast<sk_codec_t*>(codec);
}

static inline SkTypeface* AsTypeface(sk_typeface_t* typeface) {
    return reinterpret_cast<SkTypeface*>(typeface);
}

static inline sk_typeface_t* ToTypeface(SkTypeface* typeface) {
    return reinterpret_cast<sk_typeface_t*>(typeface);
}

static inline sk_colorspace_t* ToColorSpace(SkColorSpace* colorspace) {
    return reinterpret_cast<sk_colorspace_t*>(colorspace);
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

static inline void from_c(const sk_matrix_t* cmatrix, SkMatrix* matrix) {
    matrix->setAll(
        cmatrix->mat[0], cmatrix->mat[1], cmatrix->mat[2],
        cmatrix->mat[3], cmatrix->mat[4], cmatrix->mat[5],
        cmatrix->mat[6], cmatrix->mat[7], cmatrix->mat[8]);
}

static inline void from_sk(const SkMatrix* matrix, sk_matrix_t* cmatrix) {
    matrix->get9(cmatrix->mat);
}

static inline sk_shader_t* ToShader(SkShader* shader) {
    return reinterpret_cast<sk_shader_t*>(shader);
}

static inline const SkFILEStream* AsFileStream(const sk_stream_filestream_t* cfilestream) {
    return reinterpret_cast<const SkFILEStream*>(cfilestream);
}

static inline SkFILEStream* AsFileStream(sk_stream_filestream_t* cfilestream) {
    return reinterpret_cast<SkFILEStream*>(cfilestream);
}

static inline sk_stream_filestream_t* ToFileStream(SkFILEStream* stream) {
    return reinterpret_cast<sk_stream_filestream_t*>(stream);
}

static inline const SkMemoryStream* AsMemoryStream(const sk_stream_memorystream_t* cmemorystream) {
    return reinterpret_cast<const SkMemoryStream*>(cmemorystream);
}

static inline SkMemoryStream* AsMemoryStream(sk_stream_memorystream_t* cmemorystream) {
    return reinterpret_cast<SkMemoryStream*>(cmemorystream);
}

static inline sk_stream_memorystream_t* ToMemoryStream(SkMemoryStream* stream) {
    return reinterpret_cast<sk_stream_memorystream_t*>(stream);
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

static inline sk_stream_t* ToStream(SkStream* cstream) {
    return reinterpret_cast<sk_stream_t*>(cstream);
}

static inline sk_stream_asset_t* ToStreamAsset(SkStreamAsset* cstream) {
    return reinterpret_cast<sk_stream_asset_t*>(cstream);
}

static inline SkStreamAsset* AsStreamAsset(sk_stream_asset_t* cstream) {
    return reinterpret_cast<SkStreamAsset*>(cstream);
}

static inline SkFILEWStream* AsFileWStream(sk_wstream_filestream_t* cfilestream) {
    return reinterpret_cast<SkFILEWStream*>(cfilestream);
}

static inline SkDynamicMemoryWStream* AsDynamicMemoryWStream(sk_wstream_dynamicmemorystream_t* cmemorystream) {
    return reinterpret_cast<SkDynamicMemoryWStream*>(cmemorystream);
}

static inline SkWStream* AsWStream(sk_wstream_t* cstream) {
    return reinterpret_cast<SkWStream*>(cstream);
}

static inline sk_wstream_filestream_t* ToFileWStream(SkFILEWStream* filestream) {
    return reinterpret_cast<sk_wstream_filestream_t*>(filestream);
}

static inline sk_wstream_dynamicmemorystream_t* ToDynamicMemoryWStream(SkDynamicMemoryWStream* memorystream) {
    return reinterpret_cast<sk_wstream_dynamicmemorystream_t*>(memorystream);
}

static inline sk_wstream_t* ToWStream(SkWStream* stream) {
    return reinterpret_cast<sk_wstream_t*>(stream);
}

static inline const SkPoint& AsPoint(const sk_point_t& p) {
    return reinterpret_cast<const SkPoint&>(p);
}

static inline const SkPoint* AsPoint(const sk_point_t* p) {
    return reinterpret_cast<const SkPoint*>(p);
}

static inline const SkIPoint& AsIPoint(const sk_ipoint_t& p) {
    return reinterpret_cast<const SkIPoint&>(p);
}

static inline const SkIPoint* AsIPoint(const sk_ipoint_t* p) {
    return reinterpret_cast<const SkIPoint*>(p);
}

static inline const SkSize& AsSize(const sk_size_t& p) {
    return reinterpret_cast<const SkSize&>(p);
}

static inline const SkSize* AsSize(const sk_size_t* p) {
    return reinterpret_cast<const SkSize*>(p);
}

static inline const SkISize& AsISize(const sk_isize_t& p) {
    return reinterpret_cast<const SkISize&>(p);
}

static inline const SkISize* AsISize(const sk_isize_t* p) {
    return reinterpret_cast<const SkISize*>(p);
}

static inline SkISize* AsISize(sk_isize_t* p) {
    return reinterpret_cast<SkISize*>(p);
}

static inline const sk_isize_t& ToISize(const SkISize& p) {
    return reinterpret_cast<const sk_isize_t&>(p);
}

static inline const sk_isize_t* ToISize(const SkISize* p) {
    return reinterpret_cast<const sk_isize_t*>(p);
}

static inline const SkPoint3& AsPoint3(const sk_point3_t& p) {
    return reinterpret_cast<const SkPoint3&>(p);
}

static inline const SkPoint3* AsPoint3(const sk_point3_t* p) {
    return reinterpret_cast<const SkPoint3*>(p);
}

static inline const SkImageFilter::CropRect& AsImageFilterCropRect(const sk_imagefilter_croprect_t& p) {
    return reinterpret_cast<const SkImageFilter::CropRect&>(p);
}

static inline const SkImageFilter::CropRect* AsImageFilterCropRect(const sk_imagefilter_croprect_t* p) {
    return reinterpret_cast<const SkImageFilter::CropRect*>(p);
}

static inline SkPaint::FontMetrics* AsFontMetrics(sk_fontmetrics_t* p) {
    return reinterpret_cast<SkPaint::FontMetrics*>(p);
}

static inline sk_fontmetrics_t* ToFontMetrics(SkPaint::FontMetrics* p) {
    return reinterpret_cast<sk_fontmetrics_t*>(p);
}

static inline SkString* AsString(const sk_string_t* cdata) {
    return reinterpret_cast<SkString*>(const_cast<sk_string_t*>(cdata));
}

static inline sk_string_t* ToString(SkString* data) {
    return reinterpret_cast<sk_string_t*>(data);
}

static inline SkDocument* AsDocument(sk_document_t* cdocument) {
    return reinterpret_cast<SkDocument*>(cdocument);
}

static inline sk_document_t* ToDocument(SkDocument* document) {
    return reinterpret_cast<sk_document_t*>(document);
}

static inline SkImageInfo* AsImageInfo(sk_imageinfo_t* cinfo) {
    return reinterpret_cast<SkImageInfo*>(cinfo);
}

static inline const SkImageInfo* AsImageInfo(const sk_imageinfo_t* cinfo) {
    return reinterpret_cast<const SkImageInfo*>(cinfo);
}

static inline sk_imageinfo_t* ToImageInfo(SkImageInfo* info) {
    return reinterpret_cast<sk_imageinfo_t*>(info);
}

static inline sk_imageinfo_t& ToImageInfo(SkImageInfo& info) {
    return reinterpret_cast<sk_imageinfo_t&>(info);
}

static inline const sk_imageinfo_t* ToImageInfo(const SkImageInfo* info) {
    return reinterpret_cast<const sk_imageinfo_t*>(info);
}

static inline const sk_imageinfo_t& ToImageInfo(const SkImageInfo& info) {
    return reinterpret_cast<const sk_imageinfo_t&>(info);
}

static inline bool find_sk(const sk_codec_options_t& coptions, SkCodec::Options* options) {
    SkCodec::ZeroInitialized zero;

    if (!find_sk(coptions.fZeroInitialized, &zero)) {
        // optionally report error to client?
        return false;
    }

    if (options) {
        *options = SkCodec::Options();
        options->fZeroInitialized = zero;
        if (coptions.fHasSubset) {
            options->fSubset = AsIRect((sk_irect_t*)&coptions.fSubset);
        }
    }
    return true;
}


#endif
