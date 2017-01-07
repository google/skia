/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "sk_types_priv.h"
#include "SkShadowMaskFilter.h"
#include "SkBitmapScaler.h"

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

// sk_path_arc_size_t
static_assert ((int)SkPath::ArcSize::kLarge_ArcSize   == (int)LARGE_SK_PATH_ARC_SIZE,   ASSERT_MSG(SkPath::ArcSize, sk_path_arc_size_t));
static_assert ((int)SkPath::ArcSize::kSmall_ArcSize   == (int)SMALL_SK_PATH_ARC_SIZE,   ASSERT_MSG(SkPath::ArcSize, sk_path_arc_size_t));

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

// sk_blendmode_t
static_assert ((int)SkBlendMode::kClear        == (int)CLEAR_SK_BLENDMODE,        ASSERT_MSG(SkBlendMode, sk_blendmode_t));
static_assert ((int)SkBlendMode::kSrc          == (int)SRC_SK_BLENDMODE,          ASSERT_MSG(SkBlendMode, sk_blendmode_t));
static_assert ((int)SkBlendMode::kDst          == (int)DST_SK_BLENDMODE,          ASSERT_MSG(SkBlendMode, sk_blendmode_t));
static_assert ((int)SkBlendMode::kSrcOver      == (int)SRCOVER_SK_BLENDMODE,      ASSERT_MSG(SkBlendMode, sk_blendmode_t));
static_assert ((int)SkBlendMode::kDstOver      == (int)DSTOVER_SK_BLENDMODE,      ASSERT_MSG(SkBlendMode, sk_blendmode_t));
static_assert ((int)SkBlendMode::kSrcIn        == (int)SRCIN_SK_BLENDMODE,        ASSERT_MSG(SkBlendMode, sk_blendmode_t));
static_assert ((int)SkBlendMode::kDstIn        == (int)DSTIN_SK_BLENDMODE,        ASSERT_MSG(SkBlendMode, sk_blendmode_t));
static_assert ((int)SkBlendMode::kSrcOut       == (int)SRCOUT_SK_BLENDMODE,       ASSERT_MSG(SkBlendMode, sk_blendmode_t));
static_assert ((int)SkBlendMode::kDstOut       == (int)DSTOUT_SK_BLENDMODE,       ASSERT_MSG(SkBlendMode, sk_blendmode_t));
static_assert ((int)SkBlendMode::kSrcATop      == (int)SRCATOP_SK_BLENDMODE,      ASSERT_MSG(SkBlendMode, sk_blendmode_t));
static_assert ((int)SkBlendMode::kDstATop      == (int)DSTATOP_SK_BLENDMODE,      ASSERT_MSG(SkBlendMode, sk_blendmode_t));
static_assert ((int)SkBlendMode::kXor          == (int)XOR_SK_BLENDMODE,          ASSERT_MSG(SkBlendMode, sk_blendmode_t));
static_assert ((int)SkBlendMode::kPlus         == (int)PLUS_SK_BLENDMODE,         ASSERT_MSG(SkBlendMode, sk_blendmode_t));
static_assert ((int)SkBlendMode::kModulate     == (int)MODULATE_SK_BLENDMODE,     ASSERT_MSG(SkBlendMode, sk_blendmode_t));
static_assert ((int)SkBlendMode::kScreen       == (int)SCREEN_SK_BLENDMODE,       ASSERT_MSG(SkBlendMode, sk_blendmode_t));
static_assert ((int)SkBlendMode::kOverlay      == (int)OVERLAY_SK_BLENDMODE,      ASSERT_MSG(SkBlendMode, sk_blendmode_t));
static_assert ((int)SkBlendMode::kDarken       == (int)DARKEN_SK_BLENDMODE,       ASSERT_MSG(SkBlendMode, sk_blendmode_t));
static_assert ((int)SkBlendMode::kLighten      == (int)LIGHTEN_SK_BLENDMODE,      ASSERT_MSG(SkBlendMode, sk_blendmode_t));
static_assert ((int)SkBlendMode::kColorDodge   == (int)COLORDODGE_SK_BLENDMODE,   ASSERT_MSG(SkBlendMode, sk_blendmode_t));
static_assert ((int)SkBlendMode::kColorBurn    == (int)COLORBURN_SK_BLENDMODE,    ASSERT_MSG(SkBlendMode, sk_blendmode_t));
static_assert ((int)SkBlendMode::kHardLight    == (int)HARDLIGHT_SK_BLENDMODE,    ASSERT_MSG(SkBlendMode, sk_blendmode_t));
static_assert ((int)SkBlendMode::kSoftLight    == (int)SOFTLIGHT_SK_BLENDMODE,    ASSERT_MSG(SkBlendMode, sk_blendmode_t));
static_assert ((int)SkBlendMode::kDifference   == (int)DIFFERENCE_SK_BLENDMODE,   ASSERT_MSG(SkBlendMode, sk_blendmode_t));
static_assert ((int)SkBlendMode::kExclusion    == (int)EXCLUSION_SK_BLENDMODE,    ASSERT_MSG(SkBlendMode, sk_blendmode_t));
static_assert ((int)SkBlendMode::kMultiply     == (int)MULTIPLY_SK_BLENDMODE,     ASSERT_MSG(SkBlendMode, sk_blendmode_t));
static_assert ((int)SkBlendMode::kHue          == (int)HUE_SK_BLENDMODE,          ASSERT_MSG(SkBlendMode, sk_blendmode_t));
static_assert ((int)SkBlendMode::kSaturation   == (int)SATURATION_SK_BLENDMODE,   ASSERT_MSG(SkBlendMode, sk_blendmode_t));
static_assert ((int)SkBlendMode::kColor        == (int)COLOR_SK_BLENDMODE,        ASSERT_MSG(SkBlendMode, sk_blendmode_t));
static_assert ((int)SkBlendMode::kLuminosity   == (int)LUMINOSITY_SK_BLENDMODE,   ASSERT_MSG(SkBlendMode, sk_blendmode_t));

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

