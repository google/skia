/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_x_types_DEFINED
#define sk_x_types_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

typedef struct {
    float   x;
    float   y;
    float   z;
} sk_point3_t;

typedef struct {
    float   x;
    float   y;
} sk_ipoint_t;

typedef struct {
    float   w;
    float   h;
} sk_size_t;

typedef struct {
    float   w;
    float   h;
} sk_isize_t;

typedef struct {
    uint32_t fFlags;            // Bit field to identify which values are unknown
    float    fTop;              // The greatest distance above the baseline for any glyph (will be <= 0)
    float    fAscent;           // The recommended distance above the baseline (will be <= 0)
    float    fDescent;          // The recommended distance below the baseline (will be >= 0)
    float    fBottom;           // The greatest distance below the baseline for any glyph (will be >= 0)
    float    fLeading;          // The recommended distance to add between lines of text (will be >= 0)
    float    fAvgCharWidth;     // the average character width (>= 0)
    float    fMaxCharWidth;     // the max character width (>= 0)
    float    fXMin;             // The minimum bounding box x value for all glyphs
    float    fXMax;             // The maximum bounding box x value for all glyphs
    float    fXHeight;          // The height of an 'x' in px, or 0 if no 'x' in face
    float    fCapHeight;        // The cap height (> 0), or 0 if cannot be determined.
    float    fUnderlineThickness; // underline thickness, or 0 if cannot be determined
    float    fUnderlinePosition; // underline position, or 0 if cannot be determined
} sk_fontmetrics_t;

// Flags for fFlags member of sk_fontmetrics_t
#define FONTMETRICS_FLAGS_UNDERLINE_THICKNESS_IS_VALID (1U << 0)
#define FONTMETRICS_FLAGS_UNDERLINE_POSITION_IS_VALID (1U << 1)

/**
    A lightweight managed string.
*/
typedef struct sk_string_t sk_string_t;
/**

    A sk_bitmap_t is an abstraction that specifies a raster bitmap.
*/
typedef struct sk_bitmap_t sk_bitmap_t;
typedef struct sk_colorfilter_t sk_colorfilter_t;
typedef struct sk_imagefilter_t sk_imagefilter_t;
typedef struct sk_imagefilter_croprect_t sk_imagefilter_croprect_t;
/**
   A sk_typeface_t pecifies the typeface and intrinsic style of a font.
    This is used in the paint, along with optionally algorithmic settings like
    textSize, textSkewX, textScaleX, kFakeBoldText_Mask, to specify
    how text appears when drawn (and measured).

    Typeface objects are immutable, and so they can be shared between threads.
*/
typedef struct sk_typeface_t sk_typeface_t;
typedef uint32_t sk_font_table_tag_t;
/**
 *  Abstraction layer directly on top of an image codec.
 */
typedef struct sk_codec_t sk_codec_t;
typedef struct sk_colorspace_t sk_colorspace_t;
/**
   Various stream types
*/
typedef struct sk_stream_t sk_stream_t;
typedef struct sk_stream_filestream_t sk_stream_filestream_t;
typedef struct sk_stream_asset_t sk_stream_asset_t;
typedef struct sk_stream_memorystream_t sk_stream_memorystream_t;
typedef struct sk_stream_streamrewindable_t sk_stream_streamrewindable_t;
typedef struct sk_wstream_t sk_wstream_t;
typedef struct sk_wstream_filestream_t sk_wstream_filestream_t;
typedef struct sk_wstream_dynamicmemorystream_t sk_wstream_dynamicmemorystream_t;
/**
   High-level API for creating a document-based canvas.
*/
typedef struct sk_document_t sk_document_t;

typedef enum {
    UTF8_ENCODING,
    UTF16_ENCODING,
    UTF32_ENCODING
} sk_encoding_t;

typedef enum {
    POINTS_SK_POINT_MODE,
    LINES_SK_POINT_MODE,
    POLYGON_SK_POINT_MODE
} sk_point_mode_t;

typedef enum {
    LEFT_SK_TEXT_ALIGN,
    CENTER_SK_TEXT_ALIGN,
    RIGHT_SK_TEXT_ALIGN
} sk_text_align_t;

