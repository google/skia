/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/c/sk_types_priv.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkMatrix44.h"
#include "include/core/SkPathMeasure.h"
#include "include/core/SkRegion.h"
#include "include/core/SkShader.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkVertices.h"
#include "include/effects/Sk1DPathEffect.h"
#include "include/effects/SkBlurMaskFilter.h"
#include "include/effects/SkDisplacementMapEffect.h"
#include "include/effects/SkDropShadowImageFilter.h"
#include "include/effects/SkHighContrastFilter.h"
#include "include/effects/SkMatrixConvolutionImageFilter.h"
#include "include/effects/SkTrimPathEffect.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/encode/SkPngEncoder.h"
#include "include/encode/SkWebpEncoder.h"
#include "include/pathops/SkPathOps.h"
#include "src/core/SkMask.h"

#if SK_SUPPORT_GPU
#include "include/gpu/GrTypes.h"
#include "include/gpu/GrContextOptions.h"
#endif

#if __cplusplus >= 199711L

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define ASSERT_MSG(SK, C) "ABI changed, you must write a enumeration mapper for " TOSTRING(#SK) " to " TOSTRING(#C) "."

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

// sk_path_segment_mask_t
static_assert ((int)SkPath::SegmentMask::kLine_SegmentMask    == (int)LINE_SK_PATH_SEGMENT_MASK,    ASSERT_MSG(SkPath::SegmentMask, sk_path_segment_mask_t));
static_assert ((int)SkPath::SegmentMask::kQuad_SegmentMask    == (int)QUAD_SK_PATH_SEGMENT_MASK,    ASSERT_MSG(SkPath::SegmentMask, sk_path_segment_mask_t));
static_assert ((int)SkPath::SegmentMask::kConic_SegmentMask   == (int)CONIC_SK_PATH_SEGMENT_MASK,   ASSERT_MSG(SkPath::SegmentMask, sk_path_segment_mask_t));
static_assert ((int)SkPath::SegmentMask::kCubic_SegmentMask   == (int)CUBIC_SK_PATH_SEGMENT_MASK,   ASSERT_MSG(SkPath::SegmentMask, sk_path_segment_mask_t));

// sk_text_align_t
static_assert ((int)SkTextUtils::Align::kLeft_Align     == (int)LEFT_SK_TEXT_ALIGN,     ASSERT_MSG(SkTextUtils::Align, sk_text_align_t));
static_assert ((int)SkTextUtils::Align::kCenter_Align   == (int)CENTER_SK_TEXT_ALIGN,   ASSERT_MSG(SkTextUtils::Align, sk_text_align_t));
static_assert ((int)SkTextUtils::Align::kRight_Align    == (int)RIGHT_SK_TEXT_ALIGN,    ASSERT_MSG(SkTextUtils::Align, sk_text_align_t));

// sk_text_encoding_t
static_assert ((int)SkTextEncoding::kUTF8      == (int)UTF8_SK_TEXT_ENCODING,       ASSERT_MSG(SkTextEncoding, sk_text_encoding_t));
static_assert ((int)SkTextEncoding::kUTF16     == (int)UTF16_SK_TEXT_ENCODING,      ASSERT_MSG(SkTextEncoding, sk_text_encoding_t));
static_assert ((int)SkTextEncoding::kUTF32     == (int)UTF32_SK_TEXT_ENCODING,      ASSERT_MSG(SkTextEncoding, sk_text_encoding_t));
static_assert ((int)SkTextEncoding::kGlyphID   == (int)GLYPH_ID_SK_TEXT_ENCODING,   ASSERT_MSG(SkTextEncoding, sk_text_encoding_t));

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
static_assert ((int)SkColorType::kUnknown_SkColorType        == (int)UNKNOWN_SK_COLORTYPE,        ASSERT_MSG(SkColorType, sk_colortype_t));
static_assert ((int)SkColorType::kAlpha_8_SkColorType        == (int)ALPHA_8_SK_COLORTYPE,        ASSERT_MSG(SkColorType, sk_colortype_t));
static_assert ((int)SkColorType::kRGB_565_SkColorType        == (int)RGB_565_SK_COLORTYPE,        ASSERT_MSG(SkColorType, sk_colortype_t));
static_assert ((int)SkColorType::kARGB_4444_SkColorType      == (int)ARGB_4444_SK_COLORTYPE,      ASSERT_MSG(SkColorType, sk_colortype_t));
static_assert ((int)SkColorType::kRGBA_8888_SkColorType      == (int)RGBA_8888_SK_COLORTYPE,      ASSERT_MSG(SkColorType, sk_colortype_t));
static_assert ((int)SkColorType::kRGB_888x_SkColorType       == (int)RGB_888X_SK_COLORTYPE,       ASSERT_MSG(SkColorType, sk_colortype_t));
static_assert ((int)SkColorType::kBGRA_8888_SkColorType      == (int)BGRA_8888_SK_COLORTYPE,      ASSERT_MSG(SkColorType, sk_colortype_t));
static_assert ((int)SkColorType::kRGBA_1010102_SkColorType   == (int)RGBA_1010102_SK_COLORTYPE,   ASSERT_MSG(SkColorType, sk_colortype_t));
static_assert ((int)SkColorType::kRGB_101010x_SkColorType    == (int)RGB_101010X_SK_COLORTYPE,    ASSERT_MSG(SkColorType, sk_colortype_t));
static_assert ((int)SkColorType::kGray_8_SkColorType         == (int)GRAY_8_SK_COLORTYPE,         ASSERT_MSG(SkColorType, sk_colortype_t));
static_assert ((int)SkColorType::kRGBA_F16Norm_SkColorType   == (int)RGBA_F16_NORM_SK_COLORTYPE,  ASSERT_MSG(SkColorType, sk_colortype_t));
static_assert ((int)SkColorType::kRGBA_F16_SkColorType       == (int)RGBA_F16_SK_COLORTYPE,       ASSERT_MSG(SkColorType, sk_colortype_t));
static_assert ((int)SkColorType::kRGBA_F32_SkColorType       == (int)RGBA_F32_SK_COLORTYPE,       ASSERT_MSG(SkColorType, sk_colortype_t));

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
static_assert ((int)SkTileMode::kClamp    == (int)CLAMP_SK_SHADER_TILEMODE,    ASSERT_MSG(SkTileMode, sk_shader_tilemode_t));
static_assert ((int)SkTileMode::kRepeat   == (int)REPEAT_SK_SHADER_TILEMODE,   ASSERT_MSG(SkTileMode, sk_shader_tilemode_t));
static_assert ((int)SkTileMode::kMirror   == (int)MIRROR_SK_SHADER_TILEMODE,   ASSERT_MSG(SkTileMode, sk_shader_tilemode_t));
static_assert ((int)SkTileMode::kDecal    == (int)DECAL_SK_SHADER_TILEMODE,    ASSERT_MSG(SkTileMode, sk_shader_tilemode_t));

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
static_assert ((int)SkClipOp::kDifference   == (int)DIFFERENCE_SK_CLIPOP,           ASSERT_MSG(SkClipOp, sk_clipop_t));
static_assert ((int)SkClipOp::kIntersect    == (int)INTERSECT_SK_CLIPOP,            ASSERT_MSG(SkClipOp, sk_clipop_t));

// sk_encoded_image_format_t
static_assert ((int)SkEncodedImageFormat::kBMP       == (int)BMP_SK_ENCODED_FORMAT,       ASSERT_MSG(SkEncodedImageFormat, sk_encoded_image_format_t));
static_assert ((int)SkEncodedImageFormat::kGIF       == (int)GIF_SK_ENCODED_FORMAT,       ASSERT_MSG(SkEncodedImageFormat, sk_encoded_image_format_t));
static_assert ((int)SkEncodedImageFormat::kICO       == (int)ICO_SK_ENCODED_FORMAT,       ASSERT_MSG(SkEncodedImageFormat, sk_encoded_image_format_t));
static_assert ((int)SkEncodedImageFormat::kJPEG      == (int)JPEG_SK_ENCODED_FORMAT,      ASSERT_MSG(SkEncodedImageFormat, sk_encoded_image_format_t));
static_assert ((int)SkEncodedImageFormat::kPNG       == (int)PNG_SK_ENCODED_FORMAT,       ASSERT_MSG(SkEncodedImageFormat, sk_encoded_image_format_t));
static_assert ((int)SkEncodedImageFormat::kWBMP      == (int)WBMP_SK_ENCODED_FORMAT,      ASSERT_MSG(SkEncodedImageFormat, sk_encoded_image_format_t));
static_assert ((int)SkEncodedImageFormat::kWEBP      == (int)WEBP_SK_ENCODED_FORMAT,      ASSERT_MSG(SkEncodedImageFormat, sk_encoded_image_format_t));
static_assert ((int)SkEncodedImageFormat::kPKM       == (int)PKM_SK_ENCODED_FORMAT,       ASSERT_MSG(SkEncodedImageFormat, sk_encoded_image_format_t));
static_assert ((int)SkEncodedImageFormat::kKTX       == (int)KTX_SK_ENCODED_FORMAT,       ASSERT_MSG(SkEncodedImageFormat, sk_encoded_image_format_t));
static_assert ((int)SkEncodedImageFormat::kASTC      == (int)ASTC_SK_ENCODED_FORMAT,      ASSERT_MSG(SkEncodedImageFormat, sk_encoded_image_format_t));
static_assert ((int)SkEncodedImageFormat::kDNG       == (int)DNG_SK_ENCODED_FORMAT,       ASSERT_MSG(SkEncodedImageFormat, sk_encoded_image_format_t));
static_assert ((int)SkEncodedImageFormat::kHEIF      == (int)HEIF_SK_ENCODED_FORMAT,      ASSERT_MSG(SkEncodedImageFormat, sk_encoded_image_format_t));

// SK_ENCODED_ORIGIN_t
static_assert ((int)SkEncodedOrigin::kTopLeft_SkEncodedOrigin       == (int)TOP_LEFT_SK_ENCODED_ORIGIN,         ASSERT_MSG(SkEncodedOrigin, sk_encodedorigin_t));
static_assert ((int)SkEncodedOrigin::kTopRight_SkEncodedOrigin      == (int)TOP_RIGHT_SK_ENCODED_ORIGIN,        ASSERT_MSG(SkEncodedOrigin, sk_encodedorigin_t));
static_assert ((int)SkEncodedOrigin::kBottomRight_SkEncodedOrigin   == (int)BOTTOM_RIGHT_SK_ENCODED_ORIGIN,     ASSERT_MSG(SkEncodedOrigin, sk_encodedorigin_t));
static_assert ((int)SkEncodedOrigin::kBottomLeft_SkEncodedOrigin    == (int)BOTTOM_LEFT_SK_ENCODED_ORIGIN,      ASSERT_MSG(SkEncodedOrigin, sk_encodedorigin_t));
static_assert ((int)SkEncodedOrigin::kLeftTop_SkEncodedOrigin       == (int)LEFT_TOP_SK_ENCODED_ORIGIN,         ASSERT_MSG(SkEncodedOrigin, sk_encodedorigin_t));
static_assert ((int)SkEncodedOrigin::kRightTop_SkEncodedOrigin      == (int)RIGHT_TOP_SK_ENCODED_ORIGIN,        ASSERT_MSG(SkEncodedOrigin, sk_encodedorigin_t));
static_assert ((int)SkEncodedOrigin::kRightBottom_SkEncodedOrigin   == (int)RIGHT_BOTTOM_SK_ENCODED_ORIGIN,     ASSERT_MSG(SkEncodedOrigin, sk_encodedorigin_t));
static_assert ((int)SkEncodedOrigin::kLeftBottom_SkEncodedOrigin    == (int)LEFT_BOTTOM_SK_ENCODED_ORIGIN,      ASSERT_MSG(SkEncodedOrigin, sk_encodedorigin_t));
static_assert ((int)SkEncodedOrigin::kDefault_SkEncodedOrigin       == (int)DEFAULT_SK_ENCODED_ORIGIN,          ASSERT_MSG(SkEncodedOrigin, sk_encodedorigin_t));

// sk_codec_result_t
static_assert ((int)SkCodec::Result::kSuccess             == (int)SUCCESS_SK_CODEC_RESULT,              ASSERT_MSG(SkCodec::Result, sk_codec_result_t));
static_assert ((int)SkCodec::Result::kIncompleteInput     == (int)INCOMPLETE_INPUT_SK_CODEC_RESULT,     ASSERT_MSG(SkCodec::Result, sk_codec_result_t));
static_assert ((int)SkCodec::Result::kErrorInInput        == (int)ERROR_IN_INPUT_SK_CODEC_RESULT,       ASSERT_MSG(SkCodec::Result, sk_codec_result_t));
static_assert ((int)SkCodec::Result::kInvalidConversion   == (int)INVALID_CONVERSION_SK_CODEC_RESULT,   ASSERT_MSG(SkCodec::Result, sk_codec_result_t));
static_assert ((int)SkCodec::Result::kInvalidScale        == (int)INVALID_SCALE_SK_CODEC_RESULT,        ASSERT_MSG(SkCodec::Result, sk_codec_result_t));
static_assert ((int)SkCodec::Result::kInvalidParameters   == (int)INVALID_PARAMETERS_SK_CODEC_RESULT,   ASSERT_MSG(SkCodec::Result, sk_codec_result_t));
static_assert ((int)SkCodec::Result::kInvalidInput        == (int)INVALID_INPUT_SK_CODEC_RESULT,        ASSERT_MSG(SkCodec::Result, sk_codec_result_t));
static_assert ((int)SkCodec::Result::kCouldNotRewind      == (int)COULD_NOT_REWIND_SK_CODEC_RESULT,     ASSERT_MSG(SkCodec::Result, sk_codec_result_t));
static_assert ((int)SkCodec::Result::kInternalError       == (int)INTERNAL_ERROR_SK_CODEC_RESULT,       ASSERT_MSG(SkCodec::Result, sk_codec_result_t));
static_assert ((int)SkCodec::Result::kUnimplemented       == (int)UNIMPLEMENTED_SK_CODEC_RESULT,        ASSERT_MSG(SkCodec::Result, sk_codec_result_t));

// sk_codec_zero_initialized_t
static_assert ((int)SkCodec::ZeroInitialized::kYes_ZeroInitialized   == (int)YES_SK_CODEC_ZERO_INITIALIZED,   ASSERT_MSG(SkCodec::ZeroInitialized, sk_codec_zero_initialized_t));
static_assert ((int)SkCodec::ZeroInitialized::kNo_ZeroInitialized    == (int)NO_SK_CODEC_ZERO_INITIALIZED,    ASSERT_MSG(SkCodec::ZeroInitialized, sk_codec_zero_initialized_t));

// sk_transfer_function_behavior_t
static_assert ((int)SkTransferFunctionBehavior::kRespect   == (int)RESPECT_SK_TRANSFER_FUNCTION_BEHAVIOR,   ASSERT_MSG(SkTransferFunctionBehavior, sk_transfer_function_behavior_t));
static_assert ((int)SkTransferFunctionBehavior::kIgnore    == (int)IGNORE_SK_TRANSFER_FUNCTION_BEHAVIOR,    ASSERT_MSG(SkTransferFunctionBehavior, sk_transfer_function_behavior_t));

// sk_codec_scanline_order_t
static_assert ((int)SkCodec::SkScanlineOrder::kTopDown_SkScanlineOrder    == (int)TOP_DOWN_SK_CODEC_SCANLINE_ORDER,    ASSERT_MSG(SkCodec::SkScanlineOrder, sk_codec_scanline_order_t));
static_assert ((int)SkCodec::SkScanlineOrder::kBottomUp_SkScanlineOrder   == (int)BOTTOM_UP_SK_CODEC_SCANLINE_ORDER,   ASSERT_MSG(SkCodec::SkScanlineOrder, sk_codec_scanline_order_t));

// sk_codecanimation_disposalmethod_t
static_assert ((int)SkCodecAnimation::DisposalMethod::kKeep              == (int)KEEP_SK_CODEC_ANIMATION_DISPOSAL_METHOD,               ASSERT_MSG(SkCodec::SkScanlineOrder, sk_codecanimation_disposalmethod_t));
static_assert ((int)SkCodecAnimation::DisposalMethod::kRestoreBGColor    == (int)RESTORE_BG_COLOR_SK_CODEC_ANIMATION_DISPOSAL_METHOD,   ASSERT_MSG(SkCodec::SkScanlineOrder, sk_codecanimation_disposalmethod_t));
static_assert ((int)SkCodecAnimation::DisposalMethod::kRestorePrevious   == (int)RESTORE_PREVIOUS_SK_CODEC_ANIMATION_DISPOSAL_METHOD,   ASSERT_MSG(SkCodec::SkScanlineOrder, sk_codecanimation_disposalmethod_t));

// sk_path_effect_1d_style_t
static_assert ((int)SkPath1DPathEffect::Style::kTranslate_Style   == (int)TRANSLATE_SK_PATH_EFFECT_1D_STYLE,   ASSERT_MSG(SkPath1DPathEffect::Style, sk_path_effect_1d_style_t));
static_assert ((int)SkPath1DPathEffect::Style::kRotate_Style      == (int)ROTATE_SK_PATH_EFFECT_1D_STYLE,      ASSERT_MSG(SkPath1DPathEffect::Style, sk_path_effect_1d_style_t));
static_assert ((int)SkPath1DPathEffect::Style::kMorph_Style       == (int)MORPH_SK_PATH_EFFECT_1D_STYLE,       ASSERT_MSG(SkPath1DPathEffect::Style, sk_path_effect_1d_style_t));

// sk_path_effect_trim_mode_t
static_assert ((int)SkTrimPathEffect::Mode::kNormal     == (int)NORMAL_SK_PATH_EFFECT_TRIM_MODE,     ASSERT_MSG(SkTrimPathEffect::Mode, sk_path_effect_trim_mode_t));
static_assert ((int)SkTrimPathEffect::Mode::kInverted   == (int)INVERTED_SK_PATH_EFFECT_TRIM_MODE,   ASSERT_MSG(SkTrimPathEffect::Mode, sk_path_effect_trim_mode_t));

// sk_paint_style_t
static_assert ((int)SkPaint::Style::kFill_Style            == (int)FILL_SK_PAINT_STYLE,              ASSERT_MSG(SkPaint::Style, sk_paint_style_t));
static_assert ((int)SkPaint::Style::kStrokeAndFill_Style   == (int)STROKE_AND_FILL_SK_PAINT_STYLE,   ASSERT_MSG(SkPaint::Style, sk_paint_style_t));
static_assert ((int)SkPaint::Style::kStroke_Style          == (int)STROKE_SK_PAINT_STYLE,            ASSERT_MSG(SkPaint::Style, sk_paint_style_t));

// sk_font_hinting_t
static_assert ((int)SkFontHinting::kNone     == (int)NONE_SK_FONT_HINTING,     ASSERT_MSG(SkFontHinting, sk_font_hinting_t));
static_assert ((int)SkFontHinting::kSlight   == (int)SLIGHT_SK_FONT_HINTING,   ASSERT_MSG(SkFontHinting, sk_font_hinting_t));
static_assert ((int)SkFontHinting::kNormal   == (int)NORMAL_SK_FONT_HINTING,   ASSERT_MSG(SkFontHinting, sk_font_hinting_t));
static_assert ((int)SkFontHinting::kFull     == (int)FULL_SK_FONT_HINTING,     ASSERT_MSG(SkFontHinting, sk_font_hinting_t));

// sk_point_mode_t
static_assert ((int)SkCanvas::PointMode::kPoints_PointMode    == (int)POINTS_SK_POINT_MODE,    ASSERT_MSG(SkCanvas::PointMode, sk_point_mode_t));
static_assert ((int)SkCanvas::PointMode::kLines_PointMode     == (int)LINES_SK_POINT_MODE,     ASSERT_MSG(SkCanvas::PointMode, sk_point_mode_t));
static_assert ((int)SkCanvas::PointMode::kPolygon_PointMode   == (int)POLYGON_SK_POINT_MODE,   ASSERT_MSG(SkCanvas::PointMode, sk_point_mode_t));

// sk_surfaceprops_flags_t
static_assert ((int)SkSurfaceProps::Flags::kUseDeviceIndependentFonts_Flag   == (int)USE_DEVICE_INDEPENDENT_FONTS_SK_SURFACE_PROPS_FLAGS,   ASSERT_MSG(SkSurfaceProps::Flags, sk_surfaceprops_flags_t));

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

// sk_lattice_flags_t
static_assert ((int)SkCanvas::Lattice::RectType::kDefault       == (int)DEFAULT_SK_LATTICE_RECT_TYPE,       ASSERT_MSG(SkCanvas::Lattice::Flags, sk_lattice_recttype_t));
static_assert ((int)SkCanvas::Lattice::RectType::kTransparent   == (int)TRANSPARENT_SK_LATTICE_RECT_TYPE,   ASSERT_MSG(SkCanvas::Lattice::Flags, sk_lattice_recttype_t));
static_assert ((int)SkCanvas::Lattice::RectType::kFixedColor    == (int)FIXED_COLOR_SK_LATTICE_RECT_TYPE,   ASSERT_MSG(SkCanvas::Lattice::Flags, sk_lattice_recttype_t));

// sk_pathmeasure_matrixflags_t
static_assert ((int)SkPathMeasure::MatrixFlags::kGetPosition_MatrixFlag    == (int)GET_POSITION_SK_PATHMEASURE_MATRIXFLAGS,      ASSERT_MSG(SkPathMeasure::MatrixFlags, sk_pathmeasure_matrixflags_t));
static_assert ((int)SkPathMeasure::MatrixFlags::kGetTangent_MatrixFlag     == (int)GET_TANGENT_SK_PATHMEASURE_MATRIXFLAGS,       ASSERT_MSG(SkPathMeasure::MatrixFlags, sk_pathmeasure_matrixflags_t));
static_assert ((int)SkPathMeasure::MatrixFlags::kGetPosAndTan_MatrixFlag   == (int)GET_POS_AND_TAN_SK_PATHMEASURE_MATRIXFLAGS,   ASSERT_MSG(SkPathMeasure::MatrixFlags, sk_pathmeasure_matrixflags_t));

// sk_colorspace_gamut_t
static_assert ((int)SkColorSpace::Gamut::kSRGB_Gamut        == (int)SRGB_SK_COLORSPACE_GAMUT,        ASSERT_MSG(SkColorSpace::Gamut, sk_colorspace_gamut_t));
static_assert ((int)SkColorSpace::Gamut::kAdobeRGB_Gamut    == (int)ADOBE_RGB_SK_COLORSPACE_GAMUT,   ASSERT_MSG(SkColorSpace::Gamut, sk_colorspace_gamut_t));
static_assert ((int)SkColorSpace::Gamut::kDCIP3_D65_Gamut   == (int)DCIP3_D65_SK_COLORSPACE_GAMUT,   ASSERT_MSG(SkColorSpace::Gamut, sk_colorspace_gamut_t));
static_assert ((int)SkColorSpace::Gamut::kRec2020_Gamut     == (int)REC2020_SK_COLORSPACE_GAMUT,     ASSERT_MSG(SkColorSpace::Gamut, sk_colorspace_gamut_t));

// sk_mask_format_t
static_assert ((int)SkMask::Format::kBW_Format       == (int)BW_SK_MASK_FORMAT,        ASSERT_MSG(SkMask::Format, sk_mask_format_t));
static_assert ((int)SkMask::Format::kA8_Format       == (int)A8_SK_MASK_FORMAT,        ASSERT_MSG(SkMask::Format, sk_mask_format_t));
static_assert ((int)SkMask::Format::k3D_Format       == (int)THREE_D_SK_MASK_FORMAT,   ASSERT_MSG(SkMask::Format, sk_mask_format_t));
static_assert ((int)SkMask::Format::kARGB32_Format   == (int)ARGB32_SK_MASK_FORMAT,    ASSERT_MSG(SkMask::Format, sk_mask_format_t));
static_assert ((int)SkMask::Format::kLCD16_Format    == (int)LCD16_SK_MASK_FORMAT,     ASSERT_MSG(SkMask::Format, sk_mask_format_t));

// sk_matrix44_type_mask_t
static_assert ((int)SkMatrix44::TypeMask::kIdentity_Mask      == (int)IDENTITY_SK_MATRIX44_TYPE_MASK,      ASSERT_MSG(SkMatrix44::TypeMask, sk_matrix44_type_mask_t));
static_assert ((int)SkMatrix44::TypeMask::kTranslate_Mask     == (int)TRANSLATE_SK_MATRIX44_TYPE_MASK,     ASSERT_MSG(SkMatrix44::TypeMask, sk_matrix44_type_mask_t));
static_assert ((int)SkMatrix44::TypeMask::kScale_Mask         == (int)SCALE_SK_MATRIX44_TYPE_MASK,         ASSERT_MSG(SkMatrix44::TypeMask, sk_matrix44_type_mask_t));
static_assert ((int)SkMatrix44::TypeMask::kAffine_Mask        == (int)AFFINE_SK_MATRIX44_TYPE_MASK,        ASSERT_MSG(SkMatrix44::TypeMask, sk_matrix44_type_mask_t));
static_assert ((int)SkMatrix44::TypeMask::kPerspective_Mask   == (int)PERSPECTIVE_SK_MATRIX44_TYPE_MASK,   ASSERT_MSG(SkMatrix44::TypeMask, sk_matrix44_type_mask_t));

// sk_vertices_vertex_mode_t
static_assert ((int)SkVertices::VertexMode::kTriangles_VertexMode       == (int)TRIANGLES_SK_VERTICES_VERTEX_MODE,        ASSERT_MSG(SkVertices::VertexMode, sk_vertices_vertex_mode_t));
static_assert ((int)SkVertices::VertexMode::kTriangleStrip_VertexMode   == (int)TRIANGLE_STRIP_SK_VERTICES_VERTEX_MODE,   ASSERT_MSG(SkVertices::VertexMode, sk_vertices_vertex_mode_t));
static_assert ((int)SkVertices::VertexMode::kTriangleFan_VertexMode     == (int)TRIANGLE_FAN_SK_VERTICES_VERTEX_MODE,     ASSERT_MSG(SkVertices::VertexMode, sk_vertices_vertex_mode_t));

// sk_image_caching_hint_t
static_assert ((int)SkImage::CachingHint::kAllow_CachingHint      == (int)ALLOW_SK_IMAGE_CACHING_HINT,      ASSERT_MSG(SkImage::CachingHint, sk_image_caching_hint_t));
static_assert ((int)SkImage::CachingHint::kDisallow_CachingHint   == (int)DISALLOW_SK_IMAGE_CACHING_HINT,   ASSERT_MSG(SkImage::CachingHint, sk_image_caching_hint_t));

// sk_colorspace_render_target_gamma_t
static_assert ((int)SkColorSpace::RenderTargetGamma::kLinear_RenderTargetGamma   == (int)LINEAR_SK_COLORSPACE_RENDER_TARGET_GAMMA,   ASSERT_MSG(SkColorSpace::RenderTargetGamma, sk_colorspace_render_target_gamma_t));
static_assert ((int)SkColorSpace::RenderTargetGamma::kSRGB_RenderTargetGamma     == (int)SRGB_SK_COLORSPACE_RENDER_TARGET_GAMMA,     ASSERT_MSG(SkColorSpace::RenderTargetGamma, sk_colorspace_render_target_gamma_t));

// sk_gamma_named_t
static_assert ((int)SkGammaNamed::kLinear_SkGammaNamed        == (int)LINEAR_SK_GAMMA_NAMED,              ASSERT_MSG(SkGammaNamed, sk_gamma_named_t));
static_assert ((int)SkGammaNamed::kSRGB_SkGammaNamed          == (int)SRGB_SK_GAMMA_NAMED,                ASSERT_MSG(SkGammaNamed, sk_gamma_named_t));
static_assert ((int)SkGammaNamed::k2Dot2Curve_SkGammaNamed    == (int)TWO_DOT_TWO_CURVE_SK_GAMMA_NAMED,   ASSERT_MSG(SkGammaNamed, sk_gamma_named_t));
static_assert ((int)SkGammaNamed::kNonStandard_SkGammaNamed   == (int)NON_STANDARD_SK_GAMMA_NAMED,        ASSERT_MSG(SkGammaNamed, sk_gamma_named_t));

// sk_colorspace_type_t
static_assert ((int)SkColorSpace::Type::kRGB_Type    == (int)RGB_SK_COLORSPACE_TYPE,    ASSERT_MSG(SkColorSpace::Type, sk_colorspace_type_t));
static_assert ((int)SkColorSpace::Type::kCMYK_Type   == (int)CMYK_SK_COLORSPACE_TYPE,   ASSERT_MSG(SkColorSpace::Type, sk_colorspace_type_t));
static_assert ((int)SkColorSpace::Type::kGray_Type   == (int)GRAY_SK_COLORSPACE_TYPE,   ASSERT_MSG(SkColorSpace::Type, sk_colorspace_type_t));

// sk_highcontrastconfig_invertstyle_t
static_assert ((int)SkHighContrastConfig::InvertStyle::kNoInvert           == (int)NO_INVERT_SK_HIGH_CONTRAST_CONFIG_INVERT_STYLE,           ASSERT_MSG(SkHighContrastConfig::InvertStyle, sk_highcontrastconfig_invertstyle_t));
static_assert ((int)SkHighContrastConfig::InvertStyle::kInvertBrightness   == (int)INVERT_BRIGHTNESS_SK_HIGH_CONTRAST_CONFIG_INVERT_STYLE,   ASSERT_MSG(SkHighContrastConfig::InvertStyle, sk_highcontrastconfig_invertstyle_t));
static_assert ((int)SkHighContrastConfig::InvertStyle::kInvertLightness    == (int)INVERT_LIGHTNESS_SK_HIGH_CONTRAST_CONFIG_INVERT_STYLE,    ASSERT_MSG(SkHighContrastConfig::InvertStyle, sk_highcontrastconfig_invertstyle_t));

// sk_bitmap_allocflags_t
static_assert ((int)SkBitmap::AllocFlags::kZeroPixels_AllocFlag   == (int)ZERO_PIXELS_SK_BITMAP_ALLOC_FLAGS,   ASSERT_MSG(SkBitmap::AllocFlags, sk_bitmap_allocflags_t));

// sk_pngencoder_filterflags_t
static_assert ((int)SkPngEncoder::FilterFlag::kZero    == (int)ZERO_SK_PNGENCODER_FILTER_FLAGS,    ASSERT_MSG(SkPngEncoder::FilterFlag, sk_pngencoder_filterflags_t));
static_assert ((int)SkPngEncoder::FilterFlag::kNone    == (int)NONE_SK_PNGENCODER_FILTER_FLAGS,    ASSERT_MSG(SkPngEncoder::FilterFlag, sk_pngencoder_filterflags_t));
static_assert ((int)SkPngEncoder::FilterFlag::kSub     == (int)SUB_SK_PNGENCODER_FILTER_FLAGS,     ASSERT_MSG(SkPngEncoder::FilterFlag, sk_pngencoder_filterflags_t));
static_assert ((int)SkPngEncoder::FilterFlag::kUp      == (int)UP_SK_PNGENCODER_FILTER_FLAGS,      ASSERT_MSG(SkPngEncoder::FilterFlag, sk_pngencoder_filterflags_t));
static_assert ((int)SkPngEncoder::FilterFlag::kAvg     == (int)AVG_SK_PNGENCODER_FILTER_FLAGS,     ASSERT_MSG(SkPngEncoder::FilterFlag, sk_pngencoder_filterflags_t));
static_assert ((int)SkPngEncoder::FilterFlag::kPaeth   == (int)PAETH_SK_PNGENCODER_FILTER_FLAGS,   ASSERT_MSG(SkPngEncoder::FilterFlag, sk_pngencoder_filterflags_t));
static_assert ((int)SkPngEncoder::FilterFlag::kAll     == (int)ALL_SK_PNGENCODER_FILTER_FLAGS,     ASSERT_MSG(SkPngEncoder::FilterFlag, sk_pngencoder_filterflags_t));

// sk_jpegencoder_downsample_t
static_assert ((int)SkJpegEncoder::Downsample::k420   == (int)DOWNSAMPLE_420_SK_JPEGENCODER_DOWNSAMPLE,   ASSERT_MSG(SkJpegEncoder::Downsample, sk_jpegencoder_downsample_t));
static_assert ((int)SkJpegEncoder::Downsample::k422   == (int)DOWNSAMPLE_422_SK_JPEGENCODER_DOWNSAMPLE,   ASSERT_MSG(SkJpegEncoder::Downsample, sk_jpegencoder_downsample_t));
static_assert ((int)SkJpegEncoder::Downsample::k444   == (int)DOWNSAMPLE_444_SK_JPEGENCODER_DOWNSAMPLE,   ASSERT_MSG(SkJpegEncoder::Downsample, sk_jpegencoder_downsample_t));

// sk_jpegencoder_alphaoption_t
static_assert ((int)SkJpegEncoder::AlphaOption::kIgnore         == (int)IGNORE_SK_JPEGENCODER_ALPHA_OPTION,           ASSERT_MSG(SkJpegEncoder::AlphaOption, sk_jpegencoder_alphaoption_t));
static_assert ((int)SkJpegEncoder::AlphaOption::kBlendOnBlack   == (int)BLEND_ON_BLACK_SK_JPEGENCODER_ALPHA_OPTION,   ASSERT_MSG(SkJpegEncoder::AlphaOption, sk_jpegencoder_alphaoption_t));

// sk_webpencoder_compression_t
static_assert ((int)SkWebpEncoder::Compression::kLossy      == (int)LOSSY_SK_WEBPENCODER_COMPTRESSION,      ASSERT_MSG(SkWebpEncoder::Compression, sk_webpencoder_compression_t));
static_assert ((int)SkWebpEncoder::Compression::kLossless   == (int)LOSSLESS_SK_WEBPENCODER_COMPTRESSION,   ASSERT_MSG(SkWebpEncoder::Compression, sk_webpencoder_compression_t));

// sk_rrect_type_t
static_assert ((int)SkRRect::Type::kEmpty_Type       == (int)EMPTY_SK_RRECT_TYPE,        ASSERT_MSG(SkRRect::Type, sk_rrect_type_t));
static_assert ((int)SkRRect::Type::kRect_Type        == (int)RECT_SK_RRECT_TYPE,         ASSERT_MSG(SkRRect::Type, sk_rrect_type_t));
static_assert ((int)SkRRect::Type::kOval_Type        == (int)OVAL_SK_RRECT_TYPE,         ASSERT_MSG(SkRRect::Type, sk_rrect_type_t));
static_assert ((int)SkRRect::Type::kSimple_Type      == (int)SIMPLE_SK_RRECT_TYPE,       ASSERT_MSG(SkRRect::Type, sk_rrect_type_t));
static_assert ((int)SkRRect::Type::kNinePatch_Type   == (int)NINE_PATCH_SK_RRECT_TYPE,   ASSERT_MSG(SkRRect::Type, sk_rrect_type_t));
static_assert ((int)SkRRect::Type::kComplex_Type     == (int)COMPLEX_SK_RRECT_TYPE,      ASSERT_MSG(SkRRect::Type, sk_rrect_type_t));

// sk_rrect_corner_t
static_assert ((int)SkRRect::Corner::kUpperLeft_Corner    == (int)UPPER_LEFT_SK_RRECT_CORNER,    ASSERT_MSG(SkRRect::Corner, sk_rrect_corner_t));
static_assert ((int)SkRRect::Corner::kUpperRight_Corner   == (int)UPPER_RIGHT_SK_RRECT_CORNER,   ASSERT_MSG(SkRRect::Corner, sk_rrect_corner_t));
static_assert ((int)SkRRect::Corner::kLowerRight_Corner   == (int)LOWER_RIGHT_SK_RRECT_CORNER,   ASSERT_MSG(SkRRect::Corner, sk_rrect_corner_t));
static_assert ((int)SkRRect::Corner::kLowerLeft_Corner    == (int)LOWER_LEFT_SK_RRECT_CORNER,    ASSERT_MSG(SkRRect::Corner, sk_rrect_corner_t));

#if SK_SUPPORT_GPU

// gr_surfaceorigin_t
static_assert ((int)GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin      == (int)TOP_LEFT_GR_SURFACE_ORIGIN,      ASSERT_MSG(GrSurfaceOrigin, gr_surfaceorigin_t));
static_assert ((int)GrSurfaceOrigin::kBottomLeft_GrSurfaceOrigin   == (int)BOTTOM_LEFT_GR_SURFACE_ORIGIN,   ASSERT_MSG(GrSurfaceOrigin, gr_surfaceorigin_t));

// gr_pixelconfig_t
static_assert ((int)GrPixelConfig::kUnknown_GrPixelConfig          == (int)UNKNOWN_GR_PIXEL_CONFIG,          ASSERT_MSG(GrPixelConfig, gr_pixelconfig_t));
static_assert ((int)GrPixelConfig::kAlpha_8_GrPixelConfig          == (int)ALPHA_8_GR_PIXEL_CONFIG,          ASSERT_MSG(GrPixelConfig, gr_pixelconfig_t));
static_assert ((int)GrPixelConfig::kGray_8_GrPixelConfig           == (int)GRAY_8_GR_PIXEL_CONFIG,           ASSERT_MSG(GrPixelConfig, gr_pixelconfig_t));
static_assert ((int)GrPixelConfig::kRGB_565_GrPixelConfig          == (int)RGB_565_GR_PIXEL_CONFIG,          ASSERT_MSG(GrPixelConfig, gr_pixelconfig_t));
static_assert ((int)GrPixelConfig::kRGBA_4444_GrPixelConfig        == (int)RGBA_4444_GR_PIXEL_CONFIG,        ASSERT_MSG(GrPixelConfig, gr_pixelconfig_t));
static_assert ((int)GrPixelConfig::kRGBA_8888_GrPixelConfig        == (int)RGBA_8888_GR_PIXEL_CONFIG,        ASSERT_MSG(GrPixelConfig, gr_pixelconfig_t));
static_assert ((int)GrPixelConfig::kRGB_888_GrPixelConfig          == (int)RGB_888_GR_PIXEL_CONFIG,          ASSERT_MSG(GrPixelConfig, gr_pixelconfig_t));
static_assert ((int)GrPixelConfig::kBGRA_8888_GrPixelConfig        == (int)BGRA_8888_GR_PIXEL_CONFIG,        ASSERT_MSG(GrPixelConfig, gr_pixelconfig_t));
static_assert ((int)GrPixelConfig::kSRGBA_8888_GrPixelConfig       == (int)SRGBA_8888_GR_PIXEL_CONFIG,       ASSERT_MSG(GrPixelConfig, gr_pixelconfig_t));
static_assert ((int)GrPixelConfig::kSBGRA_8888_GrPixelConfig       == (int)SBGRA_8888_GR_PIXEL_CONFIG,       ASSERT_MSG(GrPixelConfig, gr_pixelconfig_t));
static_assert ((int)GrPixelConfig::kRGBA_1010102_GrPixelConfig     == (int)RGBA_1010102_GR_PIXEL_CONFIG,     ASSERT_MSG(GrPixelConfig, gr_pixelconfig_t));
static_assert ((int)GrPixelConfig::kRGBA_float_GrPixelConfig       == (int)RGBA_FLOAT_GR_PIXEL_CONFIG,       ASSERT_MSG(GrPixelConfig, gr_pixelconfig_t));
static_assert ((int)GrPixelConfig::kRG_float_GrPixelConfig         == (int)RG_FLOAT_GR_PIXEL_CONFIG,         ASSERT_MSG(GrPixelConfig, gr_pixelconfig_t));
static_assert ((int)GrPixelConfig::kAlpha_half_GrPixelConfig       == (int)ALPHA_HALF_GR_PIXEL_CONFIG,       ASSERT_MSG(GrPixelConfig, gr_pixelconfig_t));
static_assert ((int)GrPixelConfig::kRGBA_half_GrPixelConfig        == (int)RGBA_HALF_GR_PIXEL_CONFIG,        ASSERT_MSG(GrPixelConfig, gr_pixelconfig_t));

// gr_backend_t
static_assert ((int)GrBackend::kMetal_GrBackend    == (int)METAL_GR_BACKEND,    ASSERT_MSG(GrBackend, gr_backend_t));
static_assert ((int)GrBackend::kOpenGL_GrBackend   == (int)OPENGL_GR_BACKEND,   ASSERT_MSG(GrBackend, gr_backend_t));
static_assert ((int)GrBackend::kVulkan_GrBackend   == (int)VULKAN_GR_BACKEND,   ASSERT_MSG(GrBackend, gr_backend_t));

#endif

#endif