// sk_clipop_t
static_assert ((int)SkClipOp::kDifference_SkClipOp   == (int)DIFFERENCE_SK_CLIPOP,           ASSERT_MSG(SkClipOp, sk_clipop_t));
static_assert ((int)SkClipOp::kIntersect_SkClipOp    == (int)INTERSECT_SK_CLIPOP,            ASSERT_MSG(SkClipOp, sk_clipop_t));

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

// sk_path_effect_1d_style_t
static_assert ((int)SkPaint::Style::kFill_Style            == (int)FILL_SK_PAINT_STYLE,              ASSERT_MSG(SkPaint::Style, sk_paint_style_t));
static_assert ((int)SkPaint::Style::kStrokeAndFill_Style   == (int)STROKE_AND_FILL_SK_PAINT_STYLE,   ASSERT_MSG(SkPaint::Style, sk_paint_style_t));
static_assert ((int)SkPaint::Style::kStroke_Style          == (int)STROKE_SK_PAINT_STYLE,            ASSERT_MSG(SkPaint::Style, sk_paint_style_t));

// sk_path_effect_1d_style_t
static_assert ((int)SkPaint::Hinting::kNo_Hinting       == (int)NO_HINTING_SK_PAINT_HINTING,       ASSERT_MSG(SkPaint::Hinting, sk_paint_style_t));
static_assert ((int)SkPaint::Hinting::kSlight_Hinting   == (int)SLIGHT_HINTING_SK_PAINT_HINTING,   ASSERT_MSG(SkPaint::Hinting, sk_paint_style_t));
static_assert ((int)SkPaint::Hinting::kNormal_Hinting   == (int)NORMAL_HINTING_SK_PAINT_HINTING,   ASSERT_MSG(SkPaint::Hinting, sk_paint_style_t));
static_assert ((int)SkPaint::Hinting::kFull_Hinting     == (int)FULL_HINTING_SK_PAINT_HINTING,     ASSERT_MSG(SkPaint::Hinting, sk_paint_style_t));

// sk_point_mode_t
static_assert ((int)SkCanvas::PointMode::kPoints_PointMode    == (int)POINTS_SK_POINT_MODE,    ASSERT_MSG(SkCanvas::PointMode, sk_point_mode_t));
static_assert ((int)SkCanvas::PointMode::kLines_PointMode     == (int)LINES_SK_POINT_MODE,     ASSERT_MSG(SkCanvas::PointMode, sk_point_mode_t));
static_assert ((int)SkCanvas::PointMode::kPolygon_PointMode   == (int)POLYGON_SK_POINT_MODE,   ASSERT_MSG(SkCanvas::PointMode, sk_point_mode_t));