typedef enum {
    UTF8_SK_TEXT_ENCODING,
    UTF16_SK_TEXT_ENCODING,
    UTF32_SK_TEXT_ENCODING,
    GLYPH_ID_SK_TEXT_ENCODING
} sk_text_encoding_t;

typedef enum {
    WINDING_SK_PATH_FILLTYPE,
    EVENODD_SK_PATH_FILLTYPE,
    INVERSE_WINDING_SK_PATH_FILLTYPE,
    INVERSE_EVENODD_SK_PATH_FILLTYPE,
} sk_path_filltype_t;

typedef enum {
    NORMAL_TYPEFACE_STYLE      = 0,
    BOLD_TYPEFACE_STYLE        = 0x01,
    ITALIC_TYPEFACE_STYLE      = 0x02,
    BOLD_ITALIC_TYPEFACE_STYLE = 0x03
} sk_typeface_style_t;

typedef enum {
    UPRIGHT_SK_FONT_STYLE_SLANT = 0,
    ITALIC_SK_FONT_STYLE_SLANT  = 1,
    OBLIQUE_SK_FONT_STYLE_SLANT = 2,
} sk_font_style_slant_t;

typedef enum {
    NONE_SK_FILTER_QUALITY,
    LOW_SK_FILTER_QUALITY,
    MEDIUM_SK_FILTER_QUALITY,
    HIGH_SK_FILTER_QUALITY
} sk_filter_quality_t;

typedef enum {
    HAS_LEFT_SK_CROP_RECT_FLAG   = 0x01,
    HAS_TOP_SK_CROP_RECT_FLAG    = 0x02,
    HAS_WIDTH_SK_CROP_RECT_FLAG  = 0x04,
    HAS_HEIGHT_SK_CROP_RECT_FLAG = 0x08,
    HAS_ALL_SK_CROP_RECT_FLAG    = 0x0F,
} sk_crop_rect_flags_t;

typedef enum {
    DRAW_SHADOW_AND_FOREGROUND_SK_DROP_SHADOW_IMAGE_FILTER_SHADOW_MODE,
    DRAW_SHADOW_ONLY_SK_DROP_SHADOW_IMAGE_FILTER_SHADOW_MODE,
} sk_drop_shadow_image_filter_shadow_mode_t;

typedef enum {
    UNKNOWN_SK_DISPLACEMENT_MAP_EFFECT_CHANNEL_SELECTOR_TYPE,
    R_SK_DISPLACEMENT_MAP_EFFECT_CHANNEL_SELECTOR_TYPE,
    G_SK_DISPLACEMENT_MAP_EFFECT_CHANNEL_SELECTOR_TYPE,
    B_SK_DISPLACEMENT_MAP_EFFECT_CHANNEL_SELECTOR_TYPE,
    A_SK_DISPLACEMENT_MAP_EFFECT_CHANNEL_SELECTOR_TYPE,
} sk_displacement_map_effect_channel_selector_type_t;

typedef enum {
    UNKNOWN_SK_IMAGE_ENCODER_TYPE,
    BMP_SK_IMAGE_ENCODER_TYPE,
    GIF_SK_IMAGE_ENCODER_TYPE,
    ICO_SK_IMAGE_ENCODER_TYPE,
    JPEG_SK_IMAGE_ENCODER_TYPE,
    PNG_SK_IMAGE_ENCODER_TYPE,
    WBMP_SK_IMAGE_ENCODER_TYPE,
    WEBP_SK_IMAGE_ENCODER_TYPE,
    KTX_SK_IMAGE_ENCODER_TYPE,
} sk_image_encoder_t;

typedef enum {
    CLAMP_SK_MATRIX_CONVOLUTION_TILEMODE,
    REPEAT_SK_MATRIX_CONVOLUTION_TILEMODE,
    CLAMP_TO_BLACK_SK_MATRIX_CONVOLUTION_TILEMODE,
} sk_matrix_convolution_tilemode_t;

