/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypeface.h"
#include "SkFontStyle.h"

#include "sk_types_priv.h"

#if __cplusplus >= 199711L

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define ASSERT_MSG(SK, C) "ABI changed, you must write a enumeration mapper for " TOSTRING(#SK) " to " TOSTRING(#C) "."

// sk_typeface_style_t
static_assert ((int)SkTypeface::Style::kNormal       == (int)NORMAL_TYPEFACE_STYLE,        ASSERT_MSG(SkTypeface::Style, sk_typeface_style_t));
static_assert ((int)SkTypeface::Style::kBold         == (int)BOLD_TYPEFACE_STYLE,          ASSERT_MSG(SkTypeface::Style, sk_typeface_style_t));
static_assert ((int)SkTypeface::Style::kItalic       == (int)ITALIC_TYPEFACE_STYLE,        ASSERT_MSG(SkTypeface::Style, sk_typeface_style_t));
static_assert ((int)SkTypeface::Style::kBoldItalic   == (int)BOLD_ITALIC_TYPEFACE_STYLE,   ASSERT_MSG(SkTypeface::Style, sk_typeface_style_t));

// sk_encoding_t
static_assert ((int)SkTypeface::Encoding::kUTF8_Encoding    == (int)UTF8_ENCODING,    ASSERT_MSG(SkTypeface::Encoding, sk_encoding_t));
static_assert ((int)SkTypeface::Encoding::kUTF16_Encoding   == (int)UTF16_ENCODING,   ASSERT_MSG(SkTypeface::Encoding, sk_encoding_t));
static_assert ((int)SkTypeface::Encoding::kUTF32_Encoding   == (int)UTF32_ENCODING,   ASSERT_MSG(SkTypeface::Encoding, sk_encoding_t));

// sk_font_style_slant_t
static_assert ((int)SkFontStyle::Slant::kUpright_Slant   == (int)UPRIGHT_SK_FONT_STYLE_SLANT,   ASSERT_MSG(SkFontStyle::Slant, sk_font_style_slant_t));
static_assert ((int)SkFontStyle::Slant::kItalic_Slant    == (int)ITALIC_SK_FONT_STYLE_SLANT,    ASSERT_MSG(SkFontStyle::Slant, sk_font_style_slant_t));
static_assert ((int)SkFontStyle::Slant::kOblique_Slant   == (int)OBLIQUE_SK_FONT_STYLE_SLANT,   ASSERT_MSG(SkFontStyle::Slant, sk_font_style_slant_t));

// sk_path_verb_t
static_assert ((int)SkPath::Verb::kMove_Verb    == (int)MOVE_SK_PATH_VERB,    ASSERT_MSG(SkPath::Verb, sk_path_verb_t));
static_assert ((int)SkPath::Verb::kLine_Verb    == (int)LINE_SK_PATH_VERB,    ASSERT_MSG(SkPath::Verb, sk_path_verb_t));
static_assert ((int)SkPath::Verb::kQuad_Verb    == (int)QUAD_SK_PATH_VERB,    ASSERT_MSG(SkPath::Verb, sk_path_verb_t));
static_assert ((int)SkPath::Verb::kConic_Verb   == (int)CONIC_SK_PATH_VERB,   ASSERT_MSG(SkPath::Verb, sk_path_verb_t));
static_assert ((int)SkPath::Verb::kCubic_Verb   == (int)CUBIC_SK_PATH_VERB,   ASSERT_MSG(SkPath::Verb, sk_path_verb_t));
static_assert ((int)SkPath::Verb::kClose_Verb   == (int)CLOSE_SK_PATH_VERB,   ASSERT_MSG(SkPath::Verb, sk_path_verb_t));
static_assert ((int)SkPath::Verb::kDone_Verb    == (int)DONE_SK_PATH_VERB,    ASSERT_MSG(SkPath::Verb, sk_path_verb_t));

// sk_path_add_mode_t
static_assert ((int)SkPath::AddPathMode::kAppend_AddPathMode   == (int)APPEND_SK_PATH_ADD_MODE,   ASSERT_MSG(SkPath::AddPathMode, sk_path_add_mode_t));
static_assert ((int)SkPath::AddPathMode::kExtend_AddPathMode   == (int)EXTEND_SK_PATH_ADD_MODE,   ASSERT_MSG(SkPath::AddPathMode, sk_path_add_mode_t));

// sk_path_direction_t
static_assert ((int)SkPath::Direction::kCCW_Direction   == (int)CCW_SK_PATH_DIRECTION,   ASSERT_MSG(SkPath::Direction, sk_path_direction_t));
static_assert ((int)SkPath::Direction::kCW_Direction    == (int)CW_SK_PATH_DIRECTION,    ASSERT_MSG(SkPath::Direction, sk_path_direction_t));

// sk_path_filltype_t
static_assert ((int)SkPath::FillType::kWinding_FillType          == (int)WINDING_SK_PATH_FILLTYPE,           ASSERT_MSG(SkPath::FillType, sk_path_filltype_t));
static_assert ((int)SkPath::FillType::kEvenOdd_FillType          == (int)EVENODD_SK_PATH_FILLTYPE,           ASSERT_MSG(SkPath::FillType, sk_path_filltype_t));
static_assert ((int)SkPath::FillType::kInverseWinding_FillType   == (int)INVERSE_WINDING_SK_PATH_FILLTYPE,   ASSERT_MSG(SkPath::FillType, sk_path_filltype_t));
static_assert ((int)SkPath::FillType::kInverseEvenOdd_FillType   == (int)INVERSE_EVENODD_SK_PATH_FILLTYPE,   ASSERT_MSG(SkPath::FillType, sk_path_filltype_t));

// sk_text_align_t
static_assert ((int)SkPaint::Align::kLeft_Align     == (int)LEFT_SK_TEXT_ALIGN,     ASSERT_MSG(SkPaint::Align, sk_text_align_t));
static_assert ((int)SkPaint::Align::kCenter_Align   == (int)CENTER_SK_TEXT_ALIGN,   ASSERT_MSG(SkPaint::Align, sk_text_align_t));
static_assert ((int)SkPaint::Align::kRight_Align    == (int)RIGHT_SK_TEXT_ALIGN,    ASSERT_MSG(SkPaint::Align, sk_text_align_t));

// sk_text_encoding_t
static_assert ((int)SkPaint::TextEncoding::kUTF8_TextEncoding      == (int)UTF8_SK_TEXT_ENCODING,       ASSERT_MSG(SkPaint::TextEncoding, sk_text_encoding_t));
static_assert ((int)SkPaint::TextEncoding::kUTF16_TextEncoding     == (int)UTF16_SK_TEXT_ENCODING,      ASSERT_MSG(SkPaint::TextEncoding, sk_text_encoding_t));
static_assert ((int)SkPaint::TextEncoding::kUTF32_TextEncoding     == (int)UTF32_SK_TEXT_ENCODING,      ASSERT_MSG(SkPaint::TextEncoding, sk_text_encoding_t));
static_assert ((int)SkPaint::TextEncoding::kGlyphID_TextEncoding   == (int)GLYPH_ID_SK_TEXT_ENCODING,   ASSERT_MSG(SkPaint::TextEncoding, sk_text_encoding_t));

// sk_filter_quality_t
static_assert ((int)SkFilterQuality::kNone_SkFilterQuality     == (int)NONE_SK_FILTER_QUALITY,     ASSERT_MSG(SkFilterQuality, sk_filter_quality_t));
static_assert ((int)SkFilterQuality::kLow_SkFilterQuality      == (int)LOW_SK_FILTER_QUALITY,      ASSERT_MSG(SkFilterQuality, sk_filter_quality_t));
static_assert ((int)SkFilterQuality::kMedium_SkFilterQuality   == (int)MEDIUM_SK_FILTER_QUALITY,   ASSERT_MSG(SkFilterQuality, sk_filter_quality_t));
static_assert ((int)SkFilterQuality::kHigh_SkFilterQuality     == (int)HIGH_SK_FILTER_QUALITY,     ASSERT_MSG(SkFilterQuality, sk_filter_quality_t));

// sk_crop_rect_flags_t
static_assert ((int)SkImageFilter::CropRect::CropEdge::kHasLeft_CropEdge     == (int)HAS_LEFT_SK_CROP_RECT_FLAG,     ASSERT_MSG(SkImageFilter::CropRect::CropEdge, sk_crop_rect_flags_t));
static_assert ((int)SkImageFilter::CropRect::CropEdge::kHasTop_CropEdge      == (int)HAS_TOP_SK_CROP_RECT_FLAG,      ASSERT_MSG(SkImageFilter::CropRect::CropEdge, sk_crop_rect_flags_t));
static_assert ((int)SkImageFilter::CropRect::CropEdge::kHasWidth_CropEdge    == (int)HAS_WIDTH_SK_CROP_RECT_FLAG,    ASSERT_MSG(SkImageFilter::CropRect::CropEdge, sk_crop_rect_flags_t));
static_assert ((int)SkImageFilter::CropRect::CropEdge::kHasHeight_CropEdge   == (int)HAS_HEIGHT_SK_CROP_RECT_FLAG,   ASSERT_MSG(SkImageFilter::CropRect::CropEdge, sk_crop_rect_flags_t));
static_assert ((int)SkImageFilter::CropRect::CropEdge::kHasAll_CropEdge      == (int)HAS_ALL_SK_CROP_RECT_FLAG,      ASSERT_MSG(SkImageFilter::CropRect::CropEdge, sk_crop_rect_flags_t));

// sk_drop_shadow_image_filter_shadow_mode_t
static_assert ((int)SkDropShadowImageFilter::ShadowMode::kDrawShadowAndForeground_ShadowMode   == (int)DRAW_SHADOW_AND_FOREGROUND_SK_DROP_SHADOW_IMAGE_FILTER_SHADOW_MODE,   ASSERT_MSG(SkDropShadowImageFilter::ShadowMode, sk_drop_shadow_image_filter_shadow_mode_t));
static_assert ((int)SkDropShadowImageFilter::ShadowMode::kDrawShadowOnly_ShadowMode            == (int)DRAW_SHADOW_ONLY_SK_DROP_SHADOW_IMAGE_FILTER_SHADOW_MODE,             ASSERT_MSG(SkDropShadowImageFilter::ShadowMode, sk_drop_shadow_image_filter_shadow_mode_t));

// sk_displacement_map_effect_channel_selector_type_t
static_assert ((int)SkDisplacementMapEffect::ChannelSelectorType::kUnknown_ChannelSelectorType   == (int)UNKNOWN_SK_DISPLACEMENT_MAP_EFFECT_CHANNEL_SELECTOR_TYPE,   ASSERT_MSG(SkDisplacementMapEffect::ChannelSelectorType, sk_displacement_map_effect_channel_selector_type_t));
static_assert ((int)SkDisplacementMapEffect::ChannelSelectorType::kR_ChannelSelectorType         == (int)R_SK_DISPLACEMENT_MAP_EFFECT_CHANNEL_SELECTOR_TYPE,         ASSERT_MSG(SkDisplacementMapEffect::ChannelSelectorType, sk_displacement_map_effect_channel_selector_type_t));
static_assert ((int)SkDisplacementMapEffect::ChannelSelectorType::kG_ChannelSelectorType         == (int)G_SK_DISPLACEMENT_MAP_EFFECT_CHANNEL_SELECTOR_TYPE,         ASSERT_MSG(SkDisplacementMapEffect::ChannelSelectorType, sk_displacement_map_effect_channel_selector_type_t));
static_assert ((int)SkDisplacementMapEffect::ChannelSelectorType::kB_ChannelSelectorType         == (int)B_SK_DISPLACEMENT_MAP_EFFECT_CHANNEL_SELECTOR_TYPE,         ASSERT_MSG(SkDisplacementMapEffect::ChannelSelectorType, sk_displacement_map_effect_channel_selector_type_t));
static_assert ((int)SkDisplacementMapEffect::ChannelSelectorType::kA_ChannelSelectorType         == (int)A_SK_DISPLACEMENT_MAP_EFFECT_CHANNEL_SELECTOR_TYPE,         ASSERT_MSG(SkDisplacementMapEffect::ChannelSelectorType, sk_displacement_map_effect_channel_selector_type_t));

// sk_matrix_convolution_tilemode_t
static_assert ((int)SkMatrixConvolutionImageFilter::TileMode::kClamp_TileMode          == (int)CLAMP_SK_MATRIX_CONVOLUTION_TILEMODE,            ASSERT_MSG(SkMatrixConvolutionImageFilter::TileMode, sk_matrix_convolution_tilemode_t));
static_assert ((int)SkMatrixConvolutionImageFilter::TileMode::kRepeat_TileMode         == (int)REPEAT_SK_MATRIX_CONVOLUTION_TILEMODE,           ASSERT_MSG(SkMatrixConvolutionImageFilter::TileMode, sk_matrix_convolution_tilemode_t));
static_assert ((int)SkMatrixConvolutionImageFilter::TileMode::kClampToBlack_TileMode   == (int)CLAMP_TO_BLACK_SK_MATRIX_CONVOLUTION_TILEMODE,   ASSERT_MSG(SkMatrixConvolutionImageFilter::TileMode, sk_matrix_convolution_tilemode_t));

// sk_image_encoder_t
static_assert ((int)SkImageEncoder::Type::kUnknown_Type   == (int)UNKNOWN_SK_IMAGE_ENCODER_TYPE,   ASSERT_MSG(SkImageEncoder::Type, sk_image_encoder_t));
static_assert ((int)SkImageEncoder::Type::kBMP_Type       == (int)BMP_SK_IMAGE_ENCODER_TYPE,       ASSERT_MSG(SkImageEncoder::Type, sk_image_encoder_t));
static_assert ((int)SkImageEncoder::Type::kGIF_Type       == (int)GIF_SK_IMAGE_ENCODER_TYPE,       ASSERT_MSG(SkImageEncoder::Type, sk_image_encoder_t));
static_assert ((int)SkImageEncoder::Type::kICO_Type       == (int)ICO_SK_IMAGE_ENCODER_TYPE,       ASSERT_MSG(SkImageEncoder::Type, sk_image_encoder_t));
static_assert ((int)SkImageEncoder::Type::kJPEG_Type      == (int)JPEG_SK_IMAGE_ENCODER_TYPE,      ASSERT_MSG(SkImageEncoder::Type, sk_image_encoder_t));
static_assert ((int)SkImageEncoder::Type::kPNG_Type       == (int)PNG_SK_IMAGE_ENCODER_TYPE,       ASSERT_MSG(SkImageEncoder::Type, sk_image_encoder_t));
static_assert ((int)SkImageEncoder::Type::kWBMP_Type      == (int)WBMP_SK_IMAGE_ENCODER_TYPE,      ASSERT_MSG(SkImageEncoder::Type, sk_image_encoder_t));
static_assert ((int)SkImageEncoder::Type::kWEBP_Type      == (int)WEBP_SK_IMAGE_ENCODER_TYPE,      ASSERT_MSG(SkImageEncoder::Type, sk_image_encoder_t));
static_assert ((int)SkImageEncoder::Type::kKTX_Type       == (int)KTX_SK_IMAGE_ENCODER_TYPE,       ASSERT_MSG(SkImageEncoder::Type, sk_image_encoder_t));

// sk_xfermode_mode_t
static_assert ((int)SkXfermode::Mode::kClear_Mode        == (int)CLEAR_SK_XFERMODE_MODE,        ASSERT_MSG(SkXfermode::Mode, sk_xfermode_mode_t));
static_assert ((int)SkXfermode::Mode::kSrc_Mode          == (int)SRC_SK_XFERMODE_MODE,          ASSERT_MSG(SkXfermode::Mode, sk_xfermode_mode_t));
static_assert ((int)SkXfermode::Mode::kDst_Mode          == (int)DST_SK_XFERMODE_MODE,          ASSERT_MSG(SkXfermode::Mode, sk_xfermode_mode_t));
static_assert ((int)SkXfermode::Mode::kSrcOver_Mode      == (int)SRCOVER_SK_XFERMODE_MODE,      ASSERT_MSG(SkXfermode::Mode, sk_xfermode_mode_t));
static_assert ((int)SkXfermode::Mode::kDstOver_Mode      == (int)DSTOVER_SK_XFERMODE_MODE,      ASSERT_MSG(SkXfermode::Mode, sk_xfermode_mode_t));
static_assert ((int)SkXfermode::Mode::kSrcIn_Mode        == (int)SRCIN_SK_XFERMODE_MODE,        ASSERT_MSG(SkXfermode::Mode, sk_xfermode_mode_t));
static_assert ((int)SkXfermode::Mode::kDstIn_Mode        == (int)DSTIN_SK_XFERMODE_MODE,        ASSERT_MSG(SkXfermode::Mode, sk_xfermode_mode_t));
static_assert ((int)SkXfermode::Mode::kSrcOut_Mode       == (int)SRCOUT_SK_XFERMODE_MODE,       ASSERT_MSG(SkXfermode::Mode, sk_xfermode_mode_t));
static_assert ((int)SkXfermode::Mode::kDstOut_Mode       == (int)DSTOUT_SK_XFERMODE_MODE,       ASSERT_MSG(SkXfermode::Mode, sk_xfermode_mode_t));
static_assert ((int)SkXfermode::Mode::kSrcATop_Mode      == (int)SRCATOP_SK_XFERMODE_MODE,      ASSERT_MSG(SkXfermode::Mode, sk_xfermode_mode_t));
static_assert ((int)SkXfermode::Mode::kDstATop_Mode      == (int)DSTATOP_SK_XFERMODE_MODE,      ASSERT_MSG(SkXfermode::Mode, sk_xfermode_mode_t));
static_assert ((int)SkXfermode::Mode::kXor_Mode          == (int)XOR_SK_XFERMODE_MODE,          ASSERT_MSG(SkXfermode::Mode, sk_xfermode_mode_t));
static_assert ((int)SkXfermode::Mode::kPlus_Mode         == (int)PLUS_SK_XFERMODE_MODE,         ASSERT_MSG(SkXfermode::Mode, sk_xfermode_mode_t));
static_assert ((int)SkXfermode::Mode::kModulate_Mode     == (int)MODULATE_SK_XFERMODE_MODE,     ASSERT_MSG(SkXfermode::Mode, sk_xfermode_mode_t));
static_assert ((int)SkXfermode::Mode::kScreen_Mode       == (int)SCREEN_SK_XFERMODE_MODE,       ASSERT_MSG(SkXfermode::Mode, sk_xfermode_mode_t));
static_assert ((int)SkXfermode::Mode::kOverlay_Mode      == (int)OVERLAY_SK_XFERMODE_MODE,      ASSERT_MSG(SkXfermode::Mode, sk_xfermode_mode_t));
static_assert ((int)SkXfermode::Mode::kDarken_Mode       == (int)DARKEN_SK_XFERMODE_MODE,       ASSERT_MSG(SkXfermode::Mode, sk_xfermode_mode_t));
static_assert ((int)SkXfermode::Mode::kLighten_Mode      == (int)LIGHTEN_SK_XFERMODE_MODE,      ASSERT_MSG(SkXfermode::Mode, sk_xfermode_mode_t));
static_assert ((int)SkXfermode::Mode::kColorDodge_Mode   == (int)COLORDODGE_SK_XFERMODE_MODE,   ASSERT_MSG(SkXfermode::Mode, sk_xfermode_mode_t));
static_assert ((int)SkXfermode::Mode::kColorBurn_Mode    == (int)COLORBURN_SK_XFERMODE_MODE,    ASSERT_MSG(SkXfermode::Mode, sk_xfermode_mode_t));
static_assert ((int)SkXfermode::Mode::kHardLight_Mode    == (int)HARDLIGHT_SK_XFERMODE_MODE,    ASSERT_MSG(SkXfermode::Mode, sk_xfermode_mode_t));
static_assert ((int)SkXfermode::Mode::kSoftLight_Mode    == (int)SOFTLIGHT_SK_XFERMODE_MODE,    ASSERT_MSG(SkXfermode::Mode, sk_xfermode_mode_t));
static_assert ((int)SkXfermode::Mode::kDifference_Mode   == (int)DIFFERENCE_SK_XFERMODE_MODE,   ASSERT_MSG(SkXfermode::Mode, sk_xfermode_mode_t));
static_assert ((int)SkXfermode::Mode::kExclusion_Mode    == (int)EXCLUSION_SK_XFERMODE_MODE,    ASSERT_MSG(SkXfermode::Mode, sk_xfermode_mode_t));
static_assert ((int)SkXfermode::Mode::kMultiply_Mode     == (int)MULTIPLY_SK_XFERMODE_MODE,     ASSERT_MSG(SkXfermode::Mode, sk_xfermode_mode_t));
static_assert ((int)SkXfermode::Mode::kHue_Mode          == (int)HUE_SK_XFERMODE_MODE,          ASSERT_MSG(SkXfermode::Mode, sk_xfermode_mode_t));
static_assert ((int)SkXfermode::Mode::kSaturation_Mode   == (int)SATURATION_SK_XFERMODE_MODE,   ASSERT_MSG(SkXfermode::Mode, sk_xfermode_mode_t));
static_assert ((int)SkXfermode::Mode::kColor_Mode        == (int)COLOR_SK_XFERMODE_MODE,        ASSERT_MSG(SkXfermode::Mode, sk_xfermode_mode_t));
static_assert ((int)SkXfermode::Mode::kLuminosity_Mode   == (int)LUMINOSITY_SK_XFERMODE_MODE,   ASSERT_MSG(SkXfermode::Mode, sk_xfermode_mode_t));

// sk_colortype_t
static_assert ((int)SkColorType::kUnknown_SkColorType     == (int)UNKNOWN_SK_COLORTYPE,     ASSERT_MSG(SkColorType, sk_colortype_t));
static_assert ((int)SkColorType::kAlpha_8_SkColorType     == (int)ALPHA_8_SK_COLORTYPE,     ASSERT_MSG(SkColorType, sk_colortype_t));
static_assert ((int)SkColorType::kRGB_565_SkColorType     == (int)RGB_565_SK_COLORTYPE,     ASSERT_MSG(SkColorType, sk_colortype_t));
static_assert ((int)SkColorType::kARGB_4444_SkColorType   == (int)ARGB_4444_SK_COLORTYPE,   ASSERT_MSG(SkColorType, sk_colortype_t));
static_assert ((int)SkColorType::kRGBA_8888_SkColorType   == (int)RGBA_8888_SK_COLORTYPE,   ASSERT_MSG(SkColorType, sk_colortype_t));
static_assert ((int)SkColorType::kBGRA_8888_SkColorType   == (int)BGRA_8888_SK_COLORTYPE,   ASSERT_MSG(SkColorType, sk_colortype_t));
static_assert ((int)SkColorType::kIndex_8_SkColorType     == (int)Index_8_SK_COLORTYPE,     ASSERT_MSG(SkColorType, sk_colortype_t));
static_assert ((int)SkColorType::kGray_8_SkColorType      == (int)Gray_8_SK_COLORTYPE,      ASSERT_MSG(SkColorType, sk_colortype_t));
static_assert ((int)SkColorType::kRGBA_F16_SkColorType    == (int)RGBA_F16_SK_COLORTYPE,    ASSERT_MSG(SkColorType, sk_colortype_t));

// sk_alphatype_t
static_assert ((int)SkAlphaType::kUnknown_SkAlphaType    == (int)UNKNOWN_SK_ALPHATYPE,    ASSERT_MSG(SkAlphaType, sk_alphatype_t));
static_assert ((int)SkAlphaType::kOpaque_SkAlphaType     == (int)OPAQUE_SK_ALPHATYPE,     ASSERT_MSG(SkAlphaType, sk_alphatype_t));
static_assert ((int)SkAlphaType::kPremul_SkAlphaType     == (int)PREMUL_SK_ALPHATYPE,     ASSERT_MSG(SkAlphaType, sk_alphatype_t));
static_assert ((int)SkAlphaType::kUnpremul_SkAlphaType   == (int)UNPREMUL_SK_ALPHATYPE,   ASSERT_MSG(SkAlphaType, sk_alphatype_t));

// sk_pixelgeometry_t
static_assert ((int)SkPixelGeometry::kUnknown_SkPixelGeometry   == (int)UNKNOWN_SK_PIXELGEOMETRY,   ASSERT_MSG(SkPixelGeometry, sk_pixelgeometry_t));
static_assert ((int)SkPixelGeometry::kRGB_H_SkPixelGeometry     == (int)RGB_H_SK_PIXELGEOMETRY,     ASSERT_MSG(SkPixelGeometry, sk_pixelgeometry_t));
static_assert ((int)SkPixelGeometry::kBGR_H_SkPixelGeometry     == (int)BGR_H_SK_PIXELGEOMETRY,     ASSERT_MSG(SkPixelGeometry, sk_pixelgeometry_t));
static_assert ((int)SkPixelGeometry::kRGB_V_SkPixelGeometry     == (int)RGB_V_SK_PIXELGEOMETRY,     ASSERT_MSG(SkPixelGeometry, sk_pixelgeometry_t));
static_assert ((int)SkPixelGeometry::kBGR_V_SkPixelGeometry     == (int)BGR_V_SK_PIXELGEOMETRY,     ASSERT_MSG(SkPixelGeometry, sk_pixelgeometry_t));

// sk_shader_tilemode_t
static_assert ((int)SkShader::TileMode::kClamp_TileMode    == (int)CLAMP_SK_SHADER_TILEMODE,    ASSERT_MSG(SkShader::TileMode, sk_shader_tilemode_t));
static_assert ((int)SkShader::TileMode::kRepeat_TileMode   == (int)REPEAT_SK_SHADER_TILEMODE,   ASSERT_MSG(SkShader::TileMode, sk_shader_tilemode_t));
static_assert ((int)SkShader::TileMode::kMirror_TileMode   == (int)MIRROR_SK_SHADER_TILEMODE,   ASSERT_MSG(SkShader::TileMode, sk_shader_tilemode_t));

// sk_blurstyle_t
static_assert ((int)SkBlurStyle::kNormal_SkBlurStyle   == (int)NORMAL_SK_BLUR_STYLE,   ASSERT_MSG(SkBlurStyle, sk_blurstyle_t));
static_assert ((int)SkBlurStyle::kSolid_SkBlurStyle    == (int)SOLID_SK_BLUR_STYLE,    ASSERT_MSG(SkBlurStyle, sk_blurstyle_t));
static_assert ((int)SkBlurStyle::kOuter_SkBlurStyle    == (int)OUTER_SK_BLUR_STYLE,    ASSERT_MSG(SkBlurStyle, sk_blurstyle_t));
static_assert ((int)SkBlurStyle::kInner_SkBlurStyle    == (int)INNER_SK_BLUR_STYLE,    ASSERT_MSG(SkBlurStyle, sk_blurstyle_t));

// sk_stroke_cap_t
static_assert ((int)SkPaint::Cap::kButt_Cap     == (int)BUTT_SK_STROKE_CAP,     ASSERT_MSG(SkPaint::Cap, sk_stroke_cap_t));
static_assert ((int)SkPaint::Cap::kRound_Cap    == (int)ROUND_SK_STROKE_CAP,    ASSERT_MSG(SkPaint::Cap, sk_stroke_cap_t));
static_assert ((int)SkPaint::Cap::kSquare_Cap   == (int)SQUARE_SK_STROKE_CAP,   ASSERT_MSG(SkPaint::Cap, sk_stroke_cap_t));

// sk_stroke_join_t
static_assert ((int)SkPaint::Join::kMiter_Join   == (int)MITER_SK_STROKE_JOIN,   ASSERT_MSG(SkPaint::Join, sk_stroke_join_t));
static_assert ((int)SkPaint::Join::kRound_Join   == (int)ROUND_SK_STROKE_JOIN,   ASSERT_MSG(SkPaint::Join, sk_stroke_join_t));
static_assert ((int)SkPaint::Join::kBevel_Join   == (int)BEVEL_SK_STROKE_JOIN,   ASSERT_MSG(SkPaint::Join, sk_stroke_join_t));

// sk_region_op_t
static_assert ((int)SkRegion::Op::kDifference_Op          == (int)DIFFERENCE_SK_REGION_OP,           ASSERT_MSG(SkRegion::Op, sk_region_op_t));
static_assert ((int)SkRegion::Op::kIntersect_Op           == (int)INTERSECT_SK_REGION_OP,            ASSERT_MSG(SkRegion::Op, sk_region_op_t));
static_assert ((int)SkRegion::Op::kUnion_Op               == (int)UNION_SK_REGION_OP,                ASSERT_MSG(SkRegion::Op, sk_region_op_t));
static_assert ((int)SkRegion::Op::kXOR_Op                 == (int)XOR_SK_REGION_OP,                  ASSERT_MSG(SkRegion::Op, sk_region_op_t));
static_assert ((int)SkRegion::Op::kReverseDifference_Op   == (int)REVERSE_DIFFERENCE_SK_REGION_OP,   ASSERT_MSG(SkRegion::Op, sk_region_op_t));
static_assert ((int)SkRegion::Op::kReplace_Op             == (int)REPLACE_SK_REGION_OP,              ASSERT_MSG(SkRegion::Op, sk_region_op_t));

// sk_encoded_format_t
static_assert ((int)SkEncodedFormat::kUnknown_SkEncodedFormat   == (int)UNKNOWN_SK_ENCODED_FORMAT,   ASSERT_MSG(SkEncodedFormat, sk_encoded_format_t));
static_assert ((int)SkEncodedFormat::kBMP_SkEncodedFormat       == (int)BMP_SK_ENCODED_FORMAT,       ASSERT_MSG(SkEncodedFormat, sk_encoded_format_t));
static_assert ((int)SkEncodedFormat::kGIF_SkEncodedFormat       == (int)GIF_SK_ENCODED_FORMAT,       ASSERT_MSG(SkEncodedFormat, sk_encoded_format_t));
static_assert ((int)SkEncodedFormat::kICO_SkEncodedFormat       == (int)ICO_SK_ENCODED_FORMAT,       ASSERT_MSG(SkEncodedFormat, sk_encoded_format_t));
static_assert ((int)SkEncodedFormat::kJPEG_SkEncodedFormat      == (int)JPEG_SK_ENCODED_FORMAT,      ASSERT_MSG(SkEncodedFormat, sk_encoded_format_t));
static_assert ((int)SkEncodedFormat::kPNG_SkEncodedFormat       == (int)PNG_SK_ENCODED_FORMAT,       ASSERT_MSG(SkEncodedFormat, sk_encoded_format_t));
static_assert ((int)SkEncodedFormat::kWBMP_SkEncodedFormat      == (int)WBMP_SK_ENCODED_FORMAT,      ASSERT_MSG(SkEncodedFormat, sk_encoded_format_t));
static_assert ((int)SkEncodedFormat::kWEBP_SkEncodedFormat      == (int)WEBP_SK_ENCODED_FORMAT,      ASSERT_MSG(SkEncodedFormat, sk_encoded_format_t));
static_assert ((int)SkEncodedFormat::kPKM_SkEncodedFormat       == (int)PKM_SK_ENCODED_FORMAT,       ASSERT_MSG(SkEncodedFormat, sk_encoded_format_t));
static_assert ((int)SkEncodedFormat::kKTX_SkEncodedFormat       == (int)KTX_SK_ENCODED_FORMAT,       ASSERT_MSG(SkEncodedFormat, sk_encoded_format_t));
static_assert ((int)SkEncodedFormat::kASTC_SkEncodedFormat      == (int)ASTC_SK_ENCODED_FORMAT,      ASSERT_MSG(SkEncodedFormat, sk_encoded_format_t));
static_assert ((int)SkEncodedFormat::kDNG_SkEncodedFormat       == (int)DNG_SK_ENCODED_FORMAT,       ASSERT_MSG(SkEncodedFormat, sk_encoded_format_t));

// sk_codec_origin_t
static_assert ((int)SkCodec::Origin::kTopLeft_Origin       == (int)TOP_LEFT_SK_CODEC_ORIGIN,       ASSERT_MSG(SkCodec::Origin, sk_codec_origin_t));
static_assert ((int)SkCodec::Origin::kTopRight_Origin      == (int)TOP_RIGHT_SK_CODEC_ORIGIN,      ASSERT_MSG(SkCodec::Origin, sk_codec_origin_t));
static_assert ((int)SkCodec::Origin::kBottomRight_Origin   == (int)BOTTOM_RIGHT_SK_CODEC_ORIGIN,   ASSERT_MSG(SkCodec::Origin, sk_codec_origin_t));
static_assert ((int)SkCodec::Origin::kBottomLeft_Origin    == (int)BOTTOM_LEFT_SK_CODEC_ORIGIN,    ASSERT_MSG(SkCodec::Origin, sk_codec_origin_t));
static_assert ((int)SkCodec::Origin::kLeftTop_Origin       == (int)LEFT_TOP_SK_CODEC_ORIGIN,       ASSERT_MSG(SkCodec::Origin, sk_codec_origin_t));
static_assert ((int)SkCodec::Origin::kRightTop_Origin      == (int)RIGHT_TOP_SK_CODEC_ORIGIN,      ASSERT_MSG(SkCodec::Origin, sk_codec_origin_t));
static_assert ((int)SkCodec::Origin::kRightBottom_Origin   == (int)RIGHT_BOTTOM_SK_CODEC_ORIGIN,   ASSERT_MSG(SkCodec::Origin, sk_codec_origin_t));
static_assert ((int)SkCodec::Origin::kLeftBottom_Origin    == (int)LEFT_BOTTOM_SK_CODEC_ORIGIN,    ASSERT_MSG(SkCodec::Origin, sk_codec_origin_t));

// sk_codec_result_t
static_assert ((int)SkCodec::Result::kSuccess             == (int)SUCCESS_SK_CODEC_RESULT,              ASSERT_MSG(SkCodec::Result, sk_codec_result_t));
static_assert ((int)SkCodec::Result::kIncompleteInput     == (int)INCOMPLETE_INPUT_SK_CODEC_RESULT,     ASSERT_MSG(SkCodec::Result, sk_codec_result_t));
static_assert ((int)SkCodec::Result::kInvalidConversion   == (int)INVALID_CONVERSION_SK_CODEC_RESULT,   ASSERT_MSG(SkCodec::Result, sk_codec_result_t));
static_assert ((int)SkCodec::Result::kInvalidScale        == (int)INVALID_SCALE_SK_CODEC_RESULT,        ASSERT_MSG(SkCodec::Result, sk_codec_result_t));
static_assert ((int)SkCodec::Result::kInvalidParameters   == (int)INVALID_PARAMETERS_SK_CODEC_RESULT,   ASSERT_MSG(SkCodec::Result, sk_codec_result_t));
static_assert ((int)SkCodec::Result::kInvalidInput        == (int)INVALID_INPUT_SK_CODEC_RESULT,        ASSERT_MSG(SkCodec::Result, sk_codec_result_t));
static_assert ((int)SkCodec::Result::kCouldNotRewind      == (int)COULD_NOT_REWIND_SK_CODEC_RESULT,     ASSERT_MSG(SkCodec::Result, sk_codec_result_t));
static_assert ((int)SkCodec::Result::kUnimplemented       == (int)UNIMPLEMENTED_SK_CODEC_RESULT,        ASSERT_MSG(SkCodec::Result, sk_codec_result_t));

// sk_codec_zero_initialized_t
static_assert ((int)SkCodec::ZeroInitialized::kYes_ZeroInitialized   == (int)YES_SK_CODEC_ZERO_INITIALIZED,   ASSERT_MSG(SkCodec::ZeroInitialized, sk_codec_zero_initialized_t));
static_assert ((int)SkCodec::ZeroInitialized::kNo_ZeroInitialized    == (int)NO_SK_CODEC_ZERO_INITIALIZED,    ASSERT_MSG(SkCodec::ZeroInitialized, sk_codec_zero_initialized_t));

// sk_path_effect_1d_style_t
static_assert ((int)SkPath1DPathEffect::Style::kTranslate_Style   == (int)TRANSLATE_SK_PATH_EFFECT_1D_STYLE,   ASSERT_MSG(SkPath1DPathEffect::Style, sk_path_effect_1d_style_t));
static_assert ((int)SkPath1DPathEffect::Style::kRotate_Style      == (int)ROTATE_SK_PATH_EFFECT_1D_STYLE,      ASSERT_MSG(SkPath1DPathEffect::Style, sk_path_effect_1d_style_t));
static_assert ((int)SkPath1DPathEffect::Style::kMorph_Style       == (int)MORPH_SK_PATH_EFFECT_1D_STYLE,       ASSERT_MSG(SkPath1DPathEffect::Style, sk_path_effect_1d_style_t));

// sk_point_mode_t
static_assert ((int)SkCanvas::PointMode::kPoints_PointMode    == (int)POINTS_SK_POINT_MODE,    ASSERT_MSG(SkCanvas::PointMode, sk_point_mode_t));
static_assert ((int)SkCanvas::PointMode::kLines_PointMode     == (int)LINES_SK_POINT_MODE,     ASSERT_MSG(SkCanvas::PointMode, sk_point_mode_t));
static_assert ((int)SkCanvas::PointMode::kPolygon_PointMode   == (int)POLYGON_SK_POINT_MODE,   ASSERT_MSG(SkCanvas::PointMode, sk_point_mode_t));

#endif