// sk_surfaceprops_flags_t
static_assert ((int)SkSurfaceProps::Flags::kUseDeviceIndependentFonts_Flag   == (int)USE_DEVICE_INDEPENDENT_FONTS_GR_SURFACE_PROPS_FLAGS,   ASSERT_MSG(SkSurfaceProps::Flags, sk_surfaceprops_flags_t));

// gr_surfaceorigin_t
static_assert ((int)GrSurfaceOrigin::kBottomLeft_GrSurfaceOrigin   == (int)BOTTOM_LEFT_GR_SURFACE_ORIGIN,   ASSERT_MSG(GrSurfaceOrigin, gr_surfaceorigin_t));
static_assert ((int)GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin      == (int)TOP_LEFT_GR_SURFACE_ORIGIN,      ASSERT_MSG(GrSurfaceOrigin, gr_surfaceorigin_t));

// gr_pixelconfig_t
static_assert ((int)GrPixelConfig::kUnknown_GrPixelConfig          == (int)UNKNOWN_GR_PIXEL_CONFIG,          ASSERT_MSG(GrPixelConfig, gr_pixelconfig_t));
static_assert ((int)GrPixelConfig::kIndex_8_GrPixelConfig          == (int)INDEX_8_GR_PIXEL_CONFIG,          ASSERT_MSG(GrPixelConfig, gr_pixelconfig_t));
static_assert ((int)GrPixelConfig::kAlpha_8_GrPixelConfig          == (int)ALPHA_8_GR_PIXEL_CONFIG,          ASSERT_MSG(GrPixelConfig, gr_pixelconfig_t));
static_assert ((int)GrPixelConfig::kRGB_565_GrPixelConfig          == (int)RGB_565_GR_PIXEL_CONFIG,          ASSERT_MSG(GrPixelConfig, gr_pixelconfig_t));
static_assert ((int)GrPixelConfig::kRGBA_4444_GrPixelConfig        == (int)RGBA_4444_GR_PIXEL_CONFIG,        ASSERT_MSG(GrPixelConfig, gr_pixelconfig_t));
static_assert ((int)GrPixelConfig::kRGBA_8888_GrPixelConfig        == (int)RGBA_8888_GR_PIXEL_CONFIG,        ASSERT_MSG(GrPixelConfig, gr_pixelconfig_t));
static_assert ((int)GrPixelConfig::kBGRA_8888_GrPixelConfig        == (int)BGRA_8888_GR_PIXEL_CONFIG,        ASSERT_MSG(GrPixelConfig, gr_pixelconfig_t));
static_assert ((int)GrPixelConfig::kSRGBA_8888_GrPixelConfig       == (int)SRGBA_8888_GR_PIXEL_CONFIG,       ASSERT_MSG(GrPixelConfig, gr_pixelconfig_t));
static_assert ((int)GrPixelConfig::kSBGRA_8888_GrPixelConfig       == (int)SBGRA_8888_GR_PIXEL_CONFIG,       ASSERT_MSG(GrPixelConfig, gr_pixelconfig_t));
static_assert ((int)GrPixelConfig::kRGBA_8888_sint_GrPixelConfig   == (int)RGBA_8888_SINT_GR_PIXEL_CONFIG,   ASSERT_MSG(GrPixelConfig, gr_pixelconfig_t));
static_assert ((int)GrPixelConfig::kETC1_GrPixelConfig             == (int)ETC1_GR_PIXEL_CONFIG,             ASSERT_MSG(GrPixelConfig, gr_pixelconfig_t));
static_assert ((int)GrPixelConfig::kLATC_GrPixelConfig             == (int)LATC_GR_PIXEL_CONFIG,             ASSERT_MSG(GrPixelConfig, gr_pixelconfig_t));
static_assert ((int)GrPixelConfig::kR11_EAC_GrPixelConfig          == (int)R11_EAC_GR_PIXEL_CONFIG,          ASSERT_MSG(GrPixelConfig, gr_pixelconfig_t));
static_assert ((int)GrPixelConfig::kASTC_12x12_GrPixelConfig       == (int)ASTC_12X12_GR_PIXEL_CONFIG,       ASSERT_MSG(GrPixelConfig, gr_pixelconfig_t));
static_assert ((int)GrPixelConfig::kRGBA_float_GrPixelConfig       == (int)RGBA_FLOAT_GR_PIXEL_CONFIG,       ASSERT_MSG(GrPixelConfig, gr_pixelconfig_t));
static_assert ((int)GrPixelConfig::kAlpha_half_GrPixelConfig       == (int)ALPHA_HALF_GR_PIXEL_CONFIG,       ASSERT_MSG(GrPixelConfig, gr_pixelconfig_t));
static_assert ((int)GrPixelConfig::kRGBA_half_GrPixelConfig        == (int)RGBA_HALF_GR_PIXEL_CONFIG,        ASSERT_MSG(GrPixelConfig, gr_pixelconfig_t));