/**
    The logical operations that can be performed when combining two regions.
*/
typedef enum {
    DIFFERENCE_SK_REGION_OP,          //!< subtract the op region from the first region
    INTERSECT_SK_REGION_OP,           //!< intersect the two regions
    UNION_SK_REGION_OP,               //!< union (inclusive-or) the two regions
    XOR_SK_REGION_OP,                 //!< exclusive-or the two regions
    REVERSE_DIFFERENCE_SK_REGION_OP,  //!< subtract the first region from the op region
    REPLACE_SK_REGION_OP,             //!< replace the dst region with the op region
} sk_region_op_t;

/**
 *  Enum describing format of encoded data.
 */
typedef enum {
    UNKNOWN_SK_ENCODED_FORMAT,
    BMP_SK_ENCODED_FORMAT,
    GIF_SK_ENCODED_FORMAT,
    ICO_SK_ENCODED_FORMAT,
    JPEG_SK_ENCODED_FORMAT,
    PNG_SK_ENCODED_FORMAT,
    WBMP_SK_ENCODED_FORMAT,
    WEBP_SK_ENCODED_FORMAT,
    PKM_SK_ENCODED_FORMAT,
    KTX_SK_ENCODED_FORMAT,
    ASTC_SK_ENCODED_FORMAT,
    DNG_SK_ENCODED_FORMAT
} sk_encoded_format_t;

typedef enum {
    TOP_LEFT_SK_CODEC_ORIGIN     = 1, // Default
    TOP_RIGHT_SK_CODEC_ORIGIN    = 2, // Reflected across y-axis
    BOTTOM_RIGHT_SK_CODEC_ORIGIN = 3, // Rotated 180
    BOTTOM_LEFT_SK_CODEC_ORIGIN  = 4, // Reflected across x-axis
    LEFT_TOP_SK_CODEC_ORIGIN     = 5, // Reflected across x-axis, Rotated 90 CCW
    RIGHT_TOP_SK_CODEC_ORIGIN    = 6, // Rotated 90 CW
    RIGHT_BOTTOM_SK_CODEC_ORIGIN = 7, // Reflected across x-axis, Rotated 90 CW
    LEFT_BOTTOM_SK_CODEC_ORIGIN  = 8, // Rotated 90 CCW
} sk_codec_origin_t;

typedef enum {
    SUCCESS_SK_CODEC_RESULT,
    INCOMPLETE_INPUT_SK_CODEC_RESULT,
    INVALID_CONVERSION_SK_CODEC_RESULT,
    INVALID_SCALE_SK_CODEC_RESULT,
    INVALID_PARAMETERS_SK_CODEC_RESULT,
    INVALID_INPUT_SK_CODEC_RESULT,
    COULD_NOT_REWIND_SK_CODEC_RESULT,
    UNIMPLEMENTED_SK_CODEC_RESULT,
} sk_codec_result_t;

typedef enum {
    YES_SK_CODEC_ZERO_INITIALIZED,
    NO_SK_CODEC_ZERO_INITIALIZED,
} sk_codec_zero_initialized_t;

typedef struct {
    sk_codec_zero_initialized_t fZeroInitialized;
    sk_irect_t fSubset;
    bool fHasSubset;
} sk_codec_options_t;

// The verbs that can be foudn on a path
typedef enum {
	MOVE_PATH_VERB,
	LINE_PATH_VERB,
	QUAD_PATH_VERB,
	CONIC_PATH_VERB,
	CUBIC_PATH_VERB,
	CLOSE_PATH_VERB,
	DONE_PATH_VERB
} sk_path_verb_t;

typedef struct sk_path_iterator_t sk_path_iterator_t;
typedef struct sk_path_rawiterator_t sk_path_rawiterator_t;

typedef enum {
	APPEND_ADD_MODE,
	EXTEND_ADD_MODE,
} sk_path_add_mode_t;

typedef enum {
	TRANSLATE_SK_PATH_EFFECT_1D_STYLE,
	ROTATE_SK_PATH_EFFECT_1D_STYLE,
	MORPH_SK_PATH_EFFECT_1D_STYLE,
} sk_path_effect_1d_style_t;

typedef struct sk_path_effect_t sk_path_effect_t;  

SK_C_PLUS_PLUS_END_GUARD

#endif