// gr_backendtexture_flags_t
static_assert ((int)GrBackendTextureFlags::kNone_GrBackendTextureFlag           == (int)NONE_GR_BACKEND_TEXTURE_FLAGS,            ASSERT_MSG(GrBackendTextureFlags, gr_backendtexture_flags_t));
static_assert ((int)GrBackendTextureFlags::kRenderTarget_GrBackendTextureFlag   == (int)RENDER_TARGET_GR_BACKEND_TEXTURE_FLAGS,   ASSERT_MSG(GrBackendTextureFlags, gr_backendtexture_flags_t));

// gr_backend_t
static_assert ((int)GrBackend::kOpenGL_GrBackend   == (int)OPENGL_GR_BACKEND,   ASSERT_MSG(GrBackend, gr_backend_t));
static_assert ((int)GrBackend::kVulkan_GrBackend   == (int)VULKAN_GR_BACKEND,   ASSERT_MSG(GrBackend, gr_backend_t));

// sk_bitmapscaler_resizemethod_t
static_assert ((int)SkBitmapScaler::ResizeMethod::RESIZE_BOX        == (int)BOX_SK_BITMAP_SCALER_RESIZE_METHOD,        ASSERT_MSG(SkBitmapScaler::ResizeMethod, sk_bitmapscaler_resizemethod_t));
static_assert ((int)SkBitmapScaler::ResizeMethod::RESIZE_TRIANGLE   == (int)TRIANGLE_SK_BITMAP_SCALER_RESIZE_METHOD,   ASSERT_MSG(SkBitmapScaler::ResizeMethod, sk_bitmapscaler_resizemethod_t));
static_assert ((int)SkBitmapScaler::ResizeMethod::RESIZE_LANCZOS3   == (int)LANCZOS3_SK_BITMAP_SCALER_RESIZE_METHOD,   ASSERT_MSG(SkBitmapScaler::ResizeMethod, sk_bitmapscaler_resizemethod_t));
static_assert ((int)SkBitmapScaler::ResizeMethod::RESIZE_HAMMING    == (int)HAMMING_SK_BITMAP_SCALER_RESIZE_METHOD,    ASSERT_MSG(SkBitmapScaler::ResizeMethod, sk_bitmapscaler_resizemethod_t));
static_assert ((int)SkBitmapScaler::ResizeMethod::RESIZE_MITCHELL   == (int)MITCHELL_SK_BITMAP_SCALER_RESIZE_METHOD,   ASSERT_MSG(SkBitmapScaler::ResizeMethod, sk_bitmapscaler_resizemethod_t));

// SkBudgeted
static_assert ((bool)SkBudgeted::kNo    == (bool)false,   ASSERT_MSG(SkBudgeted, bool));
static_assert ((bool)SkBudgeted::kYes   == (bool)true,    ASSERT_MSG(SkBudgeted, bool));

// sk_pathop_t
static_assert ((int)SkPathOp::kDifference_SkPathOp          == (int)DIFFERENCE_SK_PATHOP,           ASSERT_MSG(SkPathOp, sk_pathop_t));
static_assert ((int)SkPathOp::kIntersect_SkPathOp           == (int)INTERSECT_SK_PATHOP,            ASSERT_MSG(SkPathOp, sk_pathop_t));
static_assert ((int)SkPathOp::kUnion_SkPathOp               == (int)UNION_SK_PATHOP,                ASSERT_MSG(SkPathOp, sk_pathop_t));
static_assert ((int)SkPathOp::kXOR_SkPathOp                 == (int)XOR_SK_PATHOP,                  ASSERT_MSG(SkPathOp, sk_pathop_t));
static_assert ((int)SkPathOp::kReverseDifference_SkPathOp   == (int)REVERSE_DIFFERENCE_SK_PATHOP,   ASSERT_MSG(SkPathOp, sk_pathop_t));

// sk_path_convexity_t
static_assert ((int)SkPath::Convexity::kUnknown_Convexity   == (int)UNKNOWN_SK_PATH_CONVEXITY,   ASSERT_MSG(SkPath::Convexity, sk_path_convexity_t));
static_assert ((int)SkPath::Convexity::kConvex_Convexity    == (int)CONVEX_SK_PATH_CONVEXITY,    ASSERT_MSG(SkPath::Convexity, sk_path_convexity_t));
static_assert ((int)SkPath::Convexity::kConcave_Convexity   == (int)CONCAVE_SK_PATH_CONVEXITY,   ASSERT_MSG(SkPath::Convexity, sk_path_convexity_t));

// gr_backendtexture_flags_t
static_assert ((int)0                                              == (int)DEFAULT_SK_LATTICE_FLAGS,       ASSERT_MSG(SkCanvas::Lattice::Flags, sk_lattice_flags_t));
static_assert ((int)SkCanvas::Lattice::Flags::kTransparent_Flags   == (int)TRANSPARENT_SK_LATTICE_FLAGS,   ASSERT_MSG(SkCanvas::Lattice::Flags, sk_lattice_flags_t));

// sk_pathmeasure_matrixflags_t
static_assert ((int)SkPathMeasure::MatrixFlags::kGetPosition_MatrixFlag    == (int)GET_POSITION_SK_PATHMEASURE_MATRIXFLAGS,      ASSERT_MSG(SkPathMeasure::MatrixFlags, sk_pathmeasure_matrixflags_t));
static_assert ((int)SkPathMeasure::MatrixFlags::kGetTangent_MatrixFlag     == (int)GET_TANGENT_SK_PATHMEASURE_MATRIXFLAGS,       ASSERT_MSG(SkPathMeasure::MatrixFlags, sk_pathmeasure_matrixflags_t));
static_assert ((int)SkPathMeasure::MatrixFlags::kGetPosAndTan_MatrixFlag   == (int)GET_POS_AND_TAN_SK_PATHMEASURE_MATRIXFLAGS,   ASSERT_MSG(SkPathMeasure::MatrixFlags, sk_pathmeasure_matrixflags_t));

// sk_shadowmaskfilter_shadowflags_t
static_assert ((int)SkShadowMaskFilter::ShadowFlags::kNone_ShadowFlag                  == (int)NONE_SK_SHADOWMASKFILTER_SHADOWFLAGS,                   ASSERT_MSG(SkShadowMaskFilter::ShadowFlags, sk_shadowmaskfilter_shadowflags_t));
static_assert ((int)SkShadowMaskFilter::ShadowFlags::kTransparentOccluder_ShadowFlag   == (int)TRANSPARENT_OCCLUDER_SK_SHADOWMASKFILTER_SHADOWFLAGS,   ASSERT_MSG(SkShadowMaskFilter::ShadowFlags, sk_shadowmaskfilter_shadowflags_t));
static_assert ((int)SkShadowMaskFilter::ShadowFlags::kLargerUmbra_ShadowFlag           == (int)LARGER_UMBRA_SK_SHADOWMASKFILTER_SHADOWFLAGS,           ASSERT_MSG(SkShadowMaskFilter::ShadowFlags, sk_shadowmaskfilter_shadowflags_t));
static_assert ((int)SkShadowMaskFilter::ShadowFlags::kGaussianEdge_ShadowFlag          == (int)GAUSSIAN_EDGE_SK_SHADOWMASKFILTER_SHADOWFLAGS,          ASSERT_MSG(SkShadowMaskFilter::ShadowFlags, sk_shadowmaskfilter_shadowflags_t));
static_assert ((int)SkShadowMaskFilter::ShadowFlags::kAll_ShadowFlag                   == (int)ALL_SK_SHADOWMASKFILTER_SHADOWFLAGS,                    ASSERT_MSG(SkShadowMaskFilter::ShadowFlags, sk_shadowmaskfilter_shadowflags_t));

// sk_encodedinfo_alpha_t
static_assert ((int)SkEncodedInfo::Alpha::kOpaque_Alpha     == (int)OPAQUE_SK_ENCODEDINFO_ALPHA,     ASSERT_MSG(SkEncodedInfo::Alpha, sk_encodedinfo_alpha_t));
static_assert ((int)SkEncodedInfo::Alpha::kUnpremul_Alpha   == (int)UNPREMUL_SK_ENCODEDINFO_ALPHA,   ASSERT_MSG(SkEncodedInfo::Alpha, sk_encodedinfo_alpha_t));
static_assert ((int)SkEncodedInfo::Alpha::kBinary_Alpha     == (int)BINARY_SK_ENCODEDINFO_ALPHA,     ASSERT_MSG(SkEncodedInfo::Alpha, sk_encodedinfo_alpha_t));

// sk_encodedinfo_color_t
static_assert ((int)SkEncodedInfo::Color::kGray_Color           == (int)GRAY_SK_ENCODEDINFO_COLOR,            ASSERT_MSG(SkEncodedInfo::Color, sk_encodedinfo_color_t));
static_assert ((int)SkEncodedInfo::Color::kGrayAlpha_Color      == (int)GRAY_ALPHA_SK_ENCODEDINFO_COLOR,      ASSERT_MSG(SkEncodedInfo::Color, sk_encodedinfo_color_t));
static_assert ((int)SkEncodedInfo::Color::kPalette_Color        == (int)PALETTE_SK_ENCODEDINFO_COLOR,         ASSERT_MSG(SkEncodedInfo::Color, sk_encodedinfo_color_t));
static_assert ((int)SkEncodedInfo::Color::kRGB_Color            == (int)RGB_SK_ENCODEDINFO_COLOR,             ASSERT_MSG(SkEncodedInfo::Color, sk_encodedinfo_color_t));
static_assert ((int)SkEncodedInfo::Color::kRGBA_Color           == (int)RGBA_SK_ENCODEDINFO_COLOR,            ASSERT_MSG(SkEncodedInfo::Color, sk_encodedinfo_color_t));
static_assert ((int)SkEncodedInfo::Color::kBGR_Color            == (int)BGR_SK_ENCODEDINFO_COLOR,             ASSERT_MSG(SkEncodedInfo::Color, sk_encodedinfo_color_t));
static_assert ((int)SkEncodedInfo::Color::kBGRX_Color           == (int)BGRX_SK_ENCODEDINFO_COLOR,            ASSERT_MSG(SkEncodedInfo::Color, sk_encodedinfo_color_t));
static_assert ((int)SkEncodedInfo::Color::kBGRA_Color           == (int)BGRA_SK_ENCODEDINFO_COLOR,            ASSERT_MSG(SkEncodedInfo::Color, sk_encodedinfo_color_t));
static_assert ((int)SkEncodedInfo::Color::kYUV_Color            == (int)YUV_SK_ENCODEDINFO_COLOR,             ASSERT_MSG(SkEncodedInfo::Color, sk_encodedinfo_color_t));
static_assert ((int)SkEncodedInfo::Color::kYUVA_Color           == (int)YUVA_SK_ENCODEDINFO_COLOR,            ASSERT_MSG(SkEncodedInfo::Color, sk_encodedinfo_color_t));
static_assert ((int)SkEncodedInfo::Color::kInvertedCMYK_Color   == (int)INVERTED_CMYK_SK_ENCODEDINFO_COLOR,   ASSERT_MSG(SkEncodedInfo::Color, sk_encodedinfo_color_t));
static_assert ((int)SkEncodedInfo::Color::kYCCK_Color           == (int)YCCK_SK_ENCODEDINFO_COLOR,            ASSERT_MSG(SkEncodedInfo::Color, sk_encodedinfo_color_t));

#endif
